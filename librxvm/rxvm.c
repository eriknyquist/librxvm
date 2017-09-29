#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvm.h"
#include "string_builder.h"
#include "stage1.h"
#include "stage2.h"
#include "stack.h"
#include "vm.h"
#include "bmh.h"
#include "lfix_to_str.h"

#ifndef NOEXTRAS
#include "bmh.h"
#endif

#define LBUF_SIZE     16
#define FBUF_SIZE     256

#ifndef NOEXTRAS
static char *fbuf;
static int bufpos;
static long fpos;
static uint64_t *charcp;
#endif /* NOEXTRAS */

int rxvm_compile (rxvm_t *compiled, char *exp)
{
    ir_stack_t *ir;
    char *lfix;
    int ret;
    size_t size;
    size_t simplesize;
    size_t cmpsize;
    size_t lfix_size;

    if (!compiled || !exp) {
        return RXVM_EPARAM;
    }

    compiled->exe = NULL;
    compiled->simple = NULL;
    compiled->lfix = NULL;

    /* Compile expression to VM instructions in an incomplete
     * IR, stored in a linked list */
    if ((ret = stage1(compiled, exp, &ir)) < 0) {
        return ret;
    }

    /* Determine whether extra space should be allocated
     * for lfix string */
    size = 0;
    lfix_size = (compiled->lfixn - compiled->lfix0) + 1;
    if (lfix_size > 1) {
        size += lfix_size + 1;
    } else {
        lfix_size = 0;
    }

    if (ir == NULL) {
        /* Simple case; expression contains no special
         * characters. Stage2 not required, but a copy of the
         * original expression string must be stored */
        simplesize = strlen(exp) + 1;
        size += simplesize;

        if ((compiled->simple = malloc(size)) == NULL) {
            return RXVM_EMEM;
        }

        memcpy(compiled->simple, exp, simplesize);
        lfix = compiled->simple + simplesize;
    } else {
        /* Normal case; expression contains characters that require
         * full compilation. Stage2 must be run to generate
         * the final instructions */
        cmpsize = sizeof(inst_t) * ir->size;
        size += cmpsize;

        if ((compiled->exe = malloc(size)) == NULL)
            return RXVM_EMEM;

        /* Convert the linked list IR into a contiguous
         * array of executable instructions, and perform any
         * available optimisations on the final executable program */
        if ((ret = stage2(ir, compiled)) < 0) {
            return ret;
        }

        lfix = ((char *)compiled->exe) + cmpsize;
    }

    if (lfix_size) {
        compiled->lfix = lfix;
#ifndef NOEXTRAS
        lfix_to_str(exp, compiled);
#endif /* NOEXTRAS */
    }

    return 0;
}

char getchar_str (void *data)
{
    char **input;

    input = (char **)data;
    return *(*input)++;
}

static int simple_match (threads_t *tm, char *simple)
{
    char c;
    char *orig;

    orig = simple;
    tm->match_start = tm->chars;

    while (*simple) {
        c = (*tm->getchar)(tm->getchar_data);
        ++tm->chars;

        if (c == tm->endchar)
            return 1;

        if (*simple == '\\')
            ++simple;

skip_readchar:
        if (!char_match(tm->icase, *simple, c)) {
            if (!tm->search) return 0;

            tm->match_start = tm->chars;

            if (simple != orig) {
                simple = orig;
                --tm->match_start;
                goto skip_readchar;
            }
        } else {
            ++simple;
        }
    }

    tm->match_end = tm->chars + 1;
    return 1;
}

int rxvm_search (rxvm_t *compiled, char *input, char **start, char **end,
                 int flags)
{
    char *sot;
    threads_t tm;
    int ret;

    if (!compiled || !input) return RXVM_EPARAM;

    memset(&tm, 0, sizeof(threads_t));
    tm.getchar = getchar_str;
    tm.getchar_data = &input;
    tm.endchar = '\0';

    sot = input;
    tm.icase = (flags & RXVM_ICASE);
    tm.nongreedy = (flags & RXVM_NONGREEDY);
    tm.multiline = (flags & RXVM_MULTILINE);
    tm.search = 1;

    ret = 0;
    if (compiled->simple) {
        simple_match(&tm, compiled->simple);
    } else {
        if ((ret = vm_init(&tm, compiled->size)) != 0)
            goto cleanup;

        vm_execute(&tm, compiled, 0);
    }

    if (tm.match_end == 0) {
        if (start)
            *start = NULL;
        if (end)
            *end = NULL;
    } else {
        if (start)
            *start = sot + tm.match_start;
        if (end)
            *end = sot + (tm.match_end - 2);
        ret = 1;
    }

cleanup:
    vm_cleanup(&tm);
    return ret;
}

