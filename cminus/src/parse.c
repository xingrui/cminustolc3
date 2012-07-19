#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include "symtab.h"
int Error = FALSE;

static TokenType token;
static void syntaxError(char *message)
{
	fprintf(listing, "\n>>> ");
	fprintf(listing, "Syntax error at line %d: %s", lineno, message);
	Error = TRUE;
}
static void match(TokenType expected)
{
	if (token == expected)
		token = getToken();
	else
	{
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		fprintf(listing, "      ");
		exit(EXIT_SUCCESS);
	}
}

static TreeNode *program(void);
static TreeNode *declaration(void);
static TreeNode *typeSpecifier(void);
static TreeNode *varDeclaration(void);
static TreeNode *params(void);
static TreeNode *param(void);
static TreeNode *statement(void);
static TreeNode *expressionStmt(void);
static TreeNode *compoundStmt(void);
static TreeNode *selectionStmt(void);
static TreeNode *iterationStmt(void);
static TreeNode *returnStmt(void);
static TreeNode *expression(void);
static TreeNode *additiveExp(TreeNode *param);
static TreeNode *term(TreeNode *param);
static TreeNode *factor(TreeNode *param);
static TreeNode *argList(void);

TreeNode *program(void)
{
	TreeNode *t = declaration();
	TreeNode *p = t;

	while (token != ENDFILE)
	{
		TreeNode *q;
		q = declaration();

		if (q != NULL)
		{
			if (t == NULL)
				t = p = q;
			else
			{
				p->sibling = q;
				p = q;
			}
		}
	}

	return t;
}

TreeNode *declaration()
{
	TreeNode *t = newDeclNode();
	t->child[0] = typeSpecifier();

	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);

	match(ID);

	switch (token)
	{
	case SEMI:
		t->kind.decl = VarDecK;
		match(SEMI);
		break;
	case LBRACKET:
		t->kind.decl = ArrayDecK;
		match(LBRACKET);
		{
			TreeNode *tmp = newExpNode(ConstK);

			if ((tmp != NULL) && (token == NUM))
			{
				tmp->attr.val = atoi(tokenString);
			}

			match(NUM);
			t->child[1] = tmp;
		}
		match(RBRACKET);
		match(SEMI);
		break;
	case LPAREN:
		t->kind.decl = FunDecK;
		match(LPAREN);
		t->child[1] = params();
		match(RPAREN);
		t->child[2] = compoundStmt();
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		exit(EXIT_SUCCESS);
	}

	return t;
}

TreeNode *varDeclaration(void)
{
	TreeNode *t = newDeclNode();
	t->child[0] = typeSpecifier();

	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);

	match(ID);

	if (token == SEMI)
	{
		t->kind.decl = VarDecK;
		match(SEMI);
	}
	else
	{
		t->kind.decl = ArrayDecK;
		match(LBRACKET);
		{
			TreeNode *tmp = newExpNode(ConstK);

			if ((tmp != NULL) && (token == NUM))
			{
				tmp->attr.val = atoi(tokenString);
			}

			match(NUM);
			t->child[1] = tmp;
		}
		match(RBRACKET);
		match(SEMI);
	}

	return t;
}

TreeNode *typeSpecifier(void)
{
	TreeNode *t = NULL;

	if (token == INT)
	{
		match(INT);
		t = newSpecNode(IntSpec);
	}
	else if (token == VOID)
	{
		match(VOID);
		t = newSpecNode(VoidSpec);
	}
	else
	{
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		exit(EXIT_SUCCESS);
	}

	return t;
}

TreeNode *params(void)
{
	if (token == RPAREN)
	{
		TreeNode *t = newParamNode();
		t->kind.param = VoidK;
		return t;
	}
	else if (token == VOID)
	{
		TreeNode *t = newParamNode();
		t->kind.param = VoidK;
		match(VOID);
		return t;
	}
	else
	{
		TreeNode *t = param();
		TreeNode *p = t;

		while (token == COMMA)
		{
			match(COMMA);
			p->sibling = param();
			p = p->sibling;
		}

		return t;
	}
}

