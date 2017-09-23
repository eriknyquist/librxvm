#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rxvm.h"
#include "test_common.h"

#define LINE_WIDTH  80
#define MIN_FSIZE   65536
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
    unsigned int linecount;

    linecount = 0;

    while (count) {
        write_size = (count < padlen) ? count : padlen;

        if (fwrite(pad, 1, write_size, fp) != write_size) {
            log_trs("FAIL", func);
            fprintf(logfp, "Error while writing padding to test file\n");
            return -1;
        }

        count -= write_size;
        linecount += write_size;

        if (count && linecount >= LINE_WIDTH) {
            fputc('\n', fp);
            linecount = 0;
            --count;
        }
    }

    return 0;
}

void fail_and_cleanup (char *failname)
{
    rxvm_free(&compiled);
    free(gen);

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    if (failname)
        rename(TMPFILE, failname);
    else
        cleanup_testfile();

    log_trs("FAIL", func);
    printf("FAIL: %s #%d\n", func, ++count);
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

    /* Make sure the whole file is big enough for the
     * the generated matching text */
    fsize = MIN_FSIZE;
    while (fsize <= genlen)
        fsize *= 2;

    /* Make sure the 2nd half of the file is big enough
     * for the the generated matching text-- in case
     * MIDDLE or LAST is used */
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
    char failed_input[48];
    char match[LIMIT * 2];
    uint64_t msize;
    int pos;
    long fpos;
    int err;

    snprintf(failed_input, sizeof(failed_input),
        "rxvm_fsearch_failed_input_%d.txt", count);

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
        fail_and_cleanup(NULL);
        return;
    }

    if ((fp = fopen(TMPFILE, "r")) == NULL) {
        fprintf(logfp, "Can't open test file "TMPFILE" for reading\n");
        fail_and_cleanup(failed_input);
        return;
    }

    if ((err = rxvm_fsearch(&compiled, fp, &msize, flags)) < 0) {
        fprintf(logfp, "%s:\n\tregex=%s (flags=%d): errcode=%d\n\n",
            failed_input, regex, flags, err);
        fail_and_cleanup(failed_input);
        return;
    } else if (err == 0) {
        fprintf(logfp, "%s:\n\tregex=%s\n\tinput=%s\n\tflags=%d\n\tNo match "
                       "found in file %s\n\n", failed_input, regex, gen, flags,
                       TMPFILE);
        fail_and_cleanup(failed_input);
        return;
    }

    fpos = ftell(fp);
    if (fpos!= pos) {
        fprintf(logfp, "%s:\n\tregex=%s\n\tinput=%s\n\tflags=%d\n\tmatch offset"
                       " is %ld, expecting %d\n\n", failed_input, regex, gen,
                       flags, fpos, pos);
        fail_and_cleanup(failed_input);
        return;
    }

    if (fread(match, 1, msize, fp) != msize) {
        fprintf(logfp, "%s:\n\terror reading match from test file\n\n",
                       failed_input);
        fail_and_cleanup(failed_input);
        return;
    }

    fclose(fp);
    fp = NULL;
    match[msize] = 0;

    if (strcmp(match, gen) != 0) {
        fprintf(logfp, "%s:\n\nregex=%s\ninput=%s\nflags=%d\nmatch should be:"
                       "\n%s\n\nbut got this:\n%s\n\n", failed_input, regex,
                       gen, flags, gen, match);
        fail_and_cleanup(failed_input);
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

    if (cleanup_testfile() != 0) {
        log_trs("FAIL", func);
        fprintf(logfp, "Failed to clean up test file "TMPFILE"\n");
    }

    cfg.limit = LIMIT;
    cfg.generosity = GENEROSITY;
    cfg.whitespace = WHITESPACE;

    verify_fsearch("xyz", "abccccc", 0, FIRST);
    verify_fsearch("xyz", "abccccc", 0, LAST);
    verify_fsearch("xyz", "abccccc", 0, MIDDLE);
    verify_fsearch("abcdef", "abcdeabcde", 0, FIRST);
    verify_fsearch("abcdef", "abcdeabcde", 0, LAST);
    verify_fsearch("abcdef", "abcdeabcde", 0, MIDDLE);
    verify_fsearch("abcdef+", "abcdeeeeee", 0, FIRST);
    verify_fsearch("abcdef+", "abcdeeeeee", 0, LAST);
    verify_fsearch("abcdef+", "abcdeeeeee", 0, MIDDLE);
    verify_fsearch("abcdef*", "abcdddddd", 0, FIRST);
    verify_fsearch("abcdef*", "abcdddddd", 0, LAST);
    verify_fsearch("abcdef*", "abcdddddd", 0, MIDDLE);
    verify_fsearch(".+", "\n\n\n\n\n", 0, FIRST );
    verify_fsearch(".+", "\n\n\n\n\n", 0, LAST );
    verify_fsearch(".+", "\n\n\n\n\n", 0, MIDDLE );
    verify_fsearch("<[^\n<>]+>", "<<\n><\n>>", 0, FIRST );
    verify_fsearch("<[^\n<>]+>", "<<\n><\n>>", 0, LAST );
    verify_fsearch("<[^\n<>]+>", "<<\n><\n>>", 0, MIDDLE );
    verify_fsearch("[^A-Za-z\n]+", "abcdefg", 0, FIRST);
    verify_fsearch("[^A-Za-z\n]+", "abcdefg", 0, LAST);
    verify_fsearch("[^A-Za-z\n]+", "abcdefg", 0, MIDDLE);
    verify_fsearch("[a-f\\-]+-", "ABCDEFFFFF", 0, FIRST);
    verify_fsearch("[a-f\\-]+-", "ABCDEFFFFF", 0, LAST);
    verify_fsearch("[a-f\\-]+-", "ABCDEFFFFF", 0, MIDDLE);
    verify_fsearch("[[*-+?]+", "@#$", 0, FIRST);
    verify_fsearch("[[*-+?]+", "@#$", 0, LAST);
    verify_fsearch("[[*-+?]+", "@#$", 0, MIDDLE);
    verify_fsearch("a+", "bc", 0, FIRST);
    verify_fsearch("a+", "bc", 0, LAST);
    verify_fsearch("a+", "bc", 0, MIDDLE);
    verify_fsearch("axq+", "bc", 0, FIRST);
    verify_fsearch("axq+", "bc", 0, LAST);
    verify_fsearch("axq+", "bc", 0, MIDDLE);
    verify_fsearch("axq+d", "bcX4", 0, FIRST);
    verify_fsearch("axq+d", "bcX4", 0, LAST);
    verify_fsearch("axq+d", "bcX4", 0, MIDDLE);
    verify_fsearch("eri+k", "erieri", 0, FIRST);
    verify_fsearch("eri+k", "erieri", 0, LAST);
    verify_fsearch("eri+k", "erieri", 0, MIDDLE);
    verify_fsearch("xyz+abcd", "lmno", 0, FIRST);
    verify_fsearch("xyz+abcd", "lmno", 0, LAST);
    verify_fsearch("xyz+abcd", "lmno", 0, MIDDLE);
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
