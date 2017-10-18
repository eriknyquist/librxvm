#ifndef RXVM_ERR_H
#define RXVM_ERR_H

#define RXVM_BADOP      -1   /* Operator used incorrectly */
#define RXVM_BADCLASS   -2   /* Unexpected character class closing character */
#define RXVM_BADREP     -3   /* Unexpected closing repetition character */
#define RXVM_BADPAREN   -4   /* Unexpected closing parenthesis */
#define RXVM_EPAREN     -5   /* Unterminated parenthesis group */
#define RXVM_ECLASS     -6   /* Unterminated character class */
#define RXVM_EREP       -7   /* Missing repetition closing character */
#define RXVM_ERANGE     -8   /* Incomplete character range */
#define RXVM_ECOMMA     -9   /* Invalid extra comma in repetition */
#define RXVM_EDIGIT     -10  /* Non-numerical character in repetition */
#define RXVM_MREP       -11  /* Empty repetition */
#define RXVM_ETRAIL     -12  /* Trailing escape character */
#define RXVM_EINVAL     -13  /* Invalid symbol */
#define RXVM_EMEM       -14  /* Failed to allocate memory */
#define RXVM_EPARAM     -15  /* Invalid parameter passed to library function */

#ifndef NOEXTRAS
#define RXVM_IOERR      -16  /* rxvm_fsearch file I/O error  */
#endif /* NOEXTRAS */

#endif
