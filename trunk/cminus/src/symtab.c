#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

Symtab* CreateNewSymtab() {
	Symtab * s = malloc(sizeof(Symtab));
	s->first = NULL;
	s->parent = NULL;
	s->scope = GLOBAL;
	s->offset = 0;
	return s;
}

int st_insert(Symtab* symtab, DeclItem declItem) {
	SymtabItem *p = symtab->first;
	SymtabItem * s;
	while (p != NULL&& p->next != NULL) {
		if (strcmp(p->p.item.intVar.name, declItem.item.intVar.name) == 0)
			return FALSE;
		p = p->next;
	}
	if (p!=NULL&& strcmp(p->p.item.intVar.name, declItem.item.intVar.name) == 0)
		return FALSE;
	s = malloc(sizeof(SymtabItem));
	s->scope = symtab->scope;
	s->p = declItem;
	s->next = NULL;
	if (p == NULL) {
		symtab->first = s;
	} else {
		p->next = s;
	}
	return TRUE;
}

SymtabItem* st_lookup(Symtab* symtab, char *name) {
	SymtabItem* p = symtab->first;
	while (p != NULL&& p->next != NULL) {
		if (strcmp(p->p.item.intVar.name, name) == 0) {
			//printf("scope %d name:%s\n", p->scope,name);
			return p;
		}
		p = p->next;
	}
	if (p != NULL&& strcmp(p->p.item.intVar.name, name) == 0) {
		//printf("scope %d name:%s\n", p->scope,name);
		return p;
	}
	if (symtab->parent != NULL)
		return st_lookup(symtab->parent, name);
	else
		return NULL;
}
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=4
#define UNINDENT indentno-=4

/* printSpaces indents by printing spaces */
static void printSpaces(void) {
	int i;
	for (i=0; i<indentno; i++)
		fprintf(listing, " ");
}
void printSymtab(Symtab* symtab) {
	SymtabItem* p;
	if (symtab == NULL)
		return;
	p = symtab->first;
	while (p!=NULL) {
		printSpaces();
		switch (p->p.t) {
		case VAR:
			printf("VAR name:%s,location:%d\n", p->p.item.intVar.name,
					p->p.item.intVar.location);
			break;
		case ARRAY:
			printf("ARRAY name:%s,location:%d,size:%d\n", p->p.item.arrayVar.name,
					p->p.item.arrayVar.location, p->p.item.arrayVar.size);
			break;
		case FUN:
			printf("FUN name:%s, retType:%d\n", p->p.item.fun.name, p->p.item.fun.retType);
			{
				Param* pa = p->p.item.fun.first;
				while (pa != NULL) {
					printf("  Param name:%s, paramType:%d, location:%d\n",
							pa->name, pa->paramType, pa->location);
					pa = pa->next;
				}
			}
			INDENT;
			printSymtab(p->p.item.fun.symtab);
			UNINDENT;
			break;
		case UNKNOWN:
			break;
		}
		p = p->next;
	}
}

void varCheck(TreeNode *t, Symtab* symtab) {
	Symtab*pre;
	if (t == NULL)
		return;
	pre = symtab;
	if (t->nodeKind==StmtK && t->kind.stmt == ComStmtK) {
		symtab = t->attr.symtab;
	}
	if (t->nodeKind == ExpK && t->kind.exp == IdK) {
		SymtabItem* pItem = st_lookup(symtab, t->attr.name);
		if (pItem == NULL) {
			printf("Id name:%s\n", t->attr.name);
			exit(EXIT_SUCCESS);
		} else if (pItem->p.t != VAR && pItem->p.t != ARRAY) {
			printf("Id name:%s\n", t->attr.name);
			exit(EXIT_SUCCESS);
		}
	} else if (t->nodeKind == ExpK && t->kind.exp == ArrayK) {
		SymtabItem* pItem = st_lookup(symtab, t->attr.name);
		if (pItem == NULL) {
			printf("Array name:%s\n", t->attr.name);
			exit(EXIT_SUCCESS);
		} else if (pItem->p.t != ARRAY) {
			printf("Array name:%s\n", t->attr.name);
			exit(EXIT_SUCCESS);
		}
	} else if (t->nodeKind == ExpK && t->kind.exp == CallK) {
		SymtabItem* pItem = st_lookup(symtab, t->attr.name);
		if (pItem == NULL) {
			printf("Call name:%s\n", t->attr.name);
			exit(EXIT_SUCCESS);
		} else if (pItem->p.t != FUN) {
			printf("Call name:%s\n", t->attr.name);
			exit(EXIT_SUCCESS);
		}
	}
	{
		int i;
		for (i = 0; i < MAXCHILDREN; i++) {
			varCheck(t->child[i], symtab);
		}
		varCheck(t->sibling, pre);
	}
}

