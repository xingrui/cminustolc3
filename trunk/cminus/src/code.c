#include "globals.h"
#include "symtab.h"
#include "code.h"
static void cGen(TreeNode * tree, Symtab* symtab);

static void genExp(TreeNode * tree, Symtab* symtab) {
	static int s_op = 0;
	switch (tree->kind.exp) {
	case AssignK: {
		SymtabItem* pItem;
		genExp(tree->child[1], symtab);
		pItem = st_lookup(symtab, tree->child[0]->attr.name);
		if (pItem!=NULL) {
			switch (pItem->p.t) {
			case VAR:
				if (pItem->p.item.intVar.location>-17) {
					fprintf(code, "\tSTR R0, R%d, #%d ;%s\n", pItem->scope + 4,
							pItem->p.item.intVar.location, tree->child[0]->attr.name);
				} else {
					int num = -pItem->p.item.intVar.location-1;
					l_insert(num);
					fprintf(code, "\tLD R1, Literal%d\n", num);
					fprintf(code, "\tNOT R1, R1\n");
					fprintf(code, "\tADD R1, R%d, R1 ;%s\n", pItem->scope+4,
							tree->child[0]->attr.name);
					fprintf(code, "\tSTR R0, R1, #0\n");
				}
				break;
			case ARRAY:
				fprintf(code, "\tADD R6, R6, #-1; PUSH\n");
				fprintf(code, "\tSTR R0, R6, #0; PUSH\n");
				if (tree->child[0]->child[0]==NULL) {
					fprintf(stderr, "error in code generation\n");
					exit(EXIT_SUCCESS);
				}
				genExp(tree->child[0]->child[0], symtab);
				if (pItem->p.item.intVar.location>-17)
					fprintf(code, "\tADD R1, R%d, #%d ;%s\n", pItem->scope + 4,
							pItem->p.item.intVar.location, tree->child[0]->attr.name);
				else {
					int num = -pItem->p.item.intVar.location-1;
					l_insert(num);
					fprintf(code, "\tLD R1, Literal%d\n", num);
					fprintf(code, "\tNOT R1, R1\n");
					fprintf(code, "\tADD R1, R%d, R1\n", pItem->scope + 4);
				}
				if (pItem->p.item.intVar.location>0) {
					fprintf(code, "\tLDR R1, R1, #0\n");
				}
				fprintf(code, "\tADD R1, R0, R1\n");
				fprintf(code, "\tLDR R0, R6, #0; POP\n");
				fprintf(code, "\tADD R6, R6, #1; POP\n");
				fprintf(code, "\tSTR R0, R1, #0\n");
				break;
			default:
				break;
			}
		}
		break;
	}
	case OpK: {
		int opNum = s_op++;
		genExp(tree->child[0], symtab);
		fprintf(code, "\tADD R6, R6, #-1; PUSH\n");
		fprintf(code, "\tSTR R0, R6, #0; PUSH\n");
		genExp(tree->child[1], symtab);
		fprintf(code, "\tLDR R1, R6, #0; POP\n");
		fprintf(code, "\tADD R6, R6, #1; POP\n");
		switch (tree->attr.op) {
		case PLUS:
			fprintf(code, "\tADD R0, R0, R1\n");
			break;
		case MINUS:
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "\tADD R0, R0, R1\n");
			break;
		case TIMES:
			fprintf(code, "\tAND R3, R3, #0\n");
			fprintf(code, "\tADD R2, R3, #1\n");
			fprintf(code, "MutLoop%d: AND R4, R1, R2\n", opNum);
			fprintf(code, "\tBRz MutNext%d", opNum);
			fprintf(code, "\tADD R3, R3, R0\n");
			fprintf(code, "\tMutNext%d: ADD R0, R0, R0\n", opNum);
			fprintf(code, "\tBRz MutEnd%d\n", opNum);
			fprintf(code, "\tADD R2, R2, R2");
			fprintf(code, "\tBRnp MutLoop%d\n", opNum);
			fprintf(code, "\tMutEnd%d: ADD R0, R3, #0\n", opNum);
			break;
		case OVER:
			//R1/R0
			fprintf(code, "\tAND R2, R2, #0\n");
			fprintf(code, "\tAND R3, R3, #0\n");
			fprintf(code, "\tADD R0, R0, #0\n");
			fprintf(code, "\tBRn FLAG%d\n", opNum);
			fprintf(code, "\tBRz DIV%dEND\n", opNum);
			fprintf(code, "\tADD R3, R3, #1");
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "FLAG%d: ADD R1, R1, #0\n", opNum);
			fprintf(code, "\tBRp OVE%d\n", opNum);
			fprintf(code, "\tADD R3, R3, #1");
			fprintf(code, "\tNOT R1, R1\n");
			fprintf(code, "\tADD R1, R1, #1\n");
			fprintf(code, "OVE%d:\tADD R2, R2, #1\n", opNum);
			fprintf(code, "\tADD R1, R1, R0\n");
			fprintf(code, "\tBRzp OVE%d\n", opNum);
			fprintf(code, "\tADD R0, R2, #-1\n");
			fprintf(code, "\tAND R3, R3, #1");
			fprintf(code, "\tBRp DIV%dEND\n", opNum);
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "DIV%dEND:\tNOP\n", opNum);
			break;
		case EQUAL:
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "\tADD R0, R0, R1\n");
			fprintf(code, "\tBRz REOP%d\n", opNum);
			fprintf(code, "\tAND R0, R0, #0\n");
			fprintf(code, "\tBR OPEND%d\n", opNum);
			fprintf(code, "REOP%d: AND R0, R0, #0\n", opNum);
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "OPEND%d: NOP\n", opNum);
			break;
		case UNEQUAL:
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "\tADD R0, R0, R1\n");
			fprintf(code, "\tBRnp REOP%d\n", opNum);
			fprintf(code, "\tAND R0, R0, #0\n");
			fprintf(code, "\tBR OPEND%d\n", opNum);
			fprintf(code, "REOP%d: AND R0, R0, #0\n", opNum);
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "OPEND%d: NOP\n", opNum);
			break;
		case LESS:
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "\tADD R0, R0, R1\n");
			fprintf(code, "\tBRn REOP%d\n", opNum);
			fprintf(code, "\tAND R0, R0, #0\n");
			fprintf(code, "\tBR OPEND%d\n", opNum);
			fprintf(code, "REOP%d: AND R0, R0, #0\n", opNum);
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "OPEND%d: NOP\n", opNum);
			break;
		case LESSEQUAL:
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "\tADD R0, R0, R1\n");
			fprintf(code, "\tBRnz REOP%d\n", opNum);
			fprintf(code, "\tAND R0, R0, #0\n");
			fprintf(code, "\tBR OPEND%d\n", opNum);
			fprintf(code, "REOP%d: AND R0, R0, #0\n", opNum);
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "OPEND%d: NOP\n", opNum);
			break;
		case MORE:
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "\tADD R0, R0, R1\n");
			fprintf(code, "\tBRp REOP%d\n", opNum);
			fprintf(code, "\tAND R0, R0, #0\n");
			fprintf(code, "\tBR OPEND%d\n", opNum);
			fprintf(code, "REOP%d: AND R0, R0, #0\n", opNum);
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "OPEND%d: NOP\n", opNum);
			break;
		case MOREEQUAL:
			fprintf(code, "\tNOT R0, R0\n");
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "\tADD R0, R0, R1\n");
			fprintf(code, "\tBRzp REOP%d\n", opNum);
			fprintf(code, "\tAND R0, R0, #0\n");
			fprintf(code, "\tBR OPEND%d\n", opNum);
			fprintf(code, "REOP%d: AND R0, R0, #0\n", opNum);
			fprintf(code, "\tADD R0, R0, #1\n");
			fprintf(code, "OPEND%d: NOP\n", opNum);
			break;
		default:
			break;
		}
		break;
	}
	case IdK: {
		SymtabItem* pItem = st_lookup(symtab, tree->attr.name);
		if (pItem!=NULL) {
			switch (pItem->p.t) {
			case VAR:
				if (pItem->p.item.intVar.location>-17)
					fprintf(code, "\tLDR R0, R%d, #%d ;%s\n", pItem->scope + 4,
							pItem->p.item.intVar.location, tree->attr.name);
				else {
					int num = -pItem->p.item.intVar.location-1;
					l_insert(num);
					fprintf(code, "\tLD R1, Literal%d\n", num);
					fprintf(code, "\tNOT R1, R1\n");
					fprintf(code, "\tADD R1, R%d, R1 ;%s\n", pItem->scope + 4,
							tree->attr.name);
					fprintf(code, "\tLDR R0, R1, #0\n");
				}
				break;
			case ARRAY:
				if (pItem->p.item.intVar.location>-17)
					fprintf(code, "\tADD R0, R%d, #%d ;%s\n", pItem->scope + 4,
							pItem->p.item.intVar.location, tree->attr.name);
				else {
					int num = -pItem->p.item.intVar.location-1;
					l_insert(num);
					fprintf(code, "\tLD R1, Literal%d\n", num);
					fprintf(code, "\tNOT R1, R1\n");
					fprintf(code, "\tADD R0, R%d, R1 ;%s\n", pItem->scope + 4,
							tree->attr.name);
				}
				if (pItem->p.item.intVar.location>0) {
					fprintf(code, "\tLDR R0, R0, #0\n");
				}
				break;
			default:
				break;
			}
		}
		break;
	}
	case ArrayK: {
		SymtabItem* pItem;
		genExp(tree->child[0], symtab);
		pItem = st_lookup(symtab, tree->attr.name);
		if (pItem!=NULL) {
			if (pItem->p.item.intVar.location>-17)
				fprintf(code, "\tADD R1, R%d, #%d ;%s\n", pItem->scope + 4,
						pItem->p.item.intVar.location, tree->attr.name);
			else {
				int num = -pItem->p.item.intVar.location-1;
				l_insert(num);
				fprintf(code, "\tLD R1, Literal%d\n", num);
				fprintf(code, "\tNOT R1, R1\n");
				fprintf(code, "\tADD R1, R%d, R1 ;%s\n", pItem->scope + 4,
						tree->attr.name);
			}
			if (pItem->p.item.intVar.location>0) {
				fprintf(code, "\tLDR R1, R1, #0\n");
			}
			fprintf(code, "\tADD R1, R0, R1\n");
			fprintf(code, "\tLDR R0, R1, #0\n");
		}
		break;
	}
	case ConstK: {
		if (tree->attr.val<-32768) {
			fprintf(stderr,"error in code generation.\nNumber Out Of Range\n");
			exit(EXIT_SUCCESS);
		} else if (tree->attr.val<-16) {
			l_insert(-tree->attr.val-1);
			fprintf(code, "\tLD R0, Literal%d\n", -tree->attr.val-1);
			fprintf(code, "\tNOT R0, R0\n");
		} else if (tree->attr.val<16) {
			fprintf(code, "\tAND R0, R0, #0\n");
			fprintf(code, "\tADD R0, R0, #%d\n", tree->attr.val);
		} else if (tree->attr.val<32768) {
			l_insert(tree->attr.val);
			fprintf(code, "\tLD R0, Literal%d\n", tree->attr.val);
		} else {
			fprintf(stderr,"error in code generation.\nNumber Out Of Range\n");
			exit(EXIT_SUCCESS);
		}
		break;
	}
	case CallK: {
		int argNum = 0;
		TreeNode *t;
		fprintf(code, "\n\t;Function Call\n");
		t = tree->child[0];
		//printf("%s\n",tree->attr.name);
		while (t!=NULL) {
			genExp(t, symtab);
			t=t->sibling;
			fprintf(code, "\tADD R6, R6, #-1; PUSH args\n");
			fprintf(code, "\tSTR R0, R6, #0; PUSH args\n");
			++argNum;
		}
		fprintf(code, "\tJSR %s\n", tree->attr.name);
		fprintf(code, "\tLDR R0, R6, #0\n");
		fprintf(code, "\tADD R6, R6, #%d\n", argNum + 1);
	}
	default:
		break;
	}
}
static void genStmt(TreeNode * tree, Symtab* symtab) {
	static int s_if = 0;
	static int s_while = 0;
	switch (tree->kind.stmt) {
	case ExpStmtK:
		genExp(tree->child[0], symtab);
		break;
	case ComStmtK:
		if (tree->child[1]!=NULL)
			//genStmt(tree->child[1], symtab);
			genStmt(tree->child[1], tree->attr.symtab);
		break;
	case SelStmtK: {
		int ifNum = s_if++;
		genExp(tree->child[0], symtab);
		fprintf(code, "\tBRz NOT_TRUE%d\n", ifNum);
		genStmt(tree->child[1], symtab);
		if (tree->child[2] != NULL) {
			fprintf(code, "\tBR  ENDELSE%d\n", ifNum);
			fprintf(code, "NOT_TRUE%d:\n", ifNum);
			genStmt(tree->child[2], symtab);
			fprintf(code, "ENDELSE%d: NOP\n", ifNum);
		} else
			fprintf(code, "NOT_TRUE%d: NOP\n", ifNum);
		break;
	}
	case IteStmtK: {
		int whileNum = s_while++;
		fprintf(code, "LOOP%d: NOP\n", whileNum);
		genExp(tree->child[0], symtab);
		fprintf(code, "\tBRz DONE%d\n", whileNum);
		genStmt(tree->child[1], symtab);
		fprintf(code, "\tBR LOOP%d\n", whileNum);
		fprintf(code, "DONE%d: NOP\n", whileNum);
		break;
	}
	case RetStmtK:
		genExp(tree->child[0], symtab);
		fprintf(code, "\tSTR R0, R5, #3\n");
		break;
	default:
		;
	}
	if (tree->sibling != NULL) {
		genStmt(tree->sibling, symtab);
	}
}

