#ifndef ERR_H_
#define ERR_H_

#define RVM_BADOP      -1   /* Operator used incorrectly */
#define RVM_BADCLASS   -2   /* Unexpected character class closing character */
#define RVM_BADPAREN   -3   /* Unexpected closing parenthesis */
#define RVM_EPAREN     -4   /* Unterminated parenthesis group */
#define RVM_ECLASS     -5   /* Unterminated character class */
#define RVM_ETRAIL     -6   /* Trailing escape character */
#define RVM_EMEM       -7   /* Failed to allocate memory */
#define RVM_ENEST      -8   /* Too many nested parenthesis groups */
#define RVM_ECLASSLEN  -9   /* Too many elements in character class */
#define RVM_EINVAL    -10   /* Invalid symbol */

#endif