TreeNode *param(void)
{
	TreeNode *t = newParamNode();
	match(INT);

	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);

	match(ID);

	if (token == LBRACKET)
	{
		t->kind.param = ArrayPaK;
		match(LBRACKET);
		match(RBRACKET);
	}
	else
	{
		t->kind.param = IntPaK;
	}

	return t;
}

TreeNode *compoundStmt(void)
{
	TreeNode *t = newStmtNode(ComStmtK);
	match(LBRACE);

	if (token == INT || token == VOID)
	{
		TreeNode *p = varDeclaration();
		t->child[0] = p;

		while (token == INT || token == VOID)
		{
			p->sibling = varDeclaration();
			p = p->sibling;
		}
	}

	if (token != RBRACE)
	{
		TreeNode *p = statement();
		t->child[1] = p;

		while (token != RBRACE)
		{
			p->sibling = statement();
			p = p->sibling;
		}
	}

	match(RBRACE);
	return t;
}
TreeNode *selectionStmt(void)
{
	TreeNode *t = newStmtNode(SelStmtK);
	match(IF);
	match(LPAREN);
	t->child[0] = expression();
	match(RPAREN);
	t->child[1] = statement();

	if (token == ELSE)
	{
		match(ELSE);
		t->child[2] = statement();
	}

	return t;
}
TreeNode *iterationStmt(void)
{
	TreeNode *t = newStmtNode(IteStmtK);
	match(WHILE);
	match(LPAREN);
	t->child[0] = expression();
	match(RPAREN);
	t->child[1] = statement();
	return t;
}
TreeNode *returnStmt(void)
{
	TreeNode *t = newStmtNode(RetStmtK);
	match(RETURN);

	if (token != SEMI)
	{
		t->child[0] = expression();
	}

	match(SEMI);
	return t;
}
TreeNode *expressionStmt(void)
{
	TreeNode *t = newStmtNode(ExpStmtK);

	if (token == SEMI)
	{
		match(SEMI);
	}
	else
	{
		t->child[0] = expression();
		match(SEMI);
	}

	return t;
}

TreeNode *expression(void)
{
	TreeNode *t, *p;

	if (token == ID)
	{
		t = newExpNode(IdK);
		t->attr.name = copyString(tokenString);
		match(ID);

		if (token == LBRACKET)
		{
			match(LBRACKET);
			t->kind.exp = ArrayK;
			t->child[0] = expression();
			match(RBRACKET);
		}

		if (token == ASSIGN)
		{
			match(ASSIGN);
			p = newExpNode(AssignK);
			p->child[0] = t;
			p->child[1] = expression();
			t = p;
		}
		else
		{
			t = additiveExp(t);

			if ((token == LESS) || (token == LESSEQUAL) || (token == MORE) || (token
			        == MOREEQUAL) || (token == EQUAL) || (token == UNEQUAL))
			{
				TokenType OpToken = token;
				TreeNode *right;
				match(token);
				right = additiveExp(NULL);
				p = newExpNode(OpK);
				p->attr.op = OpToken;
				p->child[0] = t;
				p->child[1] = right;
				t = p;
			}
		}
	}
	else
	{
		t = additiveExp(NULL);

		if ((token == LESS) || (token == LESSEQUAL) || (token == MORE) || (token
		        == MOREEQUAL) || (token == EQUAL) || (token == UNEQUAL))
		{
			TokenType OpToken = token;
			TreeNode *right;
			match(token);
			right = additiveExp(NULL);
			p = newExpNode(OpK);
			p->attr.op = OpToken;
			p->child[0] = t;
			p->child[1] = right;
			t = p;
		}
	}

	return t;
}

