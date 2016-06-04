#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"
#include "test_common.h"

typedef struct compv compv_t;

struct compv {
    char *rgx;
    char *cmp;
};

static const compv_t test_1_basic_1 = {
    .rgx = "x*",
    .cmp = "b1,3:lx:b1,3:m"
};

static const compv_t test_2_basic_2 = {
    .rgx = "aab*",
    .cmp = "la:la:b3,5:lb:b3,5:m"
};

static const compv_t test_3_basic_3 = {
    .rgx = "ab+|d*(xx)?",
    .cmp = "b1,5:la:lb:b2,4:j11:b6,8:ld:b6,8:b9,11:lx:lx:m"
};

static const compv_t test_4_basic_4 = {
    .rgx = "aa|bb|cc|dd",
    .cmp = "b1,12:b2,9:b3,6:la:la:j14:lb:lb:j14:lc:lc:j14:ld:ld:m"
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
    .cmp = "b1,3:l*:b1,3:l+:b3,5:b6,7:l?:l.:l\\:c*+?.\\:m"
};

static const compv_t test_8_nest_1 = {
    .rgx = "(a|b|c+)*b(x+|y(zs?(dd[A-F0-9]|bb)*)+)?",
    .cmp = "b1,10:b2,7:b3,5:la:j9:lb:j9:lc:b7,9:b1,10:lb:b12,30:b13,16:lx"
           ":b13,15:j30:ly:lz:b19,20:ls:b21,29:b22,26:ld:ld:cABCDEF0123456789"
           ":j28:lb:lb:b21,29:b17,30:m"
};

static const compv_t test_9_nest_2 = {
    .rgx = "a(b(c(d(e(f(g(h(i(j(k(l(m(n(o(p(q)*)*)*)*)*)*)*)*)*)*)*)*)*)*)*)*",
    .cmp = "la:b2,49:lb:b4,48:lc:b6,47:ld:b8,46:le:b10,45:lf:b12,44:lg:b14,43"
           ":lh:b16,42:li:b18,41:lj:b20,40:lk:b22,39:ll:b24,38:lm:b26,37:ln"
           ":b28,36:lo:b30,35:lp:b32,34:lq:b32,34:b30,35:b28,36:b26,37:b24,38"
           ":b22,39:b20,40:b18,41:b16,42:b14,43:b12,44:b10,45:b8,46:b6,47"
           ":b4,48:b2,49:m"
};

static const compv_t test_10_nest_3 = {
    .rgx = "a*a+(bb|c?d(ddd(ee|(ff)*)[a-f@.]g.*g*)+ss(ss[abc])*)xyz",
    .cmp = "b1,3:la:b1,3:la:b3,5:b6,9:lb:lb:j39:b10,11:lc:ld:ld:ld:ld:b16,19"
           ":le:le:j23:b20,23:lf:lf:b20,23:cabcdef@.:lg:b26,28:a:b26,28:b29,31"
           ":lg:b29,31:b12,32:ls:ls:b35,39:ls:ls:cabc:b35,39:lx:ly:lz:m"
};