int rxvm_match (rxvm_t *compiled, char *input, int flags)
{
    char *sot;
    threads_t tm;
    int ret;

    if (!compiled || !input) return RXVM_EPARAM;

    memset(&tm, 0, sizeof(threads_t));
    tm.getchar = getchar_str;
    tm.getchar_data = &input;
    tm.endchar = '\0';

    sot = input;
    tm.multiline = 1;
    tm.nongreedy = 0;
    tm.icase = (flags & RXVM_ICASE);

    ret = 0;
    if (compiled->simple) {
        if (!simple_match(&tm, compiled->simple)) {
            goto cleanup;
        }
    } else {
        if ((ret = vm_init(&tm, compiled->size)) != 0)
            goto cleanup;

        if (vm_execute(&tm, compiled, 0))
            goto cleanup;
    }

    ret = tm.match_end && !*(sot + (tm.match_end - 1));

cleanup:
    vm_cleanup(&tm);
    return ret;
}

#ifndef NOEXTRAS

static int read_backwards (FILE *fp, char *buf, long size)
{
    size_t ret;

    if ((ret = fseek(fp, -size, SEEK_CUR)) < 0)
        return RXVM_IOERR;

    if ((ret = fread(buf, 1, size, fp)) < 0)
        return RXVM_IOERR;

    if (fseek(fp, -ret, SEEK_CUR) < 0)
        return RXVM_IOERR;

    return ret;
}

static long seek_to_start_of_line (FILE *fp)
{
    char buf[LBUF_SIZE];
    long curr;
    long seek_size;
    int ret;
    int i;

    curr = ftell(fp);
    if (curr == 0) {
        return 0;
    } else if (curr < 0) {
        return RXVM_IOERR;
    }

    /* Read backwards from current position in LBUF_SIZE sized chunks until
     * a newline character is found */
    while (curr) {
        seek_size = (curr < LBUF_SIZE) ? curr : LBUF_SIZE;
        ret = read_backwards(fp, buf, seek_size);

        if (ret < 0 || ret != seek_size)
            return RXVM_IOERR;

        curr -= seek_size;
        for (i = 0; i < seek_size; ++i) {
            if (buf[i] == '\n') {
                fseek(fp, i, SEEK_CUR);
                return curr + i;
            }
        }
    }

    return curr;
}

static int seek_to_start_of_match (threads_t *tm, FILE *fp, uint64_t *msize)
{
    if (msize) {
        *msize = tm->match_end - tm->match_start - 1;
    }

    if (fseek(fp, tm->match_start, SEEK_SET) < 0UL) {
        return RXVM_IOERR;
    }

    return 0;
}

static char getchar_file (void *data)
{
    size_t num;
    FILE *fp;

    fp = (FILE *)data;

    if (bufpos == FBUF_SIZE) {
        bufpos = 0;
        num = fread(fbuf, 1, FBUF_SIZE, fp);
        fpos += FBUF_SIZE;

        if (num < FBUF_SIZE) {
            fbuf[num] = EOF;
        }
    }

    return fbuf[bufpos++];
}

static int bmh_partial(FILE *fp, rxvm_t *compiled, threads_t *tm,
    uint64_t *msize)
{
    unsigned int lfix_size;
    int err;
    uint64_t reset;

    lfix_size = (compiled->lfixn - compiled->lfix0) + 1;
    bmh_init(fp, compiled->lfix, lfix_size);
    bmh(tm);

    while (tm->match_end) {
        /* re-start Boyer-Moore from here if vm_execute fails */
        reset = tm->match_end - 1;

        /* Reset file pointer to the start of the line containing the match,
         * to be ready for vm_execute */
        if ((err = seek_to_start_of_match(tm, fp, NULL)) < 0)
           return err;

        if ((fpos = seek_to_start_of_line(fp)) < 0)
           return fpos;

        tm->chars = fpos;
        tm->match_end = 0;
        vm_execute(tm, compiled, reset);

        if (tm->match_end) {
            return 1;
        }

        fseek(fp, reset, SEEK_SET);
        tm->match_end = 0;
        bmh_reset();
        bmh(tm);
    }

    return 0;
}

