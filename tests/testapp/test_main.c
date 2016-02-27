#include <stdio.h>
#include <stdlib.h>
#include "regexvm.h"

char *rgx;
char *input;

char *test_rgx =
"ww|(xx)aa(yy)?(bb*[abC-Z]|\\.|\\*|(.\\\\cc+(dd?[+*.?])*(ab*)?)+)?";
char *test_input = "xxaayy@\\ccccccccccccccccccccccdd?d?dd?d?d*d+d?ab";

int main (int argc, char *argv[])
{
    size_t size;
    int ret;
    regexvm_t compiled;

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
#if (DEBUG)
        regexvm_print_err(ret);
#endif
        exit(ret);
    }

    size = sizeof(regexvm_t) + (sizeof(inst_t) * compiled.size);
    printf("inst_t size: %lu\ncompiled size: %lu\n", sizeof(inst_t), size);

    if (regexvm_match(&compiled, input)) {
        printf("Match!\n");
    } else {
        printf("No match.\n");
    }
    regexvm_free(&compiled);
    return 0;
}