static const compv_t test_11_nest_4 = {
    .rgx = "w(e(r(t(y(u(i(p(o(i(u(y(q(q(w(e(r(tt(f(d(f(xx(g(g(ft(y(u(j(j(j(j("
           "j(j(sd(sd(d(sdds(sefjy(hfdsd(s(s(d(d(f(g(h(j(;(l(k(j(h(h(g(f(d(s("
           "a)*)*)+)*)*)?)?)?)*)+)+)+)+)*)*)*)+)?)?)?)?)?)?)?)?)*)+)+)+)*)?)+"
           ")*)*)?)+)*)*)+)?)*)*)?)+)+)+)+)+)*)*)*)+)*)+)?)+)?",
    .cmp = "lw:b2,151:le:lr:b5,150:lt:ly:b8,149:lu:li:b11,147:lp:b13,146:lo"
           ":b15,145:li:lu:ly:lq:lq:lw:b22,139:le:b24,139:lr:b26,138:lt:lt"
           ":b29,137:lf:ld:b32,136:lf:b34,135:lx:lx:lg:b38,133:lg:b40,133:lf"
           ":lt:b43,132:ly:lu:b46,130:lj:b48,130:lj:lj:lj:lj:b53,126:lj"
           ":b55,125:ls:ld:b58,125:ls:ld:b61,125:ld:b63,125:ls:ld:ld:ls"
           ":b68,125:ls:le:lf:lj:ly:b74,125:lh:lf:ld:ls:ld:b80,125:ls:b82,125"
           ":ls:ld:b85,124:ld:b87,123:lf:b89,122:lg:lh:lj:l;:ll:b95,117:lk"
           ":b97,116:lj:b99,116:lh:b101,116:lh:b103,116:lg:b105,115:lf:ld"
           ":b108,113:ls:b110,112:la:b110,112:b108,113:b106,114:b105,115"
           ":b103,116:b95,117:b93,118:b92,119:b91,120:b90,121:b89,122:b87,123"
           ":b85,124:b83,125:b53,126:b51,127:b50,128:b49,129:b48,130:b44,131"
           ":b43,132:b40,133:b36,134:b34,135:b32,136:b30,137:b26,138:b24,139"
           ":b20,140:b19,141:b18,142:b17,143:b16,144:b15,145:b13,146:b11,147"
           ":b9,148:b8,149:b6,150:b3,151:m"
};

static const compv_t test_12_rep_1 = {
    .rgx = "abc{3}",
    .cmp = "la:lb:lc:lc:lc:m"
};

static const compv_t test_13_rep_2 = {
    .rgx = "xyz{,4}",
    .cmp = "lx:ly:b3,10:lz:b5,10:lz:b7,10:lz:b9,10:lz:m"
};

static const compv_t test_14_rep_3 = {
    .rgx = "yyd{7,}",
    .cmp = "ly:ly:ld:ld:ld:ld:ld:ld:ld:b8,10:m"
};

static const compv_t test_15_rep_4 = {
    .rgx = "yyd{5,6}",
    .cmp = "ly:ly:ld:ld:ld:ld:ld:b8,9:ld:m"
};

static const compv_t test_16_rep_5 = {
    .rgx = "efx{0,}",
    .cmp = "le:lf:b3,5:lx:b3,5:m"
};

static const compv_t test_17_rep_6 = {
    .rgx = "abc*(de+f{4,5}|xxyy){,3}",
    .cmp = "la:lb:b3,5:lc:b3,5:b6,53:b7,17:ld:le:b8,10:lf:lf:lf:lf:b15,16:lf:"
           "j21:lx:lx:ly:ly:b22,53:b23,33:ld:le:b24,26:lf:lf:lf:lf:b31,32:lf:"
           "j37:lx:lx:ly:ly:b38,53:b39,49:ld:le:b40,42:lf:lf:lf:lf:b47,48:lf:"
           "j53:lx:lx:ly:ly:m"
};

