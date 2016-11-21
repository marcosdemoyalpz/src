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
extern YYSTYPE yylval;