TreeNode *additiveExp(TreeNode *param)
{
	TreeNode *left = term(param), *right, *p;

	while ((token == PLUS) || (token == MINUS))
	{
		switch (token)
		{
		case PLUS:
			match(PLUS);
			right = term(NULL);
			p = newExpNode(OpK);
			p->attr.op = PLUS;
			p->child[0] = left;
			p->child[1] = right;
			left = p;
			break;
		case MINUS:
			match(MINUS);
			right = term(NULL);
			p = newExpNode(OpK);
			p->attr.op = MINUS;
			p->child[0] = left;
			p->child[1] = right;
			left = p;
			break;
		default:
			;
		}
	}

	return left;
}

TreeNode *term(TreeNode *param)
{
	TreeNode *left = factor(param), *right, *p;

	while ((token == TIMES) || (token == OVER))
	{
		switch (token)
		{
		case TIMES:
			match(TIMES);
			right = factor(NULL);
			p = newExpNode(OpK);
			p->attr.op = TIMES;
			p->child[0] = left;
			p->child[1] = right;
			left = p;
			break;
		case OVER:
			match(OVER);
			right = factor(NULL);
			p = newExpNode(OpK);
			p->attr.op = OVER;
			p->child[0] = left;
			p->child[1] = right;
			left = p;
			break;
		default:
			;
		}
	}

	return left;
}

TreeNode *factor(TreeNode *param)
{
	TreeNode *t;

	if (param != NULL)
	{
		switch (token)
		{
		case LPAREN:

			if (param->kind.exp == IdK)
			{
				param->kind.exp = CallK;
				match(LPAREN);
				param->child[0] = argList();
				match(RPAREN);
				return param;
			}
			else
			{
				syntaxError("unexpected token -> ");
				printToken(token, tokenString);
				exit(EXIT_SUCCESS);
			}

		default:
			return param;
		}
	}

	switch (token)
	{
	case NUM:
		t = newExpNode(ConstK);
		t->attr.val = atoi(tokenString);
		match(NUM);
		break;
	case MINUS:
		match(MINUS);
		t = newExpNode(ConstK);
		t->attr.val = -atoi(tokenString);
		match(NUM);
		break;
	case LPAREN:
		match(LPAREN);
		t = expression();
		match(RPAREN);
		break;
	case ID:
		t = newExpNode(IdK);
		t->attr.name = copyString(tokenString);
		match(ID);

		switch (token)
		{
		case LBRACKET:
			t->kind.exp = ArrayK;
			match(LBRACKET);
			t->child[0] = expression();
			match(RBRACKET);
			break;
		case LPAREN:
			t->kind.exp = CallK;
			match(LPAREN);
			t->child[0] = argList();
			match(RPAREN);
			break;
		default:
			break;
		}

		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		exit(EXIT_SUCCESS);
	}

	return t;
}

//TreeNode * argList(void) {
//	TreeNode * t= NULL;
//	if (token != RPAREN) {
//		t = expression();
//		TreeNode * p = t;
//		while (token == COMMA) {
//			match(COMMA);
//			TreeNode * q = expression();
//			p->sibling = q;
//			p = q;
//		}
//	}
//	return t;
//}

TreeNode *argList(void)
{
	TreeNode *t = NULL;

	if (token != RPAREN)
	{
		t = expression();

		while (token == COMMA)
		{
			TreeNode *p;
			match(COMMA);
			p = expression();
			p->sibling = t;
			t = p;
		}
	}

	return t;
}

TreeNode *statement(void)
{
	TreeNode *t = NULL;

	switch (token)
	{
	case IF:
		t = selectionStmt();
		break;
	case WHILE:
		t = iterationStmt();
		break;
	case RETURN:
		t = returnStmt();
		break;
	case LBRACE:
		t = compoundStmt();
		break;
	default:
		t = expressionStmt();
		break;
	}

	return t;
}

TreeNode *parse(void)
{
	TreeNode *t;
	token = getToken();
	t = program();

	if (token != ENDFILE)
		fprintf(stderr, "error");

	return t;
}