int rxvm_fsearch (rxvm_t *compiled, FILE *fp, uint64_t *match_size,
                  int flags)
{
    threads_t tm;
    int ret;
    char file_buffer[FBUF_SIZE];

    if (!compiled || !fp) return RXVM_EPARAM;
    if ((fpos = ftell(fp)) < 0) return RXVM_IOERR;

    memset(&tm, 0, sizeof(threads_t));
    tm.getchar = getchar_file;
    tm.getchar_data = fp;
    tm.endchar = EOF;

    fbuf = file_buffer;
    bufpos = FBUF_SIZE;
    tm.icase = (flags & RXVM_ICASE);
    tm.nongreedy = (flags & RXVM_NONGREEDY);
    tm.multiline = (flags & RXVM_MULTILINE);
    tm.search = 1;
    charcp = &tm.chars;

    ret = 0;
    if (compiled->simple) {
        /* Simple expression-- we can do the whole search using
         * Boyer-Moore (bmh()) */
        bmh_init(fp, compiled->simple, strlen(compiled->simple));
        bmh(&tm);
    } else {
        /* Full regular expression, with a fixed substring suitable for
         * running Boyer-Moore (bmh_partial()). Initialise the VM, since
         * bmh_partial will call vm_execute if it finds a substring match */
        if (compiled->lfix) {
            if ((ret = vm_init(&tm, compiled->size)) != 0)
                goto cleanup;

            if ((ret = bmh_partial(fp, compiled, &tm, match_size)) < 0)
                goto cleanup;

        } else {
            /* Full regular expression, with no fix substrings suitable for
             * Boyer-Moore. Initialise & execute the VM */
            if ((ret = vm_init(&tm, compiled->size)) != 0)
                goto cleanup;

            tm.chars = fpos;
            vm_execute(&tm, compiled, 0);
        }
    }

    if (tm.match_end) {
        seek_to_start_of_match(&tm, fp, match_size);
        ret = 1;
    }

cleanup:
    vm_cleanup(&tm);
    return ret;
}

void rxvm_print (rxvm_t *compiled)
{
    unsigned int i;
    inst_t inst;

    if (!compiled) return;

    if (compiled->simple) {
        printf("\"%s\" no compilation required\n", compiled->simple);
        return;
    }

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        switch(inst.op) {
            case OP_CHAR:
                printf("%d\tchar %c\n", i, inst.c);
            break;
            case OP_ANY:
                printf("%d\tany\n", i);
            break;
            case OP_SOL:
                printf("%d\tsol\n", i);
            break;
            case OP_EOL:
                printf("%d\teol\n", i);
            break;
            case OP_CLASS:
                printf("%d\tclass %s\n", i, inst.ccs);
            break;
            case OP_NCLASS:
                printf("%d\tnclass %s\n", i, inst.ccs);
            break;
            case OP_BRANCH:
                printf("%d\tbranch %d %d\n", i, inst.x, inst.y);
            break;
            case OP_JMP:
                printf("%d\tjmp %d\n", i, inst.x);
            break;
            case OP_MATCH:
                printf("%d\tmatch\n", i);
            break;
        }
    }
}

void rxvm_print_err (int err)
{
    const char *msg;

    switch (err) {
        case RXVM_BADOP:
            msg = "Operator used incorrectly";
        break;
        case RXVM_BADCLASS:
            msg = "Unexpected character class closing character";
        break;
        case RXVM_BADREP:
            msg = "Unexpected closing repetition character";
        break;
        case RXVM_BADPAREN:
            msg = "Unexpected parenthesis group closing character";
        break;
        case RXVM_EPAREN:
            msg = "Unterminated parenthesis group";
        break;
        case RXVM_ECLASS:
            msg = "Unterminated character class";
        break;
        case RXVM_EREP:
            msg = "Missing repetition closing character";
        break;
        case RXVM_ERANGE:
            msg = "Incomplete character range inside character class";
        break;
        case RXVM_ECOMMA:
            msg = "Invalid extra comma inside repetition";
        break;
        case RXVM_EDIGIT:
            msg = "Non-numerical character inside repetition";
        break;
        case RXVM_MREP:
            msg = "Empty repetition";
        break;
        case RXVM_ETRAIL:
            msg = "Trailing escape character";
        break;
        case RXVM_EINVAL:
            msg = "Invalid symbol";
        break;
        case RXVM_EMEM:
            msg = "Failed to allocate memory";
        break;
        case RXVM_EPARAM:
            msg = "Invalid parameter passed to library function";
        break;
        case RXVM_IOERR:
            msg = "rxvm_fsearch can't read from supplied FILE pointer";
        break;
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}
#endif /* NOEXTRAS */

void rxvm_free (rxvm_t *compiled)
{
    unsigned int i;

    if (!compiled) return;

    if (compiled->simple) {
        free(compiled->simple);
    } else if (compiled->exe) {
        for (i = 0; i < compiled->size; i++) {
            if (compiled->exe[i].ccs != NULL)
                free(compiled->exe[i].ccs);
        }

        free(compiled->exe);
    }
}
