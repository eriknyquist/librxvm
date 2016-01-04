#include <stdio.h>
#include <stdlib.h>
#include "lex.h"

int tok;

const char *token_string (int token)
{
    const char *ret;

    switch(tok) {
        case CHARC_OPEN:
            ret = "CHARC_OPEN";
        break;
        case CHARC_CLOSE:
            ret = "CHARC_CLOSE";
        break;
        case CHAR_RANGE:
            ret = "CHAR_RANGE";
        break;
        case LPAREN:
            ret = "LPAREN";
        break;
        case RPAREN:
            ret = "RPAREN";
        break;
        case ONE:
            ret = "ONE";
        break;
        case ZERO:
            ret = "ZERO";
        break;
        case ONEZERO:
            ret = "ONEZERO";
        break;
        case ANY:
            ret = "ANY";
        break;
        case LITERAL:
            ret = "LITERAL";
        break;
        case INVALIDSYM:
            ret = "ONE";
        break;

        default:
            ret = "unknown token";
        break;
    }

    return ret;
}

int main (void)
{
    char *regex = "(.[+*A-Z])+\\+*\\.+";

    printf("regex: %s\n\n", regex);
    printf("tokens:\n");
    while ((tok = lex(&regex)) != END) {
        printf("%s ", token_string(tok));
    }

    printf("\n");

    return 0;
}
