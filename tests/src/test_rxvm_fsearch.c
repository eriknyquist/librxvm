#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "rxvm.h"
#include "test_common.h"

#define MIN_FSIZE   80
#define LIMIT       256
#define GENEROSITY  85
#define WHITESPACE  10

#define TMPFILE     ".__test_rxvm_fsearch_testinput.txt"
#define FIRST       0
#define LAST        1
#define MIDDLE      2

static FILE *fp;
static char *gen;
static const char *func;
static rxvm_t compiled;
static rxvm_gencfg_t cfg;
static int count;

static void log_trs (char *msg, const char *func)
{
    fprintf(trsfp, ":test-result: %s %s #%d\n", msg, func, count);
}

static int cleanup_testfile (void)
{
    if (access(TMPFILE, F_OK) != -1) {
        return remove(TMPFILE);
    }

    return 0;
}

static int fill_padding (const char *pad, unsigned padlen, unsigned count,
                         FILE *fp)
{
    unsigned int write_size;

    while (count) {
        write_size = (count < padlen) ? count : padlen;

        if (fwrite(pad, 1, write_size, fp) != write_size) {
            log_trs("FAIL", func);
            fprintf(logfp, "Error while writing padding to test file\n");
            return -1;
        }

        count -= write_size;
    }

    return 0;
}

void fail_and_cleanup (void)
{
    log_trs("FAIL", func);
    rxvm_free(&compiled);
    free(gen);

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    cleanup_testfile();
}

int generate_test_file (FILE *fp, char *gen, char *pad, int position)
{
    unsigned int fsize;
    unsigned int padlen;
    unsigned int genlen;
    unsigned int midpoint;
    unsigned int firsthalf;
    unsigned int secondhalf;
    unsigned int matchpos;

    if ((fp = fopen(TMPFILE, "w")) == NULL) {
        fprintf(logfp, "Can't open test file "TMPFILE" for writing\n");
        return -1;
    }

    padlen = strlen(pad);
    genlen = cfg.len - 1;

    fsize = MIN_FSIZE;
    while (fsize <= genlen)
        fsize *= 2;

    while (1) {
        midpoint = (fsize / 2) - (genlen / 2);
        firsthalf = midpoint;
        secondhalf = fsize - midpoint - genlen;

        if (secondhalf > genlen)
            break;

        fsize *= 2;
    }

    if (position == FIRST) {
        fwrite(gen, 1, genlen, fp);
        firsthalf -= genlen;
        matchpos = 0;
    }

    if (fill_padding(pad, padlen, firsthalf, fp) != 0) return -1;

    if (position == MIDDLE) {
        fwrite(gen, 1, genlen, fp);
        secondhalf -= genlen;
        matchpos = midpoint;
    }

    if (fill_padding(pad, padlen, secondhalf, fp) != 0) return -1;

    if (position == LAST) {
        fwrite(gen, 1, genlen, fp);
        matchpos = fsize - genlen;
    } else {
        if (fill_padding(pad, padlen, genlen, fp) != 0) return -1;
    }

    fclose(fp);
    return matchpos;
}