static const compv_t test_18_rep_7 = {
    .rgx = "a*(b{2,3}p+(ffl?p){4}(hh(ee(yuy){5,}){,8}){5}){1,2}",
    .cmp = "b1,3:la:b1,3:lb:lb:b6,7:lb:lp:b7,9:lf:lf:b12,13:ll:lp:lf:lf:b17,18"
          ":ll:lp:lf:lf:b22,23:ll:lp:lf:lf:b27,28:ll:lp:lh:lh:b32,183:le:le:ly"
          ":lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b46,50:b51,183:le:le:ly:"
          "lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b65,69:b70,183:le:le:ly:l"
          "u:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b84,88:b89,183:le:le:ly:lu"
          ":ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b103,107:b108,183:le:le:ly:"
          "lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b122,126:b127,183:le:le:l"
          "y:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b141,145:b146,183:le:le"
          ":ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b160,164:b165,183:le:"
          "le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b179,183:lh:lh:b186"
          ",337:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b200,204:b2"
          "05,337:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b219,223:"
          "b224,337:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b238,24"
          "2:b243,337:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b257,"
          "261:b262,337:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b27"
          "6,280:b281,337:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b"
          "295,299:b300,337:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly"
          ":b314,318:b319,337:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:"
          "ly:b333,337:lh:lh:b340,491:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:l"
          "y:ly:lu:ly:b354,358:b359,491:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu"
          ":ly:ly:lu:ly:b373,377:b378,491:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:"
          "lu:ly:ly:lu:ly:b392,396:b397,491:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:l"
          "y:lu:ly:ly:lu:ly:b411,415:b416,491:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly"
          ":ly:lu:ly:ly:lu:ly:b430,434:b435,491:le:le:ly:lu:ly:ly:lu:ly:ly:lu:"
          "ly:ly:lu:ly:ly:lu:ly:b449,453:b454,491:le:le:ly:lu:ly:ly:lu:ly:ly:l"
          "u:ly:ly:lu:ly:ly:lu:ly:b468,472:b473,491:le:le:ly:lu:ly:ly:lu:ly:ly"
          ":lu:ly:ly:lu:ly:ly:lu:ly:b487,491:lh:lh:b494,645:le:le:ly:lu:ly:ly:"
          "lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b508,512:b513,645:le:le:ly:lu:ly:l"
          "y:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b527,531:b532,645:le:le:ly:lu:ly"
          ":ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b546,550:b551,645:le:le:ly:lu:"
          "ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b565,569:b570,645:le:le:ly:l"
          "u:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b584,588:b589,645:le:le:ly"
          ":lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b603,607:b608,645:le:le:"
          "ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b622,626:b627,645:le:l"
          "e:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b641,645:lh:lh:b648,"
          "799:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b662,666:b66"
          "7,799:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b681,685:b"
          "686,799:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b700,704"
          ":b705,799:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b719,7"
          "23:b724,799:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b738"
          ",742:b743,799:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b7"
          "57,761:b762,799:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:"
          "b776,780:b781,799:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:l"
          "y:b795,799:b800,1596:lb:lb:b803,804:lb:lp:b804,806:lf:lf:b809,810:l"
          "l:lp:lf:lf:b814,815:ll:lp:lf:lf:b819,820:ll:lp:lf:lf:b824,825:ll:lp"
          ":lh:lh:b829,980:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:"
          "b843,847:b848,980:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:l"
          "y:b862,866:b867,980:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu"
          ":ly:b881,885:b886,980:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:"
          "lu:ly:b900,904:b905,980:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:l"
          "y:lu:ly:b919,923:b924,980:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly"
          ":ly:lu:ly:b938,942:b943,980:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:"
          "ly:ly:lu:ly:b957,961:b962,980:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:l"
          "u:ly:ly:lu:ly:b976,980:lh:lh:b983,1134:le:le:ly:lu:ly:ly:lu:ly:ly:l"
          "u:ly:ly:lu:ly:ly:lu:ly:b997,1001:b1002,1134:le:le:ly:lu:ly:ly:lu:ly"
          ":ly:lu:ly:ly:lu:ly:ly:lu:ly:b1016,1020:b1021,1134:le:le:ly:lu:ly:ly"
          ":lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1035,1039:b1040,1134:le:le:ly:lu"
          ":ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1054,1058:b1059,1134:le:le"
          ":ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1073,1077:b1078,1134"
          ":le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1092,1096:b109"
          "7,1134:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1111,111"
          "5:b1116,1134:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b11"
          "30,1134:lh:lh:b1137,1288:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:"
          "ly:lu:ly:b1151,1155:b1156,1288:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:"
          "lu:ly:ly:lu:ly:b1170,1174:b1175,1288:le:le:ly:lu:ly:ly:lu:ly:ly:lu:"
          "ly:ly:lu:ly:ly:lu:ly:b1189,1193:b1194,1288:le:le:ly:lu:ly:ly:lu:ly:"
          "ly:lu:ly:ly:lu:ly:ly:lu:ly:b1208,1212:b1213,1288:le:le:ly:lu:ly:ly:"
          "lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1227,1231:b1232,1288:le:le:ly:lu:"
          "ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1246,1250:b1251,1288:le:le:"
          "ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1265,1269:b1270,1288:"
          "le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1284,1288:lh:lh"
          ":b1291,1442:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b130"
          "5,1309:b1310,1442:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:l"
          "y:b1324,1328:b1329,1442:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:l"
          "y:lu:ly:b1343,1347:b1348,1442:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:l"
          "u:ly:ly:lu:ly:b1362,1366:b1367,1442:le:le:ly:lu:ly:ly:lu:ly:ly:lu:l"
          "y:ly:lu:ly:ly:lu:ly:b1381,1385:b1386,1442:le:le:ly:lu:ly:ly:lu:ly:l"
          "y:lu:ly:ly:lu:ly:ly:lu:ly:b1400,1404:b1405,1442:le:le:ly:lu:ly:ly:l"
          "u:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1419,1423:b1424,1442:le:le:ly:lu:l"
          "y:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1438,1442:lh:lh:b1445,1596:l"
          "e:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1459,1463:b1464,"
          "1596:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1478,1482:"
          "b1483,1596:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:b1497"
          ",1501:b1502,1596:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly"
          ":b1516,1520:b1521,1596:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly"
          ":lu:ly:b1535,1539:b1540,1596:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly:ly:lu"
          ":ly:ly:lu:ly:b1554,1558:b1559,1596:le:le:ly:lu:ly:ly:lu:ly:ly:lu:ly"
          ":ly:lu:ly:ly:lu:ly:b1573,1577:b1578,1596:le:le:ly:lu:ly:ly:lu:ly:ly"
          ":lu:ly:ly:lu:ly:ly:lu:ly:b1592,1596:m"
};

