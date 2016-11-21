#ifndef lint
#if __GNUC__ - 0 >= 4 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ >= 1)
__attribute__((__used__))
#endif
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#ifdef _LIBC
#include "namespace.h"
#endif
#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)

#define YYPREFIX "yy"

#define YYPURE 0

#line 2 "testlang_parse.y"
/*	$NetBSD: testlang_parse.y,v 1.13 2012/09/19 11:51:56 blymn Exp $	*/

/*-
 * Copyright 2009 Brett Lymn <blymn@NetBSD.org>
 *
 * All rights reserved.
 *
 * This code has been donated to The NetBSD Foundation by the Author.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software withough specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */
#include <assert.h>
#include <curses.h>
#include <errno.h>
#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/syslimits.h>
#include <time.h>
#include <vis.h>
#include <stdint.h>
#include "returns.h"

#define YYDEBUG 1

extern int verbose;
extern int cmdpipe[2];
extern int slvpipe[2];
extern int master;
extern struct pollfd readfd;
extern char *check_path;
extern char *cur_file;		/* from director.c */

int yylex(void);

size_t line;

static int input_delay;

/* time delay between inputs chars - default to 0.1ms minimum to prevent
 * problems with input tests
 */
#define DELAY_MIN 0.1

/* time delay after a function call - allows the slave time to
 * run the function and output data before we do other actions.
 * Set this to 50ms.
 */
#define POST_CALL_DELAY 50

static struct timespec delay_spec = {0, 1000 * DELAY_MIN};
static struct timespec delay_post_call = {0, 1000 * POST_CALL_DELAY};

static char *input_str;	/* string to feed in as input */
static bool no_input;	/* don't need more input */

#define READ_PIPE  0
#define WRITE_PIPE 1

const char *returns_enum_names[] = {
	"unused", "numeric", "string", "byte", "ERR", "OK", "NULL", "not NULL",
	"variable", "reference", "returns count", "slave error"
};

typedef enum {
	arg_static,
	arg_byte,
	arg_var,
	arg_null
} args_state_t;

static const char *args_enum_names[] = {
	"static", "byte", "var", "NULL"
};

typedef struct {
	args_state_t	arg_type;
	size_t		arg_len;
	char		*arg_string;
	int		var_index;
} args_t;

typedef struct {
	char		*function;
	int		nrets;		/* number of returns */
	returns_t	*returns;	/* array of expected returns */
	int		nargs;		/* number of arguments */
	args_t		*args;		/* arguments for the call */
} cmd_line_t;

static cmd_line_t	command;

typedef struct {
	char *name;
	size_t len;
	returns_enum_t type;
	void *value;
} var_t;

static size_t nvars; 		/* Number of declared variables */
static var_t *vars; 		/* Variables defined during the test. */

static int	check_function_table(char *, const char *[], int);
static int	find_var_index(const char *);
static void 	assign_arg(args_state_t, void *);
static int	assign_var(char *);
void		init_parse_variables(int);
static void	validate(int, void *);
static void	validate_return(const char *, const char *, int);
static void	validate_variable(int, returns_enum_t, const void *, int, int);
static void	validate_byte(returns_t *, returns_t *, int);
static void	write_cmd_pipe(char *);
static void	write_cmd_pipe_args(args_state_t, void *);
static void	read_cmd_pipe(returns_t *);
static void	write_func_and_args(void);
static void	compare_streams(char *, bool);
static void	do_function_call(size_t);
static void	save_slave_output(bool);
static void	validate_type(returns_enum_t, returns_t *, int);
static void	set_var(returns_enum_t, char *, void *);
static void	validate_reference(int, void *);
static char	*numeric_or(char *, char *);
static char	*get_numeric_var(const char *);
static void	perform_delay(struct timespec *);

static const char *input_functions[] = {
	"getch", "getnstr", "getstr", "mvgetnstr", "mvgetstr", "mvgetnstr",
	"mvgetstr", "mvscanw", "mvwscanw", "scanw", "wgetch", "wgetnstr",
	"wgetstr"
};

static const unsigned ninput_functions =
	sizeof(input_functions) / sizeof(char *);

saved_data_t saved_output;

#line 163 "testlang_parse.y"
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
	char *string;
	returns_t *retval;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 199 "testlang_parse.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();