static void verify_fsearch (char *regex, char *pad, int flags, int position)
{
    char match[LIMIT * 2];
    uint64_t msize;
    int pos;
    long fpos;
    int err;

    if ((err = rxvm_compile(&compiled, regex)) < 0) {
        log_trs("FAIL", func);
        fprintf(logfp, "Error: compilation failed (%d): %s\n",
                err, regex);
        return;
    }

    if ((gen = rxvm_gen(&compiled, &cfg)) == NULL) {
        log_trs("FAIL", func);
        fprintf(logfp, "Memory allocation failed during input generation");
        rxvm_free(&compiled);
        return;
    }

    if ((pos = generate_test_file(fp, gen, pad, position)) < 0) {
        fail_and_cleanup();
        return;
    }

    if ((fp = fopen(TMPFILE, "r")) == NULL) {
        fprintf(logfp, "Can't open test file "TMPFILE" for reading\n");
        fail_and_cleanup();
        return;
    }

    if ((err = rxvm_fsearch(&compiled, fp, &msize, flags)) < 0) {
        fprintf(logfp, "verify_fsearch: regex=%s (flags=%d):"
                " errcode=%d\n", regex, flags, err);
        fail_and_cleanup();
        return;
    } else if (err == 0) {
        fprintf(logfp, "verify_fsearch:\n\nregex=%s\ninput=%s\nflags=%d\nNo"
                       " match found in file %s\n", regex, gen, flags, TMPFILE);
        fail_and_cleanup();
        return;
    }

    fpos = ftell(fp);
    if (fpos!= pos) {
        fprintf(logfp, "verify_fsearch:\n\nregex=%s\ninput=%s\nflags=%d\nmatch"
                       " offset is %ld, expecting %d\n", regex, gen, flags,
                       fpos, pos);
        fail_and_cleanup();
        return;
    }

    if (fread(match, 1, msize, fp) != msize) {
        fprintf(logfp, "Error reading match from test file");
        fail_and_cleanup();
        return;
    }

    fclose(fp);
    fp = NULL;
    match[msize] = 0;

    if (strcmp(match, gen) != 0) {
        fprintf(logfp, "verify_fsearch:\n\nregex=%s\ninput=%s\nflags=%d\nmatch"
                       "should be:\n%s\n\nbut got this:\n%s\n\n", regex, gen,
                       flags, gen, match);
        fail_and_cleanup();
        return;
    }

    if (cleanup_testfile() != 0) {
        log_trs("FAIL", func);
        fprintf(logfp, "Failed to clean up test file "TMPFILE"\n");
        return;
    }

    rxvm_free(&compiled);
    free(gen);

    log_trs("PASS", func);
    printf("PASS: %s #%d\n", func, ++count);
}

void test_rxvm_fsearch (void)
{
    fp = NULL;
    count = 0;
    func = __func__;
    srand(time(NULL));

    if (cleanup_testfile() != 0) {
        log_trs("FAIL", func);
        fprintf(logfp, "Failed to clean up test file "TMPFILE"\n");
    }

    cfg.limit = LIMIT;
    cfg.generosity = GENEROSITY;
    cfg.whitespace = WHITESPACE;

    verify_fsearch("xyz", "abc", 0, FIRST);
    verify_fsearch("xyz", "abc", 0, LAST);
    verify_fsearch("xyz", "abc", 0, MIDDLE);
    verify_fsearch("abcdef", "abcde", 0, FIRST);
    verify_fsearch("abcdef", "abcde", 0, LAST);
    verify_fsearch("abcdef", "abcde", 0, MIDDLE);
    verify_fsearch("[^A-Za-z]+", "abc", 0, FIRST);
    verify_fsearch("[^A-Za-z]+", "abc", 0, LAST);
    verify_fsearch("[^A-Za-z]+", "abc", 0, MIDDLE);
    verify_fsearch("[a-f\\-]+-", "ABCDEF", 0, FIRST);
    verify_fsearch("[a-f\\-]+-", "ABCDEF", 0, LAST);
    verify_fsearch("[a-f\\-]+-", "ABCDEF", 0, MIDDLE);
    verify_fsearch("[[*-+?]+", "@#$", 0, FIRST);
    verify_fsearch("[[*-+?]+", "@#$", 0, LAST);
    verify_fsearch("[[*-+?]+", "@#$", 0, MIDDLE);
    verify_fsearch("a+", "bc", 0, FIRST);
    verify_fsearch("a+", "bc", 0, LAST);
    verify_fsearch("a+", "bc", 0, MIDDLE);
    verify_fsearch("a+|bb?|cc*|d{12}", "ef", 0, FIRST);
    verify_fsearch("a+|bb?|cc*|d{12}", "ef", 0, LAST);
    verify_fsearch("a+|bb?|cc*|d{12}", "ef", 0, MIDDLE);
    verify_fsearch("<[^zxcv]*>", "zxcv", 0, FIRST);
    verify_fsearch("<[^zxcv]*>", "zxcv", 0, LAST);
    verify_fsearch("<[^zxcv]*>", "zxcv", 0, MIDDLE);
    verify_fsearch("v+(dd*|[A-F]){4,}", "zxcq", 0, FIRST);
    verify_fsearch("v+(dd*|[A-F]){4,}", "zxcq", 0, LAST);
    verify_fsearch("v+(dd*|[A-F]){4,}", "zxcq", 0, MIDDLE);
    verify_fsearch("([*]+[+]?\\?){8,10}", ".:.:", 0, FIRST);
    verify_fsearch("([*]+[+]?\\?){8,10}", ".:.:", 0, LAST);
    verify_fsearch("([*]+[+]?\\?){8,10}", ".:.:", 0, MIDDLE);
}
