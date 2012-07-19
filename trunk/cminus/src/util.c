#include "globals.h"
#include "util.h"

void printToken(TokenType token, const char *tokenString)
{
	switch (token)
	{
	case IF:
	case ELSE:
	case INT:
	case RETURN:
	case VOID:
	case WHILE:
		fprintf(listing, "reserved word: %s\n", tokenString);
		break;
	case PLUS:
		fprintf(listing, "+\n");
		break;
	case MINUS:
		fprintf(listing, "-\n");
		break;
	case TIMES:
		fprintf(listing, "*\n");
		break;
	case OVER:
		fprintf(listing, "/\n");
		break;
	case LESS:
		fprintf(listing, "<\n");
		break;
	case LESSEQUAL:
		fprintf(listing, "<=\n");
		break;
	case MORE:
		fprintf(listing, ">\n");
		break;
	case MOREEQUAL:
		fprintf(listing, ">=\n");
		break;
	case EQUAL:
		fprintf(listing, "==\n");
		break;
	case UNEQUAL:
		fprintf(listing, "!=\n");
		break;
	case ASSIGN:
		fprintf(listing, "=\n");
		break;
	case SEMI:
		fprintf(listing, ";\n");
		break;
	case COMMA:
		fprintf(listing, ",\n");
		break;
	case LPAREN:
		fprintf(listing, "(\n");
		break;
	case RPAREN:
		fprintf(listing, ")\n");
		break;
	case LBRACKET:
		fprintf(listing, "[\n");
		break;
	case RBRACKET:
		fprintf(listing, "]\n");
		break;
	case LBRACE:
		fprintf(listing, "{\n");
		break;
	case RBRACE:
		fprintf(listing, "}\n");
		break;
	case ENDFILE:
		fprintf(listing, "EOF\n");
		break;
	case NUM:
		fprintf(listing, "NUM, val= %s\n", tokenString);
		break;
	case ID:
		fprintf(listing, "ID, name= %s\n", tokenString);
		break;
	case ERROR:
		fprintf(listing, "ERROR: %s\n", tokenString);
		break;
	default: /* should never happen */
		fprintf(listing, "Unknown token: %d\n", token);
	}
}

char *copyString(char *s)
{
	int n;
	char *t;

	if (s == NULL)
		return NULL;

	n = strlen(s) + 1;
	t = malloc(n);

	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
		strcpy(t, s);

	return t;
}

TreeNode *newDeclNode(void)
{
	TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
	int i;

	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
	{
		for (i = 0; i < MAXCHILDREN; i++)
			t->child[i] = NULL;

		t->sibling = NULL;
		t->nodeKind = DeclK;
		t->lineno = lineno;
	}

	return t;
}

TreeNode *newSpecNode(SpecKind spec)
{
	TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
	int i;

	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
	{
		for (i = 0; i < MAXCHILDREN; i++)
			t->child[i] = NULL;

		t->sibling = NULL;
		t->nodeKind = SpecK;
		t->kind.spec = spec;
		t->lineno = lineno;
	}

	return t;
}

TreeNode *newParamNode(void)
{
	TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
	int i;

	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
	{
		for (i = 0; i < MAXCHILDREN; i++)
			t->child[i] = NULL;

		t->sibling = NULL;
		t->nodeKind = ParamK;
		t->lineno = lineno;
	}

	return t;
}

TreeNode *newExpNode(ExpKind kind)
{
	TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
	int i;

	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
	{
		for (i = 0; i < MAXCHILDREN; i++)
			t->child[i] = NULL;

		t->sibling = NULL;
		t->nodeKind = ExpK;
		t->kind.exp = kind;
		t->lineno = lineno;
	}

	return t;
}

TreeNode *newStmtNode(StmtKind kind)
{
	TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
	int i;

	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
	{
		for (i = 0; i < MAXCHILDREN; i++)
			t->child[i] = NULL;

		t->sibling = NULL;
		t->nodeKind = StmtK;
		t->kind.stmt = kind;
		t->lineno = lineno;
	}

	return t;
}

static int indentno = 0;
#define INDENT indentno+=2
#define UNINDENT indentno-=2

static void printSpaces(void)
{
	int i;

	for (i = 0; i < indentno; i++)
		fprintf(listing, " ");
}

void printTree(TreeNode *tree)
{
	int i;
	INDENT;

	while (tree != NULL)
	{
		printSpaces();

		if (tree->nodeKind == DeclK)
		{
			switch (tree->kind.decl)
			{
			case VarDecK:
				fprintf(listing, "VarDec: %s\n", tree->attr.name);
				break;
			case ArrayDecK:
				fprintf(listing, "ArrayDec: %s\n", tree->attr.name);
				break;
			case FunDecK:
				fprintf(listing, "FunDecK: %s\n", tree->attr.name);
				break;
			default:
				fprintf(listing, "Unknown DeclKind kind\n");
				break;
			}
		}
		else if (tree->nodeKind == StmtK)
		{
			switch (tree->kind.stmt)
			{
			case SelStmtK:
				fprintf(listing, "Sel\n");
				break;
			case IteStmtK:
				fprintf(listing, "Ite\n");
				break;
			case ExpStmtK:
				fprintf(listing, "Exp\n");
				break;
			case ComStmtK:
				fprintf(listing, "Com\n");
				break;
			case RetStmtK:
				fprintf(listing, "Ret\n");
				break;
			default:
				fprintf(listing, "Unknown StmtNode kind\n");
				break;
			}
		}
		else if (tree->nodeKind == ExpK)
		{
			switch (tree->kind.exp)
			{
			case AssignK:
				fprintf(listing, "Assign:\n");
				break;
			case OpK:
				fprintf(listing, "Op: ");
				printToken(tree->attr.op, "\0");
				break;
			case ConstK:
				fprintf(listing, "Const: %d\n", tree->attr.val);
				break;
			case IdK:
				fprintf(listing, "Id: %s\n", tree->attr.name);
				break;
			case ArrayK:
				fprintf(listing, "Array: %s\n", tree->attr.name);
				break;
			case CallK:
				fprintf(listing, "Call: %s\n", tree->attr.name);
				break;
			default:
				fprintf(listing, "Unknown ExpNode kind\n");
				break;
			}
		}
		else if (tree->nodeKind == SpecK)
		{
			switch (tree->kind.spec)
			{
			case IntSpec:
				fprintf(listing, "Int\n");
				break;
			case VoidSpec:
				fprintf(listing, "Void\n");
				break;
			default:
				fprintf(listing, "Unknown SpecNode kind\n");
				break;
			}
		}
		else if (tree->nodeKind == ParamK)
		{
			switch (tree->kind.param)
			{
			case IntPaK:
				fprintf(listing, "IntPa: %s\n", tree->attr.name);
				break;
			case ArrayPaK:
				fprintf(listing, "ArrayPa: %s\n", tree->attr.name);
				break;
			case VoidK:
				fprintf(listing, "VoidPa\n");
				break;
			default:
				fprintf(listing, "Unknown ParamNode kind\n");
				break;
			}
		}
		else
			fprintf(listing, "Unknown node kind\n");

		for (i = 0; i < MAXCHILDREN; i++)
			printTree(tree->child[i]);

		tree = tree->sibling;
	}

	UNINDENT;
}