#define PATH 257
#define STRING 258
#define BYTE 259
#define VARNAME 260
#define FILENAME 261
#define VARIABLE 262
#define REFERENCE 263
#define NULL_RET 264
#define NON_NULL 265
#define ERR_RET 266
#define OK_RET 267
#define numeric 268
#define DELAY 269
#define INPUT 270
#define COMPARE 271
#define COMPAREND 272
#define ASSIGN 273
#define EOL 274
#define CALL 275
#define CHECK 276
#define NOINPUT 277
#define OR 278
#define LHB 279
#define RHB 280
#define CALL2 281
#define CALL3 282
#define CALL4 283
#define DRAIN 284
#define YYERRCODE 256
static const short yylhs[] = {                           -1,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   13,    1,   15,    1,   16,    1,   17,
    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
   10,   11,   11,   18,   18,   18,   22,   22,   22,   22,
   22,   22,   22,   22,   21,   23,   19,   14,   14,   14,
   20,   24,   20,   25,   20,   26,   20,   27,   20,   28,
   20,   29,   20,   30,   20,   31,   20,   32,   20,   12,
};
static const short yylen[] = {                            2,
    0,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    0,    5,    0,    7,    0,    5,    0,
    5,    5,    6,    7,    8,    4,    3,    3,    2,    3,
    3,    3,    3,    1,    1,    1,    1,    3,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    3,
    0,    0,    5,    0,    3,    0,    3,    0,    3,    0,
    3,    0,    3,    0,    3,    0,    3,    0,    3,    1,
};
static const short yydefred[] = {                         0,
    0,    0,    0,    0,    0,   70,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   39,   40,   45,   46,   43,   44,   41,   42,
   37,    0,    0,   35,   34,   36,    0,   29,    0,    0,
    0,    2,    3,    4,    5,    6,    7,    8,    9,   10,
   11,   12,   13,   27,   28,   30,   31,   32,   33,   18,
   20,   14,    0,   49,   48,    0,   47,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   38,   60,   56,
   58,   64,   62,   66,   68,   54,    0,    0,   26,    0,
    0,    0,   19,   21,   15,   16,   50,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   22,    0,    0,    0,
    0,   61,   57,   59,   65,   63,   67,   69,   55,   52,
   23,    0,    0,   17,    0,   24,    0,   53,   25,
};
static const short yydgoto[] = {                         13,
   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
   24,   25,   85,   76,  121,   83,   84,   43,   78,   98,
   44,   45,   46,  135,  115,  109,  110,  108,  112,  111,
  113,  114,
};
static const short yysindex[] = {                      -137,
 -264, -246, -256, -247, -245,    0, -174, -228, -234, -174,
 -174, -174,    0, -137, -137, -137, -137, -137, -137, -137,
 -137, -137, -137, -137, -137, -234, -234, -234, -234, -234,
 -234, -155,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -259, -227,    0,    0,    0, -148,    0, -174, -174,
 -174,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0, -259,    0,    0, -267,    0, -197, -234, -227,
 -174, -174, -234, -234, -234, -242, -259,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -259, -234,    0, -197,
 -227, -174,    0,    0,    0,    0,    0, -197, -197, -197,
 -197, -197, -197, -197, -197, -241,    0, -234, -197, -227,
 -234,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -234, -197,    0, -197,    0, -234,    0,    0,
};
static const short yyrindex[] = {                        41,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -231,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -231,
    0,    0,    0,    0,    0,    0,    0, -231, -231, -231,
 -231, -231, -231, -231, -231,    0,    0,    0, -231,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0, -231,    0, -231,    0,    0,    0,    0,
};
static const short yygindex[] = {                       133,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   -9,    0,  -39,    0,    0,    0,   -4,  -78,  -84,
   36,    3,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
#define YYTABLESIZE 158
static const short yytable[] = {                         48,
   28,  100,   74,   26,   29,   49,   50,   51,   75,   30,
   87,   27,   88,   31,   32,  118,   64,   65,   66,   67,
   68,   69,  119,  122,  123,  124,  125,  126,  127,  128,
  129,   35,   77,   86,  132,   87,   87,  106,  130,    6,
    1,  133,   51,   47,   80,   81,   82,  107,  137,   79,
  138,    0,    0,    0,    0,    0,    0,  116,    0,   89,
   90,   91,   92,   93,   94,    0,   95,    0,    0,   99,
   96,    0,    0,  103,  104,  105,  101,  102,    0,    0,
    0,   97,    0,   33,   34,   35,    0,   36,  117,   37,
   38,   39,   40,   41,    0,    0,    0,  120,    0,    0,
    0,    0,   70,   71,   42,    0,    0,    0,  131,   33,
   34,  134,   72,    0,    0,   37,   38,   39,   40,   41,
    0,    0,  136,   73,    0,    0,    0,  139,    0,    0,
   42,    1,    2,    3,    4,    5,    6,    7,    8,    9,
    0,    0,    0,   10,   11,   12,   52,   53,   54,   55,
   56,   57,   58,   59,   60,   61,   62,   63,
};
static const short yycheck[] = {                          9,
  257,   80,  262,  268,  261,   10,   11,   12,  268,  257,
  278,  258,  280,  261,  260,  100,   26,   27,   28,   29,
   30,   31,  101,  108,  109,  110,  111,  112,  113,  114,
  115,  260,  260,   73,  119,  278,  278,  280,  280,  274,
    0,  120,  274,    8,   49,   50,   51,   87,  133,   47,
  135,   -1,   -1,   -1,   -1,   -1,   -1,   97,   -1,  257,
  258,  259,  260,  261,  262,   -1,  264,   -1,   -1,   79,
  268,   -1,   -1,   83,   84,   85,   81,   82,   -1,   -1,
   -1,  279,   -1,  258,  259,  260,   -1,  262,   98,  264,
  265,  266,  267,  268,   -1,   -1,   -1,  102,   -1,   -1,
   -1,   -1,  258,  259,  279,   -1,   -1,   -1,  118,  258,
  259,  121,  268,   -1,   -1,  264,  265,  266,  267,  268,
   -1,   -1,  132,  279,   -1,   -1,   -1,  137,   -1,   -1,
  279,  269,  270,  271,  272,  273,  274,  275,  276,  277,
   -1,   -1,   -1,  281,  282,  283,   14,   15,   16,   17,
   18,   19,   20,   21,   22,   23,   24,   25,
};
#define YYFINAL 13
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 284
#if YYDEBUG
static const char *yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"PATH","STRING","BYTE","VARNAME",
"FILENAME","VARIABLE","REFERENCE","NULL_RET","NON_NULL","ERR_RET","OK_RET",
"numeric","DELAY","INPUT","COMPARE","COMPAREND","ASSIGN","EOL","CALL","CHECK",
"NOINPUT","OR","LHB","RHB","CALL2","CALL3","CALL4","DRAIN",
};
static const char *yyrule[] = {
"$accept : statement",
"statement :",
"statement : assign statement",
"statement : call statement",
"statement : call2 statement",
"statement : call3 statement",
"statement : call4 statement",
"statement : check statement",
"statement : delay statement",
"statement : input statement",
"statement : noinput statement",
"statement : compare statement",
"statement : comparend statement",
"statement : eol statement",
"$$1 :",
"assign : ASSIGN VARNAME numeric $$1 eol",
"$$2 :",
"assign : ASSIGN VARNAME LHB expr RHB $$2 eol",
"$$3 :",
"assign : ASSIGN VARNAME STRING $$3 eol",
"$$4 :",
"assign : ASSIGN VARNAME BYTE $$4 eol",
"call : CALL result fn_name args eol",
"call2 : CALL2 result result fn_name args eol",
"call3 : CALL3 result result result fn_name args eol",
"call4 : CALL4 result result result result fn_name args eol",
"check : CHECK var returns eol",
"delay : DELAY numeric eol",
"input : INPUT STRING eol",
"noinput : NOINPUT eol",
"compare : COMPARE PATH eol",
"compare : COMPARE FILENAME eol",
"comparend : COMPAREND PATH eol",
"comparend : COMPAREND FILENAME eol",
"result : returns",
"result : var",
"result : reference",
"returns : numeric",
"returns : LHB expr RHB",
"returns : STRING",
"returns : BYTE",
"returns : ERR_RET",
"returns : OK_RET",
"returns : NULL_RET",
"returns : NON_NULL",
"var : VARNAME",
"reference : VARIABLE",
"fn_name : VARNAME",
"expr : numeric",
"expr : VARIABLE",
"expr : expr OR expr",
"args :",
"$$5 :",
"args : LHB expr RHB $$5 args",
"$$6 :",
"args : numeric $$6 args",
"$$7 :",
"args : STRING $$7 args",
"$$8 :",
"args : BYTE $$8 args",
"$$9 :",
"args : PATH $$9 args",
"$$10 :",
"args : FILENAME $$10 args",
"$$11 :",
"args : VARNAME $$11 args",
"$$12 :",
"args : VARIABLE $$12 args",
"$$13 :",
"args : NULL_RET $$13 args",
"eol : EOL",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

typedef struct {
    unsigned stacksize;
    short    *s_base;
    short    *s_mark;
    short    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 435 "testlang_parse.y"

static void
excess(const char *fname, size_t lineno, const char *func, const char *comment,
    const void *data, size_t datalen)
{
	size_t dstlen = datalen * 4 + 1;
	char *dst = malloc(dstlen);

	if (dst == NULL)
		err(1, "malloc");

	if (strnvisx(dst, dstlen, data, datalen, VIS_WHITE | VIS_OCTAL) == -1)
		err(1, "strnvisx");

	warnx("%s, %zu: [%s] Excess %zu bytes%s [%s]",
	    fname, lineno, func, datalen, comment, dst);
	free(dst);
}

/*
 * Get the value of a variable, error if the variable has not been set or
 * is not a numeric type.
 */
static char *
get_numeric_var(const char *var)
{
	int i;

	if ((i = find_var_index(var)) < 0)
		err(1, "Variable %s is undefined", var);

	if (vars[i].type != ret_number)
		err(1, "Variable %s is not a numeric type", var);

	return vars[i].value;
}

/*
 * Perform a bitwise OR on two numbers and return the result.
 */
static char *
numeric_or(char *n1, char *n2)
{
	unsigned long i1, i2, result;
	char *ret;

	i1 = strtoul(n1, NULL, 10);
	i2 = strtoul(n2, NULL, 10);

	result = i1 | i2;
	asprintf(&ret, "%lu", result);

	if (verbose) {
		fprintf(stderr, "numeric or of 0x%lx (%s) and 0x%lx (%s)"
		    " results in 0x%lx (%s)\n",
		    i1, n1, i2, n2, result, ret);
	}

	return ret;
}

/*
 * Sleep for the specified time, handle the sleep getting interrupted
 * by a signal.
 */
static void
perform_delay(struct timespec *ts)
{
	struct timespec delay_copy, delay_remainder;

	delay_copy = *ts;
	while (nanosleep(&delay_copy, &delay_remainder) < 0) {
		if (errno != EINTR)
			err(2, "nanosleep returned error");
		delay_copy = delay_remainder;
	}
}

/*
 * Assign the value given to the named variable.
 */
static void
set_var(returns_enum_t type, char *name, void *value)
{
	int i;
	char *number;
	returns_t *ret;

	i = find_var_index(name);
	if (i < 0)
		i = assign_var(name);

	vars[i].type = type;
	if ((type == ret_number) || (type == ret_string)) {
		number = value;
		vars[i].len = strlen(number) + 1;
		vars[i].value = malloc(vars[i].len + 1);
		if (vars[i].value == NULL)
			err(1, "Could not malloc memory for assign string");
		strcpy(vars[i].value, number);
	} else {
		/* can only be a byte value */
		ret = value;
		vars[i].len = ret->return_len;
		vars[i].value = malloc(vars[i].len);
		if (vars[i].value == NULL)
			err(1, "Could not malloc memory to assign byte string");
		memcpy(vars[i].value, ret->return_value, vars[i].len);
	}
}

/*
 * Add a new variable to the vars array, the value will be assigned later,
 * when a test function call returns.
 */
static int
assign_var(char *varname)
{
	var_t *temp;
	char *name;

	if ((name = malloc(strlen(varname) + 1)) == NULL)
		err(1, "Alloc of varname failed");

	if ((temp = realloc(vars, sizeof(*temp) * (nvars + 1))) == NULL) {
		free(name);
		err(1, "Realloc of vars array failed");
	}

	strcpy(name, varname);
	vars = temp;
	vars[nvars].name = name;
	vars[nvars].len = 0;
	vars[nvars].value = NULL;
	nvars++;

	return (nvars - 1);
}

/*
 * Allocate and assign a new argument of the given type.
 */
static void
assign_arg(args_state_t arg_type, void *arg)
{
	args_t *temp, cur;
	char *str = arg;
	returns_t *ret;

	if (verbose) {
		fprintf(stderr, "function is >%s<, adding arg >%s< type %s\n",
		       command.function, str, args_enum_names[arg_type]);
	}

	cur.arg_type = arg_type;
	switch (arg_type) {
	case arg_var:
		cur.var_index = find_var_index(arg);
		if (cur.var_index < 0)
			err(1, "Invalid variable %s at line %zu of file %s",
			    str, line, cur_file);
		cur.arg_type = ret_string;
		break;

	case arg_byte:
		ret = arg;
		cur.arg_len = ret->return_len;
		cur.arg_string = malloc(cur.arg_len);
		if (cur.arg_string == NULL)
			err(1, "Could not malloc memory for arg bytes");
		memcpy(cur.arg_string, ret->return_value, cur.arg_len);
		break;

	case arg_null:
		cur.arg_len = 0;
		cur.arg_string = NULL;
		break;

	default:
		cur.arg_len = strlen(str);
		cur.arg_string = malloc(cur.arg_len + 1);
		if (cur.arg_string == NULL)
			err(1, "Could not malloc memory for arg string");
		strcpy(cur.arg_string, arg);
	}

	temp = realloc(command.args, sizeof(*temp) * (command.nargs + 1));
	if (temp == NULL)
		err(1, "Failed to reallocate args");
	command.args = temp;
	memcpy(&command.args[command.nargs], &cur, sizeof(args_t));
	command.nargs++;
}

/*
 * Allocate and assign a new return.
 */
static void
assign_rets(returns_enum_t ret_type, void *ret)
{
	returns_t *temp, cur;
	char *ret_str;
	returns_t *ret_ret;

	cur.return_type = ret_type;
	if (ret_type != ret_var) {
		if ((ret_type == ret_number) || (ret_type == ret_string)) {
			ret_str = ret;
			cur.return_len = strlen(ret_str) + 1;
			cur.return_value = malloc(cur.return_len + 1);
			if (cur.return_value == NULL)
				err(1,
				    "Could not malloc memory for arg string");
			strcpy(cur.return_value, ret_str);
		} else if (ret_type == ret_byte) {
			ret_ret = ret;
			cur.return_len = ret_ret->return_len;
			cur.return_value = malloc(cur.return_len);
			if (cur.return_value == NULL)
				err(1,
				    "Could not malloc memory for byte string");
			memcpy(cur.return_value, ret_ret->return_value,
			       cur.return_len);
		} else if (ret_type == ret_ref) {
			if ((cur.return_index = find_var_index(ret)) < 0)
				err(1, "Undefined variable reference");
		}
	} else {
		cur.return_index = find_var_index(ret);
		if (cur.return_index < 0)
			cur.return_index = assign_var(ret);
	}

	temp = realloc(command.returns, sizeof(*temp) * (command.nrets + 1));
	if (temp == NULL)
		err(1, "Failed to reallocate returns");
	command.returns = temp;
	memcpy(&command.returns[command.nrets], &cur, sizeof(returns_t));
	command.nrets++;
}

/*
 * Find the given variable name in the var array and return the i
 * return -1 if var is not found.
 */
static int
find_var_index(const char *var_name)
{
	int result;
	size_t i;

	result = -1;

	for (i = 0; i < nvars; i++) {
		if (strcmp(var_name, vars[i].name) == 0) {
			result = i;
			break;
		}
	}

	return result;
}

/*
 * Check the given function name in the given table of names, return 1 if
 * there is a match.
 */
static int check_function_table(char *function, const char *table[],
				int nfunctions)
{
	int i;

	for (i = 0; i < nfunctions; i++) {
		if ((strlen(function) == strlen(table[i])) &&
		    (strcmp(function, table[i]) == 0))
			return 1;
	}

	return 0;
}

/*
 * Compare the output from the slave against the given file and report
 * any differences.
 */
static void
compare_streams(char *filename, bool discard)
{
	char check_file[PATH_MAX], drain[100], ref, data;
	struct pollfd fds[2];
	int nfd, check_fd;
	ssize_t result;
	size_t offs;

	/*
	 * Don't prepend check path iff check file has an absolute
	 * path.
	 */
	if (filename[0] != '/') {
		if (strlcpy(check_file, check_path, sizeof(check_file))
		    >= sizeof(check_file))
			err(2, "CHECK_PATH too long");

		if (strlcat(check_file, "/", sizeof(check_file))
		    >= sizeof(check_file))
			err(2, "Could not append / to check file path");
	} else {
		check_file[0] = '\0';
	}

	if (strlcat(check_file, filename, sizeof(check_file))
	    >= sizeof(check_file))
		err(2, "Path to check file path overflowed");

	if ((check_fd = open(check_file, O_RDONLY, 0)) < 0)
		err(2, "failed to open file %s line %zu of file %s",
		    check_file, line, cur_file);

	fds[0].fd = check_fd;
	fds[0].events = POLLIN;
	fds[1].fd = master;
	fds[1].events = POLLIN;

	nfd = 2;
	/*
	 * if we have saved output then only check for data in the
	 * reference file since the slave data may already be drained.
	 */
	if (saved_output.count > 0)
		nfd = 1;

	offs = 0;
	while (poll(fds, nfd, 500) == nfd) {
		if (fds[0].revents & POLLIN) {
			if ((result = read(check_fd, &ref, 1)) < 1) {
				if (result != 0) {
					err(2,
					    "Bad read on file %s", check_file);
				} else {
					break;
				}
			}
		}

		if (saved_output.count > 0) {
			data = saved_output.data[saved_output.readp];
			saved_output.count--;
			saved_output.readp++;
			/* run out of saved data, switch to file */
			if (saved_output.count == 0)
				nfd = 2;
		} else {
			if (fds[0].revents & POLLIN) {
				if (read(master, &data, 1) < 1)
					err(2, "Bad read on slave pty");
			} else
				continue;
		}

		if (verbose) {
			fprintf(stderr, "Comparing reference byte 0x%x (%c)"
				" against slave byte 0x%x (%c)\n",
				ref, (ref >= ' ') ? ref : '-',
				data, (data >= ' ' )? data : '-');
		}

		if (ref != data) {
			errx(2, "%s, %zu: refresh data from slave does "
			    "not match expected from file %s offs %zu "
			    "[reference 0x%x (%c) != slave 0x%x (%c)]",
			    cur_file, line, check_file, offs,
			    ref, (ref >= ' ') ? ref : '-',
			    data, (data >= ' ') ? data : '-');
		}

		offs++;
	}


	if (saved_output.count > 0)
		excess(cur_file, line, __func__, " from slave",
		    &saved_output.data[saved_output.readp], saved_output.count);

	/* discard any excess saved output if required */
	if (discard) {
		saved_output.count = 0;
		saved_output.readp = 0;
	}

	if ((result = poll(&fds[0], 2, 0)) != 0) {
		if (result == -1)
			err(2, "poll of file descriptors failed");

		if ((fds[1].revents & POLLIN) == POLLIN) {
			save_slave_output(true);
		} else if ((fds[0].revents & POLLIN) == POLLIN) {
			/*
			 * handle excess in file if it exists.  Poll
			 * says there is data until EOF is read.
			 * Check next read is EOF, if it is not then
			 * the file really has more data than the
			 * slave produced so flag this as a warning.
			 */
			result = read(check_fd, drain, sizeof(drain));
			if (result == -1)
				err(1, "read of data file failed");

			if (result > 0) {
				excess(check_file, 0, __func__, "", drain,
				    result);
			}
		}
	}

	close(check_fd);
}

/*
 * Pass a function call and arguments to the slave and wait for the
 * results.  The variable nresults determines how many returns we expect
 * back from the slave.  These results will be validated against the
 * expected returns or assigned to variables.
 */
static void
do_function_call(size_t nresults)
{
#define MAX_RESULTS 4
	char *p;
	int do_input;
	size_t i;
	struct pollfd fds[3];
	returns_t response[MAX_RESULTS], returns_count;
	assert(nresults <= MAX_RESULTS);

	do_input = check_function_table(command.function, input_functions,
	    ninput_functions);

	write_func_and_args();

	/*
	 * We should get the number of returns back here, grab it before
	 * doing input otherwise it will confuse the input poll
	 */
	read_cmd_pipe(&returns_count);
	if (returns_count.return_type != ret_count)
		err(2, "expected return type of ret_count but received %s",
		    returns_enum_names[returns_count.return_type]);

	perform_delay(&delay_post_call); /* let slave catch up */

	if (verbose) {
		fprintf(stderr, "Expect %zu results from slave, slave "
		    "reported %zu\n", nresults, returns_count.return_len);
	}

	if ((no_input == false) && (do_input == 1)) {
		if (verbose) {
			fprintf(stderr, "doing input with inputstr >%s<\n",
			    input_str);
		}

		if (input_str == NULL)
			errx(2, "%s, %zu: Call to input function "
			    "but no input defined", cur_file, line);

		fds[0].fd = slvpipe[READ_PIPE];
		fds[0].events = POLLIN;
		fds[1].fd = master;
		fds[1].events = POLLOUT;
 		p = input_str;
		save_slave_output(false);
		while(*p != '\0') {
			perform_delay(&delay_spec);

			if (poll(fds, 2, 0) < 0)
				err(2, "poll failed");
			if (fds[0].revents & POLLIN) {
				warnx("%s, %zu: Slave function "
				    "returned before end of input string",
				    cur_file, line);
				break;
			}
			if ((fds[1].revents & POLLOUT) == 0)
				continue;
			if (verbose) {
				fprintf(stderr, "Writing char >%c< to slave\n",
				    *p);
			}
			if (write(master, p, 1) != 1) {
				warn("%s, %zu: Slave function write error",
				    cur_file, line);
				break;
			}
			p++;

		}
		save_slave_output(false);

		if (verbose) {
			fprintf(stderr, "Input done.\n");
		}

		/* done with the input string, free the resources */
		free(input_str);
		input_str = NULL;
	}

	if (verbose) {
		fds[0].fd = slvpipe[READ_PIPE];
		fds[0].events = POLLIN;

		fds[1].fd = slvpipe[WRITE_PIPE];
		fds[1].events = POLLOUT;

		fds[2].fd = master;
		fds[2].events = POLLIN | POLLOUT;

		i = poll(&fds[0], 3, 1000);
		fprintf(stderr, "Poll returned %zu\n", i);
		for (i = 0; i < 3; i++) {
			fprintf(stderr, "revents for fd[%zu] = 0x%x\n",
				i, fds[i].revents);
		}
	}

	/* drain any trailing output */
	save_slave_output(false);

	for (i = 0; i < returns_count.return_len; i++) {
		read_cmd_pipe(&response[i]);
	}

	/*
	 * Check for a slave error in the first return slot, if the
	 * slave errored then we may not have the number of returns we
	 * expect but in this case we should report the slave error
	 * instead of a return count mismatch.
	 */
	if ((returns_count.return_len > 0) &&
	    (response[0].return_type == ret_slave_error))
		err(2, "Slave returned error: %s",
		    (const char *)response[0].return_value);

	if (returns_count.return_len != nresults)
		err(2, "Incorrect number of returns from slave, expected %zu "
		    "but received %zu", nresults, returns_count.return_len);

	if (verbose) {
		for (i = 0; i < nresults; i++) {
			if ((response[i].return_type != ret_byte) &&
			    (response[i].return_type != ret_err) &&
			    (response[i].return_type != ret_ok))
				fprintf(stderr,
					"received response >%s< "
					"expected",
					(const char *)response[i].return_value);
			else
				fprintf(stderr, "received");

			fprintf(stderr, " return_type %s\n",
			    returns_enum_names[command.returns[i].return_type]);
		}
	}

	for (i = 0; i < nresults; i++) {
		if (command.returns[i].return_type != ret_var) {
			validate(i, &response[i]);
		} else {
			vars[command.returns[i].return_index].len =
				response[i].return_len;
			vars[command.returns[i].return_index].value =
				response[i].return_value;
			vars[command.returns[i].return_index].type =
				response[i].return_type;
		}
	}

	if (verbose && (saved_output.count > 0))
		excess(cur_file, line, __func__, " from slave",
		    &saved_output.data[saved_output.readp], saved_output.count);

	init_parse_variables(0);
}

/*
 * Write the function and command arguments to the command pipe.
 */
static void
write_func_and_args(void)
{
	int i;

	if (verbose) {
		fprintf(stderr, "calling function >%s<\n", command.function);
	}

	write_cmd_pipe(command.function);
	for (i = 0; i < command.nargs; i++) {
		if (command.args[i].arg_type == arg_var)
			write_cmd_pipe_args(command.args[i].arg_type,
					    &vars[command.args[i].var_index]);
		else
			write_cmd_pipe_args(command.args[i].arg_type,
					    &command.args[i]);
	}

	write_cmd_pipe(NULL); /* signal end of arguments */
}

/*
 * Initialise the command structure - if initial is non-zero then just set
 * everything to sane values otherwise free any memory that was allocated
 * when building the structure.
 */
void
init_parse_variables(int initial)
{
	int i, result;
	struct pollfd slave_pty;

	if (initial == 0) {
		free(command.function);
		for (i = 0; i < command.nrets; i++) {
			if (command.returns[i].return_type == ret_number)
				free(command.returns[i].return_value);
		}
		free(command.returns);

		for (i = 0; i < command.nargs; i++) {
			if (command.args[i].arg_type != arg_var)
				free(command.args[i].arg_string);
		}
		free(command.args);
	} else {
		line = 0;
		input_delay = 0;
		vars = NULL;
		nvars = 0;
		input_str = NULL;
		saved_output.allocated = 0;
		saved_output.count = 0;
		saved_output.readp = 0;
		saved_output.data = NULL;
	}

	no_input = false;
	command.function = NULL;
	command.nargs = 0;
	command.args = NULL;
	command.nrets = 0;
	command.returns = NULL;

	/*
	 * Check the slave pty for stray output from the slave, at this
	 * point we should not see any data as it should have been
	 * consumed by the test functions.  If we see data then we have
	 * either a bug or are not handling an output generating function
	 * correctly.
	 */
	slave_pty.fd = master;
	slave_pty.events = POLLIN;
	result = poll(&slave_pty, 1, 0);

	if (result < 0)
		err(2, "Poll of slave pty failed");
	else if (result > 0)
		warnx("%s, %zu: Unexpected data from slave", cur_file, line);
}

/*
 * Validate the response against the expected return.  The variable
 * i is the i into the rets array in command.
 */
static void
validate(int i, void *data)
{
	char *response;
	returns_t *byte_response;

	byte_response = data;
	if ((command.returns[i].return_type != ret_byte) &&
	    (command.returns[i].return_type != ret_err) &&
	    (command.returns[i].return_type != ret_ok)) {
		if ((byte_response->return_type == ret_byte) ||
		    (byte_response->return_type == ret_err) ||
		    (byte_response->return_type == ret_ok))
			err(1, "%s: expecting type %s, received type %s"
			    " at line %zu of file %s", __func__,
			    returns_enum_names[command.returns[i].return_type],
			    returns_enum_names[byte_response->return_type],
			    line, cur_file);

		response = byte_response->return_value;
	}

	switch (command.returns[i].return_type) {
	case ret_err:
		validate_type(ret_err, byte_response, 0);
		break;

	case ret_ok:
		validate_type(ret_ok, byte_response, 0);
		break;

	case ret_null:
		validate_return("NULL", response, 0);
		break;

	case ret_nonnull:
		validate_return("NULL", response, 1);
		break;

	case ret_string:
	case ret_number:
		validate_return(command.returns[i].return_value,
				response, 0);
		break;

	case ret_ref:
		validate_reference(i, response);
		break;

	case ret_byte:
		validate_byte(&command.returns[i], byte_response, 0);
		break;

	default:
		err(1, "Malformed statement at line %zu of file %s",
		    line, cur_file);
		break;
	}
}

/*
 * Validate the return against the contents of a variable.
 */
static void
validate_reference(int i, void *data)
{
	char *response;
	returns_t *byte_response;
	var_t *varp;

	varp = &vars[command.returns[i].return_index];

	byte_response = data;
	if (command.returns[i].return_type != ret_byte)
		response = data;

	if (verbose) {
		fprintf(stderr,
		    "%s: return type of %s, value %s \n", __func__,
		    returns_enum_names[varp->type],
		    (const char *)varp->value);
	}

	switch (varp->type) {
	case ret_string:
	case ret_number:
		validate_return(varp->value, response, 0);
		break;

	case ret_byte:
		validate_byte(varp->value, byte_response, 0);
		break;

	default:
		err(1,
		    "Invalid return type for reference at line %zu of file %s",
		    line, cur_file);
		break;
	}
}

/*
 * Validate the return type against the expected type, throw an error
 * if they don't match.
 */
static void
validate_type(returns_enum_t expected, returns_t *value, int check)
{
	if (((check == 0) && (expected != value->return_type)) ||
	    ((check == 1) && (expected == value->return_type)))
		err(1, "Validate expected type %s %s %s line %zu of file %s",
		    returns_enum_names[expected],
		    (check == 0)? "matching" : "not matching",
		    returns_enum_names[value->return_type], line, cur_file);

	if (verbose) {
		fprintf(stderr, "Validate expected type %s %s %s line %zu"
		    " of file %s\n",
		    returns_enum_names[expected],
		    (check == 0)? "matching" : "not matching",
		    returns_enum_names[value->return_type], line, cur_file);
	}
}

/*
 * Validate the return value against the expected value, throw an error
 * if they don't match.
 */
static void
validate_return(const char *expected, const char *value, int check)
{
	if (((check == 0) && strcmp(expected, value) != 0) ||
	    ((check == 1) && strcmp(expected, value) == 0))
		errx(1, "Validate expected %s %s %s line %zu of file %s",
		    expected,
		    (check == 0)? "matching" : "not matching", value,
		    line, cur_file);
	if (verbose) {
		fprintf(stderr, "Validated expected value %s %s %s "
		    "at line %zu of file %s\n", expected,
		    (check == 0)? "matches" : "does not match",
		    value, line, cur_file);
	}
}

/*
 * Validate the return value against the expected value, throw an error
 * if they don't match expectations.
 */
static void
validate_byte(returns_t *expected, returns_t *value, int check)
{
	char *ch;
	size_t i;

	if (verbose) {
		ch = value->return_value;
		fprintf(stderr, "checking returned byte stream: ");
		for (i = 0; i < value->return_len; i++)
			fprintf(stderr, "%s0x%x", (i != 0)? ", " : "", ch[i]);
		fprintf(stderr, "\n");

		fprintf(stderr, "%s byte stream: ",
			(check == 0)? "matches" : "does not match");
		ch = (char *) expected->return_value;
		for (i = 0; i < expected->return_len; i++)
			fprintf(stderr, "%s0x%x", (i != 0)? ", " : "", ch[i]);
		fprintf(stderr, "\n");
	}

	/*
	 * No chance of a match if lengths differ...
	 */
	if ((check == 0) && (expected->return_len != value->return_len))
	    errx(1, "Byte validation failed, length mismatch, expected %zu,"
		"received %zu", expected->return_len, value->return_len);

	/*
	 * If check is 0 then we want to throw an error IFF the byte streams
	 * do not match, if check is 1 then throw an error if the byte
	 * streams match.
	 */
	if (((check == 0) && memcmp(expected->return_value, value->return_value,
				    value->return_len) != 0) ||
	    ((check == 1) && (expected->return_len == value->return_len) &&
	     memcmp(expected->return_value, value->return_value,
		    value->return_len) == 0))
		errx(1, "Validate expected %s byte stream at line %zu"
		    "of file %s",
		    (check == 0)? "matching" : "not matching", line, cur_file);
	if (verbose) {
		fprintf(stderr, "Validated expected %s byte stream "
		    "at line %zu of file %s\n",
		    (check == 0)? "matching" : "not matching",
		    line, cur_file);
	}
}

/*
 * Validate the variable at i against the expected value, throw an
 * error if they don't match, if check is non-zero then the match is
 * negated.
 */
static void
validate_variable(int ret, returns_enum_t type, const void *value, int i,
    int check)
{
	returns_t *retval;
	var_t *varptr;

	retval = &command.returns[ret];
	varptr = &vars[command.returns[ret].return_index];

	if (varptr->value == NULL)
		err(1, "Variable %s has no value assigned to it", varptr->name);


	if (varptr->type != type)
		err(1, "Variable %s is not the expected type", varptr->name);

	if (type != ret_byte) {
		if ((((check == 0) && strcmp(value, varptr->value) != 0))
		    || ((check == 1) && strcmp(value, varptr->value) == 0))
			err(1, "Variable %s contains %s instead of %s"
			    " value %s at line %zu of file %s",
			    varptr->name, (const char *)varptr->value,
			    (check == 0)? "expected" : "not matching",
			    (const char *)value,
			    line, cur_file);
		if (verbose) {
			fprintf(stderr, "Variable %s contains %s value "
			    "%s at line %zu of file %s\n",
			    varptr->name,
			    (check == 0)? "expected" : "not matching",
			    (const char *)varptr->value, line, cur_file);
		}
	} else {
		if ((check == 0) && (retval->return_len != varptr->len))
			err(1, "Byte validation failed, length mismatch");

		/*
		 * If check is 0 then we want to throw an error IFF
		 * the byte streams do not match, if check is 1 then
		 * throw an error if the byte streams match.
		 */
		if (((check == 0) && memcmp(retval->return_value, varptr->value,
					    varptr->len) != 0) ||
		    ((check == 1) && (retval->return_len == varptr->len) &&
		     memcmp(retval->return_value, varptr->value,
			    varptr->len) == 0))
			err(1, "Validate expected %s byte stream at line %zu"
			    " of file %s",
			    (check == 0)? "matching" : "not matching",
			    line, cur_file);
		if (verbose) {
			fprintf(stderr, "Validated expected %s byte stream "
			    "at line %zu of file %s\n",
			    (check == 0)? "matching" : "not matching",
			    line, cur_file);
		}
	}
}

/*
 * Write a string to the command pipe - we feed the number of bytes coming
 * down first to allow storage allocation and then follow up with the data.
 * If cmd is NULL then feed a -1 down the pipe to say the end of the args.
 */
static void
write_cmd_pipe(char *cmd)
{
	args_t arg;
	size_t len;

	if (cmd == NULL)
		len = 0;
	else
		len = strlen(cmd);

	arg.arg_type = arg_static;
	arg.arg_len = len;
	arg.arg_string = cmd;
	write_cmd_pipe_args(arg.arg_type, &arg);

}

static void
write_cmd_pipe_args(args_state_t type, void *data)
{
	var_t *var_data;
	args_t *arg_data;
	int len, send_type;
	void *cmd;

	arg_data = data;
	switch (type) {
	case arg_var:
		var_data = data;
		len = var_data->len;
		cmd = var_data->value;
		if (type == arg_byte)
			send_type = ret_byte;
		else
			send_type = ret_string;
		break;

	case arg_null:
		send_type = ret_null;
		len = 0;
		break;

	default:
		if ((arg_data->arg_len == 0) && (arg_data->arg_string == NULL))
			len = -1;
		else
			len = arg_data->arg_len;
		cmd = arg_data->arg_string;
		if (type == arg_byte)
			send_type = ret_byte;
		else
			send_type = ret_string;
	}

	if (verbose) {
		fprintf(stderr, "Writing type %s to command pipe\n",
		    returns_enum_names[send_type]);
	}

	if (write(cmdpipe[WRITE_PIPE], &send_type, sizeof(int)) < 0)
		err(1, "command pipe write for type failed");

	if (verbose) {
		fprintf(stderr, "Writing length %d to command pipe\n", len);
	}

	if (write(cmdpipe[WRITE_PIPE], &len, sizeof(int)) < 0)
		err(1, "command pipe write for length failed");

	if (len > 0) {
		if (verbose) {
			fprintf(stderr, "Writing data >%s< to command pipe\n",
			    (const char *)cmd);
		}
		if (write(cmdpipe[WRITE_PIPE], cmd, len) < 0)
			err(1, "command pipe write of data failed");
	}
}

/*
 * Read a response from the command pipe, first we will receive the
 * length of the response then the actual data.
 */
static void
read_cmd_pipe(returns_t *response)
{
	int len, type;
	struct pollfd rfd[2];
	char *str;

	/*
	 * Check if there is data to read - just in case slave has died, we
	 * don't want to block on the read and just hang.  We also check
	 * output from the slave because the slave may be blocked waiting
	 * for a flush on its stdout.
	 */
	rfd[0].fd = slvpipe[READ_PIPE];
	rfd[0].events = POLLIN;
	rfd[1].fd = master;
	rfd[1].events = POLLIN;

	do {
		if (poll(rfd, 2, 4000) == 0)
			errx(2, "%s, %zu: Command pipe read timeout",
			    cur_file, line);

		if ((rfd[1].revents & POLLIN) == POLLIN) {
			if (verbose) {
				fprintf(stderr,
				    "draining output from slave\n");
			}
			save_slave_output(false);
		}
	}
	while((rfd[1].revents & POLLIN) == POLLIN);

	if (read(slvpipe[READ_PIPE], &type, sizeof(int)) < 0)
		err(1, "command pipe read for type failed");
	response->return_type = type;

	if ((type != ret_ok) && (type != ret_err) && (type != ret_count)) {
		if (read(slvpipe[READ_PIPE], &len, sizeof(int)) < 0)
			err(1, "command pipe read for length failed");
		response->return_len = len;

		if (verbose) {
			fprintf(stderr,
			    "Reading %d bytes from command pipe\n", len);
		}

		if ((response->return_value = malloc(len + 1)) == NULL)
			err(1, "Failed to alloc memory for cmd pipe read");

		if (read(slvpipe[READ_PIPE], response->return_value, len) < 0)
			err(1, "command pipe read of data failed");

		if (response->return_type != ret_byte) {
			str = response->return_value;
			str[len] = '\0';

			if (verbose) {
				fprintf(stderr, "Read data >%s< from pipe\n",
				    (const char *)response->return_value);
			}
		}
	} else {
		response->return_value = NULL;
		if (type == ret_count) {
			if (read(slvpipe[READ_PIPE], &len, sizeof(int)) < 0)
				err(1, "command pipe read for number of "
				       "returns failed");
			response->return_len = len;
		}

		if (verbose) {
			fprintf(stderr, "Read type %s from pipe\n",
			    returns_enum_names[type]);
		}
	}
}

/*
 * Check for writes from the slave on the pty, save the output into a
 * buffer for later checking if discard is false.
 */
#define MAX_DRAIN 256

static void
save_slave_output(bool discard)
{
	char *new_data, drain[MAX_DRAIN];
	size_t to_allocate;
	ssize_t result;
	size_t i;

	result = 0;
	for (;;) {
		if (result == -1)
			err(2, "poll of slave pty failed");
		result = MAX_DRAIN;
		if ((result = read(master, drain, result)) < 0) {
			if (errno == EAGAIN)
				break;
			else
				err(2, "draining slave pty failed");
		}
		if (result == 0)
			abort();

		if (!discard) {
			if ((size_t)result >
			    (saved_output.allocated - saved_output.count)) {
				to_allocate = 1024 * ((result / 1024) + 1);

				if ((new_data = realloc(saved_output.data,
					saved_output.allocated + to_allocate))
				    == NULL)
					err(2, "Realloc of saved_output failed");
				saved_output.data = new_data;
				saved_output.allocated += to_allocate;
			}

			if (verbose) {
				fprintf(stderr, "count = %zu, "
				    "allocated = %zu\n", saved_output.count,
				    saved_output.allocated);
				for (i = 0; i < (size_t)result; i++) {
					fprintf(stderr, "Saving slave output "
					    "at %zu: 0x%x (%c)\n",
					    saved_output.count + i, drain[i],
					    (drain[i] >= ' ')? drain[i] : '-');
				}
			}

			memcpy(&saved_output.data[saved_output.count], drain,
			       result);
			saved_output.count += result;

			if (verbose) {
				fprintf(stderr, "count = %zu, "
				    "allocated = %zu\n", saved_output.count,
				    saved_output.allocated);
			}
		} else {
			if (verbose) {
				for (i = 0; i < (size_t)result; i++) {
					fprintf(stderr, "Discarding slave "
					    "output 0x%x (%c)\n",
					    drain[i],
					    (drain[i] >= ' ')? drain[i] : '-');
				}
			}
		}
	}
}

static void
yyerror(const char *msg)
{
	warnx("%s in line %zu of file %s", msg, line, cur_file);
}
#line 1689 "testlang_parse.c"

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (short *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack)) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 14:
#line 207 "testlang_parse.y"
	{set_var(ret_number, yystack.l_mark[-1].string, yystack.l_mark[0].string);}
break;
case 16:
#line 208 "testlang_parse.y"
	{set_var(ret_number, yystack.l_mark[-3].string, yystack.l_mark[-1].string);}
break;
case 18:
#line 209 "testlang_parse.y"
	{set_var(ret_string, yystack.l_mark[-1].string, yystack.l_mark[0].string);}
break;
case 20:
#line 210 "testlang_parse.y"
	{set_var(ret_byte, yystack.l_mark[-1].string, yystack.l_mark[0].retval);}
break;
case 22:
#line 213 "testlang_parse.y"
	{
	do_function_call(1);
}
break;
case 23:
#line 218 "testlang_parse.y"
	{
	do_function_call(2);
}
break;
case 24:
#line 223 "testlang_parse.y"
	{
	do_function_call(3);
}
break;
case 25:
#line 228 "testlang_parse.y"
	{
	do_function_call(4);
 }
break;
case 26:
#line 233 "testlang_parse.y"
	{
	returns_t retvar;
	var_t *vptr;
	if (command.returns[0].return_index == -1)
		err(1, "Undefined variable in check statement, line %zu"
		    " of file %s", line, cur_file);

	if (verbose) {
		fprintf(stderr, "Checking contents of variable %s for %s\n",
		    vars[command.returns[0].return_index].name,
		    returns_enum_names[command.returns[1].return_type]);
	}

	if (((command.returns[1].return_type == ret_byte) &&
	     (vars[command.returns[0].return_index].type != ret_byte)) ||
	    vars[command.returns[0].return_index].type != ret_string)
		err(1, "Var type %s (%d) does not match return type %s (%d)",
		    returns_enum_names[
		    vars[command.returns[0].return_index].type],
		    vars[command.returns[0].return_index].type,
		    returns_enum_names[command.returns[1].return_type],
		    command.returns[1].return_type);

	switch (command.returns[1].return_type) {
	case ret_err:
		validate_variable(0, ret_string, "ERR",
				  command.returns[0].return_index, 0);
		break;

	case ret_ok:
		validate_variable(0, ret_string, "OK",
				  command.returns[0].return_index, 0);
		break;

	case ret_null:
		validate_variable(0, ret_string, "NULL",
				  command.returns[0].return_index, 0);
		break;

	case ret_nonnull:
		validate_variable(0, ret_string, "NULL",
				  command.returns[0].return_index, 1);
		break;

	case ret_string:
	case ret_number:
		if (verbose) {
			fprintf(stderr, " %s == returned %s\n",
			    (const char *)command.returns[1].return_value,
			    (const char *)
			    vars[command.returns[0].return_index].value);
		}
		validate_variable(0, ret_string,
		    command.returns[1].return_value,
		    command.returns[0].return_index, 0);
		break;

	case ret_byte:
		vptr = &vars[command.returns[0].return_index];
		retvar.return_len = vptr->len;
		retvar.return_type = vptr->type;
		retvar.return_value = vptr->value;
		validate_byte(&retvar, &command.returns[1], 0);
		break;

	default:
		err(1, "Malformed check statement at line %zu "
		    "of file %s", line, cur_file);
		break;
	}

	init_parse_variables(0);
 }
break;
case 27:
#line 308 "testlang_parse.y"
	{
	/* set the inter-character delay */
	if (sscanf(yystack.l_mark[-1].string, "%d", &input_delay) == 0)
		err(1, "delay specification %s could not be converted to "
		    "numeric at line %zu of file %s", yystack.l_mark[-1].string, line, cur_file);
	if (verbose) {
		fprintf(stderr, "Set input delay to %d ms\n", input_delay);
	}

	if (input_delay < DELAY_MIN)
		input_delay = DELAY_MIN;
	/*
	 * Fill in the timespec structure now ready for use later.
	 * The delay is specified in milliseconds so convert to timespec
	 * values
	 */
	delay_spec.tv_sec = input_delay / 1000;
	delay_spec.tv_nsec = (input_delay - 1000 * delay_spec.tv_sec) * 1000;
	if (verbose) {
		fprintf(stderr, "set delay to %jd.%jd\n",
		    (intmax_t)delay_spec.tv_sec,
		    (intmax_t)delay_spec.tv_nsec);
	}

	init_parse_variables(0);
 }
break;
case 28:
#line 336 "testlang_parse.y"
	{
	if (input_str != NULL) {
		warnx("%s, %zu: Discarding unused input string",
		    cur_file, line);
		free(input_str);
	}

	if ((input_str = malloc(strlen(yystack.l_mark[-1].string) + 1)) == NULL)
		err(2, "Cannot allocate memory for input string");

	strlcpy(input_str, yystack.l_mark[-1].string, strlen(yystack.l_mark[-1].string) + 1);
}
break;
case 29:
#line 351 "testlang_parse.y"
	{
	if (input_str != NULL) {
		warnx("%s, %zu: Discarding unused input string",
		    cur_file, line);
		free(input_str);
	}

	no_input = true;
 }
break;
case 31:
#line 363 "testlang_parse.y"
	{
	compare_streams(yystack.l_mark[-1].string, true);
}
break;
case 33:
#line 371 "testlang_parse.y"
	{
	compare_streams(yystack.l_mark[-1].string, false);
}
break;
case 37:
#line 382 "testlang_parse.y"
	{ assign_rets(ret_number, yystack.l_mark[0].string); }
break;
case 38:
#line 383 "testlang_parse.y"
	{ assign_rets(ret_number, yystack.l_mark[-1].string); }
break;
case 39:
#line 384 "testlang_parse.y"
	{ assign_rets(ret_string, yystack.l_mark[0].string); }
break;
case 40:
#line 385 "testlang_parse.y"
	{ assign_rets(ret_byte, (void *) yystack.l_mark[0].retval); }
break;
case 41:
#line 386 "testlang_parse.y"
	{ assign_rets(ret_err, NULL); }
break;
case 42:
#line 387 "testlang_parse.y"
	{ assign_rets(ret_ok, NULL); }
break;
case 43:
#line 388 "testlang_parse.y"
	{ assign_rets(ret_null, NULL); }
break;
case 44:
#line 389 "testlang_parse.y"
	{ assign_rets(ret_nonnull, NULL); }
break;
case 45:
#line 392 "testlang_parse.y"
	{
	assign_rets(ret_var, yystack.l_mark[0].string);
 }
break;
case 46:
#line 397 "testlang_parse.y"
	{
	assign_rets(ret_ref, yystack.l_mark[0].string);
 }
break;
case 47:
#line 401 "testlang_parse.y"
	{
	if (command.function != NULL)
		free(command.function);

	command.function = malloc(strlen(yystack.l_mark[0].string) + 1);
	if (command.function == NULL)
		err(1, "Could not allocate memory for function name");
	strcpy(command.function, yystack.l_mark[0].string);
 }
break;
case 49:
#line 414 "testlang_parse.y"
	{ yyval.string = get_numeric_var(yystack.l_mark[0].string); }
break;
case 50:
#line 416 "testlang_parse.y"
	{ yyval.string = numeric_or(yystack.l_mark[-2].string, yystack.l_mark[0].string); }
break;
case 52:
#line 420 "testlang_parse.y"
	{ assign_arg(arg_static, yystack.l_mark[-1].string); }
break;
case 54:
#line 421 "testlang_parse.y"
	{ assign_arg(arg_static, yystack.l_mark[0].string); }
break;
case 56:
#line 422 "testlang_parse.y"
	{ assign_arg(arg_static, yystack.l_mark[0].string); }
break;
case 58:
#line 423 "testlang_parse.y"
	{ assign_arg(arg_byte, yystack.l_mark[0].retval); }
break;
case 60:
#line 424 "testlang_parse.y"
	{ assign_arg(arg_static, yystack.l_mark[0].string); }
break;
case 62:
#line 425 "testlang_parse.y"
	{ assign_arg(arg_static, yystack.l_mark[0].string); }
break;
case 64:
#line 426 "testlang_parse.y"
	{ assign_arg(arg_static, yystack.l_mark[0].string); }
break;
case 66:
#line 427 "testlang_parse.y"
	{ assign_arg(arg_var, yystack.l_mark[0].string); }
break;
case 68:
#line 428 "testlang_parse.y"
	{ assign_arg(arg_null, yystack.l_mark[0].string); }
break;
#line 2179 "testlang_parse.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (short) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
