#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"

char *lfix_to_str (char *orig, rxvm_t *compiled)
{
    int size;
    int end;
    int i;
    int reti;
    char *ret;

    size = (compiled->lfixn - compiled->lfix0) + 1;

    if ( size < 2 || (ret = malloc(size + 1)) == NULL) {
        return NULL;
    }

    reti = 0;
    end = compiled->lfixn;

    for (i = compiled->lfix0; i <= end; ++i) {
        if (orig[i] == '\\') {
            ++end;
        } else {
            ret[reti++] = orig[i];
        }
    }

    ret[reti] = 0;
    return ret;
}
