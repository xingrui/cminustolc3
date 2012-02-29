#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 6

extern FILE* source; /* source code text file */
extern FILE* listing; /* listing output text file */
extern FILE* code; /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

typedef enum
/* book-keeping tokens */
{	ENDFILE,ERROR,
	/* reserved words */
	IF,ELSE,INT,RETURN,VOID,WHILE,
	/* multicharacter tokens */
	ID,NUM,
	/* special symbols */
	PLUS,MINUS,TIMES,OVER,
	LESS,LESSEQUAL,MORE,MOREEQUAL,EQUAL,UNEQUAL,ASSIGN,
	SEMI,COMMA,
	LPAREN,RPAREN,LBRACKET,RBRACKET,LBRACE,RBRACE
} TokenType;

typedef enum {DeclK,StmtK,ExpK,SpecK,ParamK} NodeKind;

typedef enum {VarDecK,ArrayDecK,FunDecK} DeclKind;
typedef enum {ExpStmtK,ComStmtK,SelStmtK,IteStmtK,RetStmtK} StmtKind;
typedef enum {AssignK,OpK,ConstK,IdK,ArrayK,CallK} ExpKind;
typedef enum {IntSpec,VoidSpec} SpecKind;
typedef enum {IntPaK,ArrayPaK,VoidK} ParamKind;

#define MAXCHILDREN 3
struct SSymtab;
typedef struct treeNode {
	struct treeNode * child[MAXCHILDREN];
	struct treeNode * sibling;
	int lineno;
	NodeKind nodeKind;
	union {
		DeclKind decl;
		StmtKind stmt;
		ExpKind exp;
		SpecKind spec;
		ParamKind param;
	} kind;
	union {
		struct SSymtab* symtab;
		TokenType op;
		int val;
		char * name;
	} attr;
} TreeNode;

extern int EchoSource;
extern int TraceScan;
extern int TraceParse;
extern int TraceAnalyze;
extern int TraceCode;

#endif /*GLOBALS_H_*/
