#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

#define NUM_TESTS 10

typedef struct compv compv_t;

struct compv {
    char *rgx;
    char *cmp;
};

static const compv_t test_1_basic_1 = {
    .rgx = "x*",
    .cmp = "b3,1:lx:b3,1:m"
};

static const compv_t test_2_basic_2 = {
    .rgx = "aab*",
    .cmp = "la:la:b5,3:lb:b5,3:m"
};

static const compv_t test_3_basic_3 = {
    .rgx = "ab+|d*(xx)?",
    .cmp = "b1,5:la:lb:b2,4:j11:b8,6:ld:b8,6:b9,11:lx:lx:m"
};

static const compv_t test_4_basic_4 = {
    .rgx = "aa|bb|cc|dd",
    .cmp = "b1,12:b2,9:b3,6:la:la:j8:lb:lb:j11:lc:lc:j14:ld:ld:m"
};

static const compv_t test_5_basic_5 = {
    .rgx = "(aa)bb(cc)",
    .cmp = "la:la:lb:lb:lc:lc:m"
};

static const compv_t test_6_basic_6 = {
    .rgx = "(aa)bb(cc(dd(ee(ff))))",
    .cmp = "la:la:lb:lb:lc:lc:ld:ld:le:le:lf:lf:m"
};

static const compv_t test_7_basic_7 = {
    .rgx = "\\**\\++\\??\\.\\\\[*+?.\\\\]",
    .cmp = "b3,1:l*:b3,1:l+:b3,5:b6,7:l?:l.:l\\:c*+?.\\:m"
};

static const compv_t test_8_nest_1 = {
    .rgx = "(a|b|c+)*b(x+|y(zs?(dd[A-F0-9]|bb)*)+)?",
    .cmp = "b10,1:b2,7:b3,5:la:j6:lb:j9:lc:b7,9:b10,1:lb:b12,30:b13,16:lx"
           ":b13,15:j30:ly:lz:b19,20:ls:b29,21:b22,26:ld:ld:cABCDEF0123456789"
           ":j28:lb:lb:b29,21:b17,30:m"
};

static const compv_t test_9_nest_2 = {
    .rgx = "a(b(c(d(e(f(g(h(i(j(k(l(m(n(o(p(q)*)*)*)*)*)*)*)*)*)*)*)*)*)*)*)*",
    .cmp = "la:b49,2:lb:b48,4:lc:b47,6:ld:b46,8:le:b45,10:lf:b44,12:lg:b43,14"
           ":lh:b42,16:li:b41,18:lj:b40,20:lk:b39,22:ll:b38,24:lm:b37,26:ln"
           ":b36,28:lo:b35,30:lp:b34,32:lq:b34,32:b35,30:b36,28:b37,26:b38,24"
           ":b39,22:b40,20:b41,18:b42,16:b43,14:b44,12:b45,10:b46,8:b47,6"
           ":b48,4:b49,2:m"
};

static const compv_t test_10_nest_3 = {
    .rgx = "a*a+(bb|c?d(ddd(ee|(ff)*)[a-f@.]g.*g*)+ss(ss[abc])*)xyz",
    .cmp = "b3,1:la:b3,1:la:b3,5:b6,9:lb:lb:j39:b10,11:lc:ld:ld:ld:ld:b16,19"
           ":le:le:j23:b23,20:lf:lf:b23,20:cabcdef@.:lg:b28,26:a:b28,26:b31,29"
           ":lg:b31,29:b12,32:ls:ls:b39,35:ls:ls:cabc:b39,35:lx:ly:lz:m"
};

static const compv_t *tests[NUM_TESTS] = {
    &test_1_basic_1, &test_2_basic_2, &test_3_basic_3, &test_4_basic_4,
    &test_5_basic_5, &test_6_basic_6, &test_7_basic_7, &test_8_nest_1,
    &test_9_nest_2, &test_10_nest_3
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

int ccs_cmp (inst_t *inst, char **str)
{

    int i;

    i = 0;
    while (inst->ccs[i]) {
        if (!**str || **str == ':'|| inst->ccs[i] != **str)
            return 1;
        ++*str;
        ++i;
    }

    return (**str == ':') ? 0 : 1;
}

int cmpexe (char *str, regexvm_t *prog)
{
    int i;
    inst_t *inst;

    for (i = 0; i < prog->size; i++) {
        inst = prog->exe[i];

        if (!*str)
            return 1;

        switch (*str) {
            case 'l':
                if (inst->op != OP_CHAR || inst->c != *(++str))
                    return i + 1;
                ++str;
            break;
            case 'b':
                if (inst->op != OP_BRANCH)
                    return i + 1;
                ++str;
                if (inst->x != parse_int(&str))
                    return i + 1;
                ++str;
                if (inst->y != parse_int(&str))
                    return i + 1;
            break;
            case 'j':
                if (inst->op != OP_JMP)
                    return i + 1;
                ++str;
                if (inst->x != parse_int(&str))
                    return i + 1;
            break;
            case 'c':
                if (inst->op != OP_CLASS)
                    return i + 1;
                ++str;
                if (ccs_cmp(inst, &str))
                    return i + 1;
            break;
            case 'a':
                if (inst->op != OP_ANY)
                    return i + 1;
                ++str;
            break;
            case 'm':
                if (inst->op != OP_MATCH)
                    return i + 1;
            break;
        }

        if (*str)
            ++str;
    }

    return (*str) ? i : 0;
}

void print_prog_str (char *str, int err)
{
    int i;

    i = 1;

    printf("0\t");
    while(*str) {
        if (*str == ':') {
            if (err == i) {
                printf(" <----");
            }
            printf("\n%d\t", i++);
        } else {
            printf("%c", *str);
        }
        ++str;
    }
    printf("\n");
}

void print_prog_cmp (regexvm_t *compiled, int err)
{
    int i;
    inst_t *inst;

    for (i = 0; i < compiled->size; i++) {
        inst = compiled->exe[i];

        switch(inst->op) {
            case OP_CHAR:
                printf("%d\tl%c", i, inst->c);
            break;
            case OP_ANY:
                printf("%d\ta", i);
            break;
            case OP_CLASS:
                printf("%d\tc%s", i, inst->ccs);
            break;
            case OP_BRANCH:
                printf("%d\tb%d,%d", i, inst->x, inst->y);
            break;
            case OP_JMP:
                printf("%d\tj%d", i, inst->x);
            break;
            case OP_MATCH:
                printf("%d\tm", i);
            break;
        }

        if ((i + 1) == err)
            printf(" <----");
        printf("\n");
    }
    printf("\n");
}

int verify_regexvm_cmp (char *expected, char *regex)
{
    int ret;
    int err;
    regexvm_t compiled;

    if ((ret = regexvm_compile(&compiled, regex)) < 0)
        return ret;

    if ((err = cmpexe(expected, &compiled)) != 0) {
        fprintf(stderr, "\nFail: instructions do not match expected\n");
        printf("\nexpected:\n\n");
        print_prog_str(expected, err);
        printf("\nseen:\n\n");
        print_prog_cmp(&compiled, err);
        ret = 1;
    } else {
        ret = 0;
    }

    regexvm_free(&compiled);
    return ret;
}

int test_regexvm_compile(void)
{
    int ret;
    int i;

    ret = 0;
    for (i = 0; i < NUM_TESTS; ++i) {
        ret += verify_regexvm_cmp(tests[i]->cmp, tests[i]->rgx);
        printf("%s: test %d %s\n", __func__, i + 1,
            (ret) ? "failed" : "passed");
    }

    return ret;
}
