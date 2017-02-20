#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "rxvm.h"
#include "string_builder.h"
#include "randexp.h"
#include "lex.h"

#define BLOCK_SIZE          50
#define MAX_REP             10

#define SIMPLE_CHOICES      2
#define BASIC_CHOICES       5
#define REPS_CHOICES        4
#define ELEM_CHOICES        4
/*#define ELEM_CHOICES        6 */
#define SETITEM_CHOICES     2

#define NUM_META            15

#define LITERALS            (((PRINTABLE_HIGH - PRINTABLE_LOW) + \
                            (WS_HIGH - WS_LOW)) - NUM_META)
static strb_t strb;
static randexp_cfg_t *cfg;
static const char *meta = "()*+-.?[]{|}^$\\";
static char charmap[LITERALS + NUM_META];

static void nonterm_re       (void);
static void nonterm_union    (void);
static void nonterm_simple   (void);
static void nonterm_concat   (void);
static void nonterm_basic    (void);
static void nonterm_zero     (void);
static void nonterm_one      (void);
static void nonterm_onezero  (void);
static void nonterm_rep      (void);
static void nonterm_reps     (void);
static void nonterm_repn     (void);
static void nonterm_reprange (void);
static void nonterm_repmore  (void);
static void nonterm_repless  (void);
static void nonterm_elem     (void);
static void nonterm_group    (void);
static void nonterm_any      (void);
/*static void nonterm_sol      (void);*/
/*static void nonterm_eol      (void);*/
static void nonterm_char     (void);
static void nonterm_char_ccs (void);
static void nonterm_set      (void);
static void nonterm_setitems (void);
static void nonterm_setitem  (void);
static void nonterm_range    (void);

typedef void (*nonterm_t)(void);

nonterm_t simple_choices[SIMPLE_CHOICES] = {nonterm_concat, nonterm_basic};
nonterm_t basic_choices[BASIC_CHOICES] =
    {nonterm_zero, nonterm_one, nonterm_onezero, nonterm_rep, nonterm_elem};
nonterm_t reps_choices[REPS_CHOICES] =
    {nonterm_repn, nonterm_reprange, nonterm_repmore, nonterm_repless};
nonterm_t elem_choices[ELEM_CHOICES] =
    {nonterm_group, nonterm_any, /*nonterm_eol, nonterm_sol,*/ nonterm_char,
    nonterm_set};
nonterm_t setitem_choices[SETITEM_CHOICES] = {nonterm_range, nonterm_char_ccs};

static unsigned int rand_range (unsigned int low, unsigned int high)
{
    return (unsigned int) low + (rand() % ((high - low) + 1));
}

static int ismeta (char c)
{
    int i;

    for (i = 0; meta[i]; ++i) {
        if (meta[i] == c)
            return 1;
    }

    return 0;
}

void init_charmap ()
{
    int i;
    char c;

    /* charmap starts with whitespace...*/
    for (i = 0; (i + WS_LOW) <= WS_HIGH; ++i) {
        charmap[i] = i + WS_LOW;
    }

    c = PRINTABLE_LOW;
    while (i < LITERALS) {
        if (!ismeta(c)) {
            charmap[i++] = c;
        }
        ++c;
    }

    for (i = 0; i < NUM_META; ++i) {
        charmap[LITERALS + i] = meta[i];
    }
}

static char get_rand_literal (uint8_t meta)
{
    unsigned int hi;

    hi = (meta) ? LITERALS + NUM_META : LITERALS;
    return charmap[rand_range(0, hi - 1)];
}

static int choice (int prob)
{
    return (rand_range(0, 100) < prob);
}

void nonterm_range (void)
{
    strb_addc(&strb, get_rand_literal(0));
    strb_addc(&strb, '-');
    strb_addc(&strb, get_rand_literal(0));
}

void nonterm_setitem (void)
{
    nonterm_t nonterm;

    if (choice(cfg->literals))
        nonterm = nonterm_char_ccs;
    else
        nonterm = setitem_choices[rand_range(0, (SETITEM_CHOICES -1))];

    nonterm();
}

