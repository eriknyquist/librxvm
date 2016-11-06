#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "rxvm.h"
#include "test_common.h"

#define NUMNAMES        7

unsigned int rand_range (unsigned int low, unsigned int high)
{
    return (unsigned int) low + (rand() % ((high - low) + 1));
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
            msg = "Unexpected repetition closing character";
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
        case RXVM_MREP:
            msg = "Empty repetition";
        break;
        case RXVM_ETRAIL:
            msg = "Trailing escape character";
        break;
        case RXVM_EMEM:
            msg = "Failed to allocate memory";
        break;
        case RXVM_EINVAL:
            msg = "Unrecognised symbol";
        break;
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}

void rxvm_print_oneline (rxvm_t *compiled)
{
    unsigned int i;
    inst_t *inst;

    for (i = 0; i < compiled->size; i++) {
        inst = &compiled->exe[i];

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

    progsize = sizeof(rxvm_t) + (sizeof(inst_t) * size);
    vmsize = ((sizeof(int) * size) * 2) + (size * 2);
    printf("compiled size: %zu bytes\nVM size: %zu bytes\n", progsize, vmsize);
}
#endif

int compile_testexp (rxvm_t *compiled, char *exp)
{
    int ret;

    if ((ret = rxvm_compile(compiled, exp)) < 0)
        return ret;

#if (TEST_PRINT_SIZES)
    printf("expression: %s\n", exp);
    print_heap_usage(compiled->size);
    printf("\n");
#endif
    return 0;
}

void test_err (char *regex, char *input, const char *test, char *msg, int ret)
{
    FILE *fp;
    uint8_t file;
    char buf[80];

    snprintf(buf, sizeof(buf), "%s.err", test);
    if ((fp = fopen(buf, "a")) == NULL) {
        fp = stdout;
        file = 0;
        fprintf(fp, "Error opening log file %s: printing to stdout", buf);
    } else {
        file = 1;
    }

    fprintf(fp, "reason for failure : %s\n", msg);
    fprintf(fp, "         exit code : %d\n", ret);
    fprintf(fp, "        expression : %s\n", regex);
    fprintf(fp, "        input text : %s\n\n", input);
    if (file) fclose(fp);
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
