#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include "lcmgen.h"

#include "getopt.h"
#include "tokenize.h"

void setup_c_options(getopt_t *gopt);
int emit_c(lcmgen_t *lcm);

void setup_java_options(getopt_t *gopt);
int emit_java(lcmgen_t *lcm);

void setup_python_options(getopt_t *gopt);
int emit_python(lcmgen_t *lcm);

int main(int argc, char *argv[])
{
    getopt_t *gopt = getopt_create();

    getopt_add_bool  (gopt, 'h',  "help",     0,    "Show this help");
    getopt_add_bool  (gopt, 't',  "tokenize", 0,    "Show tokenization");
    getopt_add_bool  (gopt, 'd',  "debug",    0,    "Show parsed file");
    getopt_add_bool  (gopt, 0,    "lazy",     0,    "Generate output file only if .lcm is newer");

    // we only support portable declarations now.
    // getopt_add_bool  (gopt, 0,    "warn-unsafe", 1, "Warn about unportable declarations");

    getopt_add_spacer(gopt, "**** C options ****");
    getopt_add_bool  (gopt, 'c', "c",         0,     "Emit C code");
    setup_c_options(gopt);

    getopt_add_spacer(gopt, "**** Java options ****");
    getopt_add_bool  (gopt, 'j', "java",      0,     "Emit Java code");
    setup_java_options(gopt);

    getopt_add_spacer(gopt, "**** Python options ****");
    getopt_add_bool  (gopt, 'p', "python",      0,     "Emit Python code");
    setup_python_options(gopt);

    if (!getopt_parse(gopt, argc, argv, 1) || getopt_get_bool(gopt,"help")) {
        printf("Usage: %s [options] <input files>\n\n", argv[0]);
        getopt_do_usage(gopt);
        return 0;
    }

    lcmgen_t *lcm = lcmgen_create();
    lcm->gopt = gopt;

    for (unsigned int i = 0; i < g_ptr_array_size(gopt->extraargs); i++) {
        char *path = g_ptr_array_index(gopt->extraargs, i);

        int res = lcmgen_handle_file(lcm, path);
        if (res)
            return -1;
    }

    int did_something = 0;
    // if they requested tokenizing (debug) output, we've done that now. Exit.
    if (getopt_get_bool(gopt, "tokenize")) {
        did_something = 1;
        return 0;
    }

    if (getopt_get_bool(gopt, "debug")) {
        did_something = 1;
        lcmgen_dump(lcm);
    }

    if (getopt_get_bool(gopt, "c")) {
        did_something = 1;
        if (emit_c(lcm)) {
            printf("An error occurred while emitting C code.\n");
        }
    }

    if (getopt_get_bool(gopt, "java")) {
        did_something = 1;
        if (emit_java(lcm)) {
            perror("An error occurred while emitting Java code.\n");
        }
    }

    if (getopt_get_bool(gopt, "python")) {
        did_something = 1;
        if (emit_python(lcm)) {
            printf("An error occurred while emitting Python code.\n");
        }
    }

    if (did_something == 0) {
        printf("No actions specified. Try --help.\n");
    }

    return 0;
}
