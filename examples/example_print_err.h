#ifndef _EXAMPLE_PRINT_ERR_H_
#define _EXAMPLE_PRINT_ERR_H_


void example_print_err (int err)
{
    const char *msg;

    switch (err) {
        case RXVM_BADOP:
            msg = "Operator used incorrectly";
        break;
        case RXVM_BADCLASS:
            msg = "Unexpected character class closing character";
        break;
        case RXVM_BADREP:
            msg = "Unexpected closing repetition character";
        break;
        case RXVM_BADPAREN:
            msg = "Unexpected parenthesis group closing character";
        break;
        case RXVM_EPAREN:
            msg = "Unterminated parenthesis group";
        break;
        case RXVM_ECLASS:
            msg = "Unterminated character class";
        break;
        case RXVM_EREP:
            msg = "Missing repetition closing character";
        break;
        case RXVM_MREP:
            msg = "Empty repetition";
        break;
        case RXVM_ETRAIL:
            msg = "Trailing escape character";
        break;
        case RXVM_EMEM:
            msg = "Failed to allocate memory";
        break;
        case RXVM_EINVAL:
            msg = "Invalid symbol";
        break;
        case RXVM_EPARAM:
            msg = "Invalid parameter passed to library function";
        break;
        default:
            msg = "Unrecognised error code";
    }

    printf("Error %d: %s\n", err, msg);
}

#endif
