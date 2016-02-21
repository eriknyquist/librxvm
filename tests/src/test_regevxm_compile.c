#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

#define NUM_TESTS 5

typedef struct compv compv_t;

struct compv {
    char *rgx;
    char *cmp;
    unsigned int len;
};

static const compv_t test_1_basic_1 = {
    .rgx = "ab*",
    .cmp = "la:b4,2:lb:b4,2:m",
    .len = 4
};

static const compv_t test_2_basic_2 = {
    .rgx = "ab+|d*(xx)?",
    .cmp = "b1,5:la:lb:b2,4:j11:b8,6:ld:b8,6:b9,11:lx:lx:m",
    .len = 12
};

static const compv_t test_3_basic_3 = {
    .rgx = "aa|bb|cc|dd",
    .cmp = "b1,12:b2,9:b3,6:la:la:j8:lb:lb:j11:lc:lc:j14:ld:ld:m",
    .len = 15
};

static const compv_t test_4_basic_4 = {
    .rgx = "(aa)bb(cc)",
    .cmp = "la:la:lb:lb:lc:lc:m",
    .len = 7
};

static const compv_t test_5_basic_5 = {
    .rgx = "(aa)bb(cc(dd(ee(ff))))",
    .cmp = "la:la:lb:lb:lc:lc:ld:ld:le:le:lf:lf:m",
    .len = 13
};

static const compv_t *tests[NUM_TESTS] = {
    &test_1_basic_1, &test_2_basic_2, &test_3_basic_3, &test_4_basic_4,
    &test_5_basic_5
};

unsigned int parse_int (char **str)
{
    unsigned int ret;

    ret = 0;
    while (**str && **str != ':' && **str != ',') {
        if (ret > 0)
            ret *= 10;

        ret += **str - 48;
        ++(*str);
    }

    return ret;
}

char *ccs_alloc (char **str)
{
    char *ret;
    unsigned int len;
    unsigned int i;
    char *start;

    len = 0;
    start = *str;

    while (**str && **str != ':') {
        ++*str;
        ++len;
    }

    if ((ret = malloc(sizeof(char) * (len + 1))) == NULL)
        return NULL;

    for (i = 0; i < len; i++) {
        ret[i] = start[i];
    }

    ret[len] = '\0';
    return ret;
}

inst_t *alloc_inst (void)
{
    inst_t *inst;

    if ((inst = malloc(sizeof(inst_t))) == NULL) {
        fprintf(stderr, "Failed to allocate mem. for instruction\n");
    }

    inst->op = 0;
    inst->c = 0;
    inst->ccs = NULL;
    inst->x = 0;
    inst->y = 0;

    return inst;
}

int str_to_prog (char *str, regexvm_t *prog, unsigned int len)
{
    size_t exesize;
    inst_t *inst;

    prog->size = 0;
    exesize = sizeof(inst_t *) * len;

    if ((prog->exe = malloc(exesize)) == NULL) {
        fprintf(stderr, "Failed to allocate mem. for exe list\n");
        return 1;
    }

    while (*str) {
        inst = alloc_inst();

        switch (*str) {
            case 'l':
                inst->op = OP_CHAR;
                inst->c = *(++str);
                do { ++str; } while (*str && *str != ':');
            break;
            case 'b':
                inst->op = OP_BRANCH;
                ++str;
                inst->x = parse_int(&str);
                ++str;
                inst->y = parse_int(&str);
            break;
            case 'j':
                inst->op = OP_JMP;
                ++str;
                inst->x = parse_int(&str);
            break;
            case 'c':
                inst->op = OP_CLASS;
                ++str;
                inst->ccs = ccs_alloc(&str);
            break;
            case 'a':
                inst->op = OP_ANY;
            break;
            case 'm':
                inst->op = OP_MATCH;
            break;
        }

        prog->exe[prog->size] = inst;
        ++prog->size;
        ++str;
    }

    return 0;
}

int cmpinst (inst_t *a, inst_t *b)
{
    int ret;

    ret = 0;
    if (a->op != b->op)
        return 1;

    switch (a->op) {
        case OP_CHAR:
            if (a->c != b->c)
                ret = 1;
        break;
        case OP_CLASS:
            while (*a->ccs) {
                if (!*b->ccs) {
                    ret = 1;
                    break;
                }

                if (*(a->ccs++) != *(b->ccs++))
                    ret = 1;
            }

            if (*b->ccs)
                ret = 1;
        break;
        case OP_BRANCH:
            if (a->x != b->x || a->y != b->y)
                ret = 1;
        break;
        case OP_JMP:
            if (a->x != b->x)
                ret = 1;
        break;
    }

    return ret;
}

int cmpexe (regexvm_t *compiled, regexvm_t *expected)
{
    int i;

    if (compiled->size != expected->size)
        return 1;

    for (i = 0; i < compiled->size; i++) {
        if (cmpinst(compiled->exe[i], expected->exe[i]))
            return 1;
    }

    return 0;
}

int verify_regexvm_cmp (char *regex, char *expected, unsigned int len)
{
    int ret;
    regexvm_t compiled;
    regexvm_t ex;

    if (str_to_prog(expected, &ex, len) != 0)
        return 1;

    printf("Testing pattern: %s\n", regex);
    if ((ret = regexvm_compile(&compiled, regex)) < 0)
        return ret;

    if (cmpexe(&compiled, &ex) != 0) {
        fprintf(stderr, "\nFail: instructions do not match expected\n");
        fprintf(stderr, "\nexpected:\n");
        regexvm_print(&ex);
        fprintf(stderr, "\nactual:\n");
        regexvm_print(&compiled);
        ret = 1;
    } else {
        printf("Passed.\n");
        ret = 0;
    }

    regexvm_free(&compiled);
    regexvm_free(&ex);
    return ret;
}

int test_regexvm_compile(void)
{
    int ret;
    int i;

    ret = 0;
    for (i = 0; i < NUM_TESTS; ++i) {
        ret += verify_regexvm_cmp(tests[i]->rgx, tests[i]->cmp, tests[i]->len);
    }

    return ret;
}
