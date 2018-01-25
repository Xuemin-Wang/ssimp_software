#ifndef OPTIONS_HH
#define OPTIONS_HH

/* 
 * Global variables of the process.  All boolean option variables are
 * FALSE by default. 
 */ 

/* The -a option (consider all trivial dependencies to be non-trivial) */ 
static bool option_nontrivial= false;

/* The -d option (debug mode) */ 
static bool option_debug= false;

/* The -E option (explain error messages) */
static bool option_explain= false;

/* The -g option (consider all optional dependencies to be non-optional) */
static bool option_nonoptional= false;

/* The -i option (interactive mode) */
static bool option_interactive= false;

/* The -J option (literal interpretation of argumens) */
static bool option_literal= false; 

/* The -k option (keep going) */ 
static bool option_keep_going= false;

/* The -K option (don't delete partially built files) */
static bool option_no_delete= false;

/* The -P option (print rules) */
static bool option_print= false;

/* The -q option (question mode) */
static bool option_question= false; 

/* The -s option (silent) */
static bool option_silent= false;

/* The -x option (use sh -x) */ 
static bool option_individual= false;

/* The -z option (output statistics) */
static bool option_statistics= false;

enum class Order {
	DFS   = 0,
	RANDOM= 1,
	
	/* -M mode is coded as Order::RANDOM */ 
};
static Order order= Order::DFS; 

/* Whether the -j option is used with a value >1 */ 
bool option_parallel= false;

/* Whether to use vectors for randomization */ 
static bool order_vec; 

/* The envp variable.  Set in main(). */
const char **envp_global;

/* Does the same as program_invocation_name (which is a GNU extension,
 * so we don't use it); the value of argv[0], set in main() */ 
static const char *dollar_zero;

#endif /* ! OPTIONS_HH */
