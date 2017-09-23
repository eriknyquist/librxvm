#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"

void lfix_to_str (char *orig, rxvm_t *compiled)
{
    int end;
    int i;
    int bufi;

    bufi = 0;
    end = compiled->lfixn;

    for (i = compiled->lfix0; i <= end; ++i) {
        if (orig[i] == '\\') {
            ++end;
        } else {
            compiled->lfix[bufi++] = orig[i];
        }
    }

    compiled->lfix[bufi] = 0;
}
