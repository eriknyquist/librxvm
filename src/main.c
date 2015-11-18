#include <stdio.h>
#include <stdlib.h>
#include "lex.h"


int main (int argc, char *argv[])
{
    const char *msg;
    char *text;
    int tok;

    if (argc != 2) {
        printf("Usage: %s <regex>\n", argv[0]);
        return 1;
    }

    /* get next token */
    tok = lex(&argv[1]);
    while (tok != END) {
        switch (tok) {
            case CHARC_OPEN:
                msg = "open char. class";
            break;
            case CHARC_CLOSE:
                msg = "close char. class";
            break;
            case CHAR_RANGE:
                msg = "char. range";
            break;
            case ONE:
                msg = "one or more";
            break;
            case ZERO:
                msg = "zero or more";
            break;
            case ANY:
                msg = "any character";
            break;
            case LITERAL:
                msg = "literal";
            break;
            case INVALIDSYM:
                msg = "invalid symbol";
            break;
        }

        printf("%20s: ", msg);
        text = get_token_text();
        printf("%s\n", text);
        free(text);

        if (tok == INVALIDSYM) return 1;

        /* get next token */
        tok = lex(&argv[1]);
    }
    return 0;
}
