#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

char *rgx;
char *input;

char *test_rgx =
"ww|(xx)aa(yy)?(bb*[abC-Z]|\\.|\\*|(.\\\\cc+(dd?[+*.?])*(ab*)?)+)?";
char *test_input = "xxaayy@\\ccccccccccccccccccccccdd?d?dd?d?d*d+d?ab";

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
        case RVM_ENEST:
            msg = "Too many nested parenthesis groups";
        break;
        case RVM_ECLASSLEN:
            msg = "Too many elements in character class";
        break;
        case RVM_EINVAL:
            msg = "Unrecognised symbol";
        break;
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}

int main (int argc, char *argv[])
{
    size_t size;
    int ret;
    regexvm_t compiled;

    ret = 0;
    if (argc == 1) {
        rgx = test_rgx;
        input = test_input;
    } else if (argc == 3) {
        rgx = argv[1];
        input = argv[2];
    } else {
        printf("Usage: %s [<regex> <input>]\n", argv[0]);
        exit(1);
    }

    if ((ret = regexvm_compile(&compiled, rgx)) < 0) {
        regexvm_print_err(ret);
        exit(ret);
    }

    regexvm_print(&compiled);
    size = sizeof(regexvm_t) + (sizeof(inst_t) * compiled.size);
    printf("inst_t size: %lu\ncompiled size: %lu\n", sizeof(inst_t), size);

    if (regexvm_match(&compiled, input)) {
        printf("Match!\n");
    } else {
        printf("No match.\n");
        ret = 1;
    }
    regexvm_free(&compiled);
    return ret;
}