void cGen(TreeNode * tree, Symtab* symtab) {
	if (tree != NULL) {
		switch (tree->nodeKind) {
		//		case StmtK:
		//			printf("cGen:StmtK\n");
		//			genStmt(tree, symtab);
		//			break;
		//		case ExpK:
		//			printf("cGen:ExpK\n");
		//			genExp(tree, symtab);
		//			break;
		case DeclK:
			switch (tree->kind.decl) {
			case FunDecK: {
				SymtabItem *pItem = st_lookup(symtab, tree->attr.name);
				if (pItem != NULL) {
					Symtab * s;
					//printf("fun name is %s\n", tree->attr.name);
					fprintf(code, "%s:\n", tree->attr.name);
					if (strcmp(tree->attr.name, "main")) {
						fprintf(code, "\tADD R6, R6, #-2\n");
						fprintf(code, "\tSTR R7, R6, #0\n");
						fprintf(code, "\tADD R6, R6, #-1\n");
						fprintf(code, "\tSTR R5, R6, #0\n");
						fprintf(code, "\tADD R5, R6, #-1\n");
					}
					s = pItem->p.item.fun.symtab;
					//allocate space for local variable...
					if (-s->offset>15) {
						l_insert(-s->offset);
						fprintf(code, "\tLD R1, Literal%d\n", -s->offset);
						fprintf(code, "\tNOT R1,R1\n");
						fprintf(code, "\tADD R1, R1, #1\n");
						fprintf(code, "\tADD R6, R6, R1\n");
						genStmt(tree->child[2], s);
						fprintf(code, "\tLD R1, Literal%d\n", -s->offset);
						fprintf(code, "\tADD R6, R6, R1\n");
					} else {
						fprintf(code, "\tADD R6, R6, #%d\n", s->offset);
						//statement-list of compound-stmt
						genStmt(tree->child[2], s);
						fprintf(code, "\tADD R6, R6, #%d\n", -s->offset);

					}
					if (strcmp(tree->attr.name, "main")) {
						fprintf(code, "\tLDR R5, R6, #0\n");
						fprintf(code, "\tADD R6, R6, #1\n");
						fprintf(code, "\tLDR R7, R6, #0\n");
						fprintf(code, "\tADD R6, R6, #1\n");
						fprintf(code, "\tRET\n");
					}
				}
				break;
			}
			default:
				break;
			}
		default:
			break;
		}
		cGen(tree->sibling, symtab);
	}
}

void output() {
	char lineBuf[100];
	FILE* out = fopen("io/output.asm", "r");
	while (fgets(lineBuf, 100, out)) {
		fprintf(code, "%s", lineBuf);
	}
}

void input() {
	char lineBuf[100];
	FILE* out = fopen("io/input.asm", "r");
	while (fgets(lineBuf, 100, out)) {
		fprintf(code, "%s", lineBuf);
	}
}

void addLiteral() {
	extern Literal* literal;
	LiteralItem*p = literal->first;
	while (p) {
		fprintf(code, "Literal%d: .FILL #%d\n", p->num, p->num);
		p=p->next;
	}
}

void codeGen(TreeNode* syntaxTree, Symtab* symtab) {
	fprintf(code, "\t.ORIG x3000\n");
	fprintf(code, "\tLEA R6, #-200\n");
	fprintf(code, "\tADD R5, R6, #-1\n");
	fprintf(code, "\tLEA R4, #-10\n");
	fprintf(code, "\tBR main\n");
	cGen(syntaxTree, symtab);
	fprintf(code, "\tHALT\n");
	addLiteral();
	output();
	input();
	fprintf(code, "\t.END\n");
}