static void innerBuildSymtab(Symtab* symtab, TreeNode * node) {
	if (node==NULL)
		return;
	switch (node->nodeKind) {
	case DeclK:
		switch (node->kind.decl) {
		case VarDecK: {
			DeclItem it;
			it.t = VAR;
			it.item.intVar.location = symtab->offset;
			it.item.intVar.name = strdup(node->attr.name);
			symtab->offset -= 1;
			st_insert(symtab, it);
			break;
		}
		case ArrayDecK: {
			DeclItem it;
			it.t = ARRAY;
			it.item.arrayVar.size = node->child[1]->attr.val;
			symtab->offset -= it.item.arrayVar.size;
			it.item.arrayVar.location = symtab->offset + 1;
			it.item.arrayVar.name = strdup(node->attr.name);
			st_insert(symtab, it);
			break;
		}
		case FunDecK: {
			DeclItem it;
			it.t = FUN;
			it.item.fun.name = strdup(node->attr.name);
			it.item.fun.retType = node->child[0]->kind.spec;
			it.item.fun.first = NULL;
			it.item.fun.symtab = CreateNewSymtab();
			it.item.fun.symtab->scope = LOCAL;
			{
				TreeNode * paramNode = node->child[1];
				Param * pt= NULL;
				int offset = 4;
				while (paramNode != NULL) {
					Param* p = malloc(sizeof(Param));
					p->paramType = paramNode->kind.param;
					if (p->paramType != VoidK) {
						p->name = paramNode->attr.name;
					} else
						break;
					if (p->paramType == IntPaK) {
						DeclItem dt;
						dt.t = VAR;
						dt.item.intVar.location = offset;
						dt.item.intVar.name = strdup(paramNode->attr.name);
						st_insert(it.item.fun.symtab, dt);
					} else if (p->paramType == ArrayPaK) {
						DeclItem dt;
						dt.t = ARRAY;
						dt.item.arrayVar.location = offset;
						dt.item.arrayVar.name = strdup(paramNode->attr.name);
						dt.item.arrayVar.size = 0;
						st_insert(it.item.fun.symtab, dt);
					}
					p->next = NULL;
					if (pt == NULL) {
						it.item.fun.first = p;
					} else {
						pt->next = p;
					}
					p->location = offset;
					++offset;
					pt = p;
					paramNode = paramNode->sibling;
				}
			}
			it.item.fun.symtab->parent = symtab;
			node->child[2]->attr.symtab = it.item.fun.symtab;
			st_insert(symtab, it);
			{
				TreeNode * t = node->child[2]->child[0];
				while (t != NULL) {
					innerBuildSymtab(it.item.fun.symtab, t);
					t = t->sibling;
				}
				t = node->child[2]->child[1];
				while (t != NULL) {
					innerBuildSymtab(it.item.fun.symtab, t);
					t = t->sibling;
				}
			}
			{
				TreeNode * t = node->child[2]->child[1];
				varCheck(t, it.item.fun.symtab);
			}
			break;
		}
		}
		break;
	case StmtK:
		switch (node->kind.stmt) {
		case ComStmtK: {
			TreeNode * t;
			Symtab*com_symtab = CreateNewSymtab();
			com_symtab->parent = symtab;
			com_symtab->offset = symtab->offset;
			node->attr.symtab = com_symtab;
			t = node->child[0];
			while (t!=NULL) {
				innerBuildSymtab(com_symtab, t);
				t = t->sibling;
			}
			t = node->child[1];
			while (t != NULL) {
				innerBuildSymtab(com_symtab, t);
				t = t->sibling;
			}
			break;
		}
		case SelStmtK:
			innerBuildSymtab(symtab, node->child[1]);
			innerBuildSymtab(symtab, node->child[2]);
			break;
		case IteStmtK:
			innerBuildSymtab(symtab, node->child[1]);
			break;
		default:
			break;
		}

	default:
		break;
	}
}

Symtab* buildSymtab(TreeNode * node) {
	Symtab* symtab = CreateNewSymtab();
	Param* pparam;
	DeclItem input;
	DeclItem output;
	TreeNode* t;
	input.t = FUN;
	input.item.fun.name = strdup("input");
	input.item.fun.retType = IntSpec;
	input.item.fun.first = NULL;
	input.item.fun.symtab = NULL;
	st_insert(symtab, input);

	pparam = malloc(sizeof(Param));
	pparam->name = strdup("value");
	pparam->paramType = IntPaK;
	pparam->location = 4;
	pparam->next = NULL;

	output.t = FUN;
	output.item.fun.name = strdup("output");
	output.item.fun.retType = VoidSpec;
	output.item.fun.first = pparam;
	output.item.fun.symtab = NULL;
	st_insert(symtab, output);

	t = node;
	while (t != NULL) {
		innerBuildSymtab(symtab, t);
		t = t->sibling;
	}
	return symtab;
}

Literal *literal=NULL;

void createLiteral() {
	if (literal==NULL) {
		literal = malloc(sizeof(Literal));
		literal->first = NULL;
	}
}

int l_insert(int num) {
	if (literal->first==NULL||literal->first->num>num) {
		LiteralItem *p = malloc(sizeof(LiteralItem));
		p->num = num;
		p->next = literal->first;
		literal->first = p;
	} else if (literal->first->num<num) {
		LiteralItem*it = literal->first;
		while (it->next!=NULL&&it->next->num<num) {
			it = it->next;
		}
		if (it->next==NULL) {
			LiteralItem *p = malloc(sizeof(LiteralItem));
			p->num = num;
			p->next = NULL;
			it->next = p;
		} else if (it->next->num>num) {
			LiteralItem *p = malloc(sizeof(LiteralItem));
			p->num = num;
			p->next = it->next;
			it->next = p;
		}
	}
	return 0;
}

void printLiteral(Literal* literal) {
	LiteralItem*p = literal->first;
	while (p) {
		printf("literal num %d\n", p->num);
		p=p->next;
	}
}

