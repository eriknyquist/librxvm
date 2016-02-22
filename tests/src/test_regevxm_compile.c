#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

#define NUM_TESTS 8

typedef struct compv compv_t;

struct compv {
    char *rgx;
    char *cmp;
    unsigned int len;
};

static const compv_t test_1_basic_1 = {
    .rgx = "aab*",
    .cmp = "la:la:b5,3:lb:b5,3:m",
    .len = 6
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

static const compv_t test_6_basic_6 = {
    .rgx = "\\**\\++\\??\\.\\\\[*+?.\\\\]",
    .cmp = "b3,1:l*:b3,1:l+:b3,5:b6,7:l?:l.:l\\:c*+?.\\:m",
    .len = 11
};

static const compv_t test_7_nest_1 = {
    .rgx = "(a|b|c+)*b(x+|y(zs?(dd[A-F0-9]|bb)*)+)?",
    .cmp = "b10,1:b2,7:b3,5:la:j6:lb:j9:lc:b7,9:b10,1:lb:b12,30:b13,16:lx"
           ":b13,15:j30:ly:lz:b19,20:ls:b29,21:b22,26:ld:ld:cABCDEF0123456789"
           ":j28:lb:lb:b29,21:b17,30:m",
    .len = 31
};

static const compv_t test_8_nest_2 = {
    .rgx = "a(b(c(d(e(f(g(h(i(j(k(l(m(n(o(p(q)*)*)*)*)*)*)*)*)*)*)*)*)*)*)*)*",
    .cmp = "la:b49,2:lb:b48,4:lc:b47,6:ld:b46,8:le:b45,10:lf:b44,12:lg:b43,14"
           ":lh:b42,16:li:b41,18:lj:b40,20:lk:b39,22:ll:b38,24:lm:b37,26:ln"
           ":b36,28:lo:b35,30:lp:b34,32:lq:b34,32:b35,30:b36,28:b37,26:b38,24"
           ":b39,22:b40,20:b41,18:b42,16:b43,14:b44,12:b45,10:b46,8:b47,6"
           ":b48,4:b49,2:m",
    .len = 50
};

static const compv_t *tests[NUM_TESTS] = {
    &test_1_basic_1, &test_2_basic_2, &test_3_basic_3, &test_4_basic_4,
    &test_5_basic_5, &test_6_basic_6, &test_7_nest_1, &test_8_nest_2
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

int ccs_alloc (inst_t *inst, char **str)
{
    unsigned int len;
    unsigned int i;
    char *start;

    len = 0;
    start = *str;

    while (**str && **str != ':') {
        ++(*str);
        ++len;
    }

    if ((inst->ccs = malloc(sizeof(char) * (len + 1))) == NULL) {
        fprintf(stderr, "Failed to allocate mem. for ccs\n");
        return 1;
    }

    for (i = 0; i < len; i++) {
        inst->ccs[i] = start[i];
    }

    inst->ccs[len] = '\0';
    return 0;
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
    inst_t *inst;

    prog->size = 0;

    if ((prog->exe = malloc(sizeof(inst_t *) * len)) == NULL) {
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
                if (ccs_alloc(inst, &str))
                    return 1;
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

int cmpccs (char *ap, char *bp)
{
    int ret = 0;

    while (*ap) {
        if (!*bp) {
            ret = 1;
            break;
        }

        if (*(ap++) != *(bp++))
            ret = 1;
    }

    if (*bp)
        ret = 1;

    return ret;
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
            if (cmpccs(a->ccs, b->ccs))
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

    regexvm_free(&ex);
    regexvm_free(&compiled);
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
