
#include "config.h"

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif


static void usage(char const *pname);
static void version(void);
Suite * util_suite();
Suite * send_suite();

#ifdef HAVE_GETOPT_LONG

/* *INDENT-OFF* */
static char usage_str[] = {
"\n"
"  -h, --help                Print the help and quit.\n"
"  -s, --suite=suites        Comma separated list of suites.\n"
"  -u, --unit=unit_tests     Comma separated list of unit tests.\n"
"  -v, --version             Print the version and quit.\n"
};

static struct option prog_opt[] = {
	{"help", 0, 0, 'h'},
	{"suite", 1, 0, 't'},
	{"unit", 1, 0, 'u'},
	{"version", 0, 0, 'v'},
	{NULL, 0, 0, 0}
};

#else

static char usage_str[] = {
"[-hv] [-s suite1,2,...,N] [-u unit_test1,2,...,N]"
};
/* *INDENT-ON* */

#endif

static void process_command_line_args(int argc, char * argv[])
{
	char const *pname = ((pname = strrchr(argv[0], '/')) != NULL) ? pname + 1 : argv[0];
	int c;
	
	/* parse args */
#define OPTIONS_STR "s:u:vh"
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
	while ((c = getopt_long(argc, argv, OPTIONS_STR, prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, OPTIONS_STR)) > 0)
#endif
	{
		switch (c) {
		case 't':
			strdup(optarg);
			break;
		case 'u':
			strdup(optarg);
			break;
		case 'v':
			version();
			break;
		case 'h':
			usage(pname);
#ifdef HAVE_GETOPT_LONG
		case ':':
			fprintf(stderr, "%s: option %s: parameter expected\n", pname, prog_opt[opt_idx].name);
			exit(1);
#endif
		case '?':
			exit(1);
		}
	}
}


int main(int argc, char * argv[])
{
	srand((unsigned int)time(NULL));

	process_command_line_args(argc, argv);

	SRunner * sr = srunner_create(util_suite());
	srunner_add_suite(sr, send_suite());
	srunner_run_all(sr, CK_VERBOSE);
	int number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void usage(char const *pname)
{
	fprintf(stderr, "usage: %s %s\n", pname, usage_str);
	exit(1);
}

static void version(void)
{
	fprintf(stderr, "Version: %s\n\n", VERSION);
	exit(0);
}