static const compv_t *cmp_tests[NUM_TESTS_COMPILE] = {
    &test_1_basic_1, &test_2_basic_2, &test_3_basic_3, &test_4_basic_4,
    &test_5_basic_5, &test_6_basic_6, &test_7_basic_7, &test_8_nest_1,
    &test_9_nest_2, &test_10_nest_3, &test_11_nest_4, &test_12_rep_1,
    &test_13_rep_2, &test_14_rep_3, &test_15_rep_4, &test_16_rep_5,
    &test_17_rep_6, &test_18_rep_7
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
            case 's':
                if (inst->op != OP_SOL)
                    return i + 1;
            break;
            case 'e':
                if (inst->op != OP_EOL)
                    return i + 1;
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
                printf("%d\tchar %c", i, inst->c);
            break;
            case OP_ANY:
                printf("%d\tany", i);
            break;
            case OP_CLASS:
                printf("%d\tclass %s", i, inst->ccs);
            break;
            case OP_BRANCH:
                printf("%d\tbranch %d %d", i, inst->x, inst->y);
            break;
            case OP_JMP:
                printf("%d\tjmp %d", i, inst->x);
            break;
            case OP_MATCH:
                printf("%d\tmatch", i);
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

    if ((ret = compile_testexp(&compiled, regex)) < 0)
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

int test_regexvm_compile (int *count)
{
    const char *msg;
    int ret;
    int err;
    int i;

    ret = 0;
    for (i = 0; i < NUM_TESTS_COMPILE; ++i) {
        if ((err = verify_regexvm_cmp(cmp_tests[i]->cmp, cmp_tests[i]->rgx))) {
            ret += err;
            msg = "not ok";
        } else {
            msg = "ok";
        }

        printf("%s %d %s\n", msg, *count, __func__);
        ++(*count);
    }

    return ret;
}
