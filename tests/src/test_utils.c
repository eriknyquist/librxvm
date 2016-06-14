#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "regexvm.h"
#include "lex.h"
#include "test_common.h"

#define NUMNAMES        7
#define BLOCK_SIZE      10

static unsigned int space;

unsigned int rand_range (unsigned int low, unsigned int high)
{
    return (unsigned int) low + (rand() % ((high - low) + 1));
}

static char *enlarge_if_needed (char *string, int size)
{
    char *temp;

    if ((size + 1) < space)
        return string;

    if ((temp = realloc(string, space + BLOCK_SIZE)) == NULL) {
        printf("Error: can't allocate any more memory.\n");
        printf("Current heap allocation size: %u bytes\n", space);
        exit(-1);
    }

    space += BLOCK_SIZE;
    return temp;
}

void regexvm_print_err (int err)
{
    const char *msg;

    switch (err) {
        case RVM_BADOP:
            msg = "Operator used incorrectly";
        break;
        case RVM_BADCLASS:
            msg = "Unexpected character class closing character";
        break;
        case RVM_BADREP:
            msg = "Unexpected repetition closing character";
        break;
        case RVM_BADPAREN:
            msg = "Unexpected parenthesis group closing character";
        break;
        case RVM_EPAREN:
            msg = "Unterminated parenthesis group";
        break;
        case RVM_ECLASS:
            msg = "Unterminated character class";
        break;
        case RVM_EREP:
            msg = "Missing repetition closing character";
        break;
        case RVM_MREP:
            msg = "Empty repetition";
        break;
        case RVM_ETRAIL:
            msg = "Trailing escape character";
        break;
        case RVM_EMEM:
            msg = "Failed to allocate memory";
        break;
        case RVM_EINVAL:
            msg = "Unrecognised symbol";
        break;
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}

void regexvm_print_oneline (regexvm_t *compiled)
{
    unsigned int i;
    inst_t *inst;

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        switch(inst->op) {
            case OP_CHAR:
                printf("l%c", inst->c);
            break;
            case OP_ANY:
                printf("a");
            break;
            case OP_SOL:
                printf("s");
            break;
            case OP_EOL:
                printf("e");
            break;
            case OP_CLASS:
                printf("c%s", inst->ccs);
            break;
            case OP_BRANCH:
                printf("b%d,%d", inst->x, inst->y);
            break;
            case OP_JMP:
                printf("j%d", inst->x);
            break;
            case OP_MATCH:
                printf("m");
            break;
        }

        if (i == (compiled->size - 1)) {
            printf("\n");
        } else {
            printf(":");
        }
    }
}

#if (TEST_PRINT_SIZES)
static void print_heap_usage (unsigned int size)
{
    size_t progsize;
    size_t vmsize;

    progsize = sizeof(regexvm_t) + (sizeof(inst_t) * size);
    vmsize = ((sizeof(int) * size) * 2) + (size * 2);
    printf("compiled size: %zu bytes\nVM size: %zu bytes\n", progsize, vmsize);
}
#endif

char *generate_matching_string (regexvm_t *compiled)
{
    char *ret;
    inst_t *inst;
    unsigned int ip;
    unsigned int size;
    unsigned int ix;

    if ((ret = malloc(BLOCK_SIZE)) == NULL)
        return NULL;

    space = BLOCK_SIZE;
    size = 0;
    ip = 0;

    inst = compiled->exe[ip];
    while (inst->op != OP_MATCH) {
        switch (inst->op) {
            case OP_CHAR:
                ret = enlarge_if_needed(ret, size);
                ret[size++] = inst->c;
                ++ip;
            break;
            case OP_ANY:
                ret = enlarge_if_needed(ret, size);
                ret[size++] = (char) rand_range(PRINTABLE_LOW, PRINTABLE_HIGH);
                ++ip;
            break;
            case OP_SOL:
                ret = enlarge_if_needed(ret, size);
                if (size && ret[size - 1] != '\n') {
                    ret[size++] = '\n';
                }
                ++ip;
            break;
            case OP_EOL:
                ret = enlarge_if_needed(ret, size);
                if (compiled->size > ip && (compiled->exe[ip + 1] != OP_CHAR
                            || compiled->exe[ip + 1]->c != '\n')) {
                    ret[size++] = '\n';
                }
                ++ip;
            break;
            case OP_CLASS:
                ret = enlarge_if_needed(ret, size);
                ix = rand_range(0, strlen(inst->ccs) - 1);
                ret[size++] = inst->ccs[ix];
                ++ip;
            break;
            case OP_BRANCH:
                ip = (rand_range(0, 1)) ? inst->x : inst->y;
            break;
            case OP_JMP:
                ip = inst->x;
            break;
        }

        inst = compiled->exe[ip];
    }

    ret[size] = '\0';
    return ret;
}

int compile_testexp (regexvm_t *compiled, char *exp)
{
    int ret;

    if ((ret = regexvm_compile(compiled, exp)) < 0)
        return ret;

#if (TEST_PRINT_SIZES)
    printf("expression: %s\n", exp);
    print_heap_usage(compiled->size);
    printf("\n");
#endif
    return 0;
}

char *hrsize (uint64_t size)
{
    static const char *names[NUMNAMES] =
    {
        "EB", "PB", "TB", "GB", "MB", "KB", "B"
    };
    char *ret;
    uint64_t mult;
    size_t retsize;
    int i;

    retsize = sizeof(char) * 20;
    mult = EXABYTES;
    ret = malloc(retsize);

    for (i = 0; i < NUMNAMES; ++i, mult /= 1024) {
        if (size < mult)
            continue;

        if ((size % mult) == 0) {
            snprintf(ret, retsize, "%"PRIu64" %s", size / mult, names[i]);
        } else {
            snprintf(ret, retsize, "%.2f %s", (float) size / mult, names[i]);
        }

        return ret;
    }

    strcpy(ret, "0");
    return ret;
}