void nonterm_setitems (void)
{
    uint8_t val;

    val = (strb.size >= cfg->limit) ? 0 : cfg->tokens;
    nonterm_setitem();
    if (choice(val)) nonterm_setitems();
}

void nonterm_set (void)
{
    strb_addc(&strb, '[');
    nonterm_setitems();
    strb_addc(&strb, ']');
}

void nonterm_char_ccs (void)
{
    char c;

    c = get_rand_literal(1);
    if (c == ']' || c == '-' || c == '\\') c = '*';
    strb_addc(&strb, c);
}

void nonterm_char (void)
{
    if (choice(cfg->escapes)) {
        strb_addc(&strb, '\\');
        strb_addc(&strb, meta[rand_range(0, NUM_META - 1)]);
    } else {
        strb_addc(&strb, get_rand_literal(0));
    }
}
/*
void nonterm_eol (void)
{
    strb_addc(&strb, '$');
}

void nonterm_sol (void)
{
    strb_addc(&strb, '^');
}
*/
void nonterm_any (void)
{
    strb_addc(&strb, '.');
}

void nonterm_group (void)
{
    strb_addc(&strb, '(');
    nonterm_re();
    strb_addc(&strb, ')');
}

void nonterm_elem (void)
{
    nonterm_t nonterm;

    if (choice(cfg->literals))
        nonterm = nonterm_char;
    else
        nonterm = elem_choices[rand_range(0, (ELEM_CHOICES - 1))];

    nonterm();
}

void nonterm_repless (void)
{
    strb_addc(&strb, ',');
    strb_addu(&strb, rand_range(1, MAX_REP));
}

void nonterm_repmore (void)
{
    strb_addu(&strb, rand_range(1, MAX_REP));
    strb_addc(&strb, ',');
}

void nonterm_reprange (void)
{
    strb_addu(&strb, rand_range(1, MAX_REP));
    strb_addc(&strb, ',');
    strb_addu(&strb, rand_range(1, MAX_REP));
}

void nonterm_repn (void)
{
    strb_addu(&strb, rand_range(1, MAX_REP));
}

void nonterm_reps (void)
{
    nonterm_t nonterm;

    nonterm = reps_choices[rand_range(0, (REPS_CHOICES - 1))];
    nonterm();
}

void nonterm_rep (void)
{
    nonterm_elem();
    strb_addc(&strb, '{');
    nonterm_reps();
    strb_addc(&strb, '}');
}

void nonterm_onezero (void)
{
    nonterm_elem();
    strb_addc(&strb, '?');
}

void nonterm_one (void)
{
    nonterm_elem();
    strb_addc(&strb, '+');
}

void nonterm_zero (void)
{
    nonterm_elem();
    strb_addc(&strb, '*');
}

void nonterm_basic (void)
{
    nonterm_t nonterm;

    if (choice(cfg->literals))
        nonterm = nonterm_elem;
    else
        nonterm = basic_choices[rand_range(0, (BASIC_CHOICES - 1))];

    nonterm();
}

void nonterm_concat (void)
{
    nonterm_simple();
    nonterm_basic();
}

void nonterm_simple (void)
{
    uint8_t val;

    val = (strb.size >= cfg->limit) ? 0 : cfg->tokens;

    if (choice(val)) {
        nonterm_concat();
    } else {
        nonterm_basic();
    }
}

void nonterm_union (void)
{
    nonterm_re();
    strb_addc(&strb, '|');
    nonterm_simple();
}

void nonterm_re (void)
{
    uint8_t val;

    val = (strb.size >= cfg->limit) ? 0 : cfg->tokens;

    if (choice(val)) {
        nonterm_union();
    } else {
        nonterm_simple();
    }
}

char *gen_randexp (randexp_cfg_t *randcfg, uint64_t *len)
{
    cfg = randcfg;
    strb_init(&strb, 128);

    nonterm_re();

    if (len) {
        *len = strb.size;
    }

    strb_addc(&strb, '\0');
    return strb.buf;
}
