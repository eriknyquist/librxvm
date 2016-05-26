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

static unsigned int rand_range (unsigned int low, unsigned int high)
{
    return (unsigned int) low + (rand() % ((high - low) + 1));
}

static char *enlarge_if_needed (char *string, int size)
{
    char *temp;

    if ((size + 1) < space)
        return string;

    if ((temp = realloc(string, space + BLOCK_SIZE)) == NULL)
        return string;

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
        case RVM_BADPAREN:
            msg = "Unexpected parenthesis group closing character";
        break;
        case RVM_EPAREN:
            msg = "Unterminated parenthesis group";
        break;
        case RVM_ECLASS:
            msg = "Unterminated character class";
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
    unsigned int ip;
    unsigned int size;
    unsigned int ix;
    char *ret;
    inst_t *inst;

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
