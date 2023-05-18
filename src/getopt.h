#ifndef _GETOPT_H_
#define _GETOPT_H_

struct option {
	/* name of long option */
	const char *name;
	/*
	 * one of no_argument, required_argument, and optional_argument:
	 * whether option takes an argument
	 */
	int has_arg;
	/* if not NULL, set *flag to val when option found */
	int *flag;
	/* if flag not NULL, value to set *flag to; else return value */
	int val;
};

int getopt(int argc, char *const *argv, const char *optstring);

extern char *optarg;
extern int optind, opterr, optopt;
extern int optreset;
 
#endif /* !_GETOPT_H_ */