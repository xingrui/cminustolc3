#ifndef UTIL_H_
#define UTIL_H_

void printToken(TokenType, const char *);
char *copyString(char *s);
TreeNode *newDeclNode(void);
TreeNode *newSpecNode(SpecKind spec);
TreeNode *newExpNode(ExpKind kind);
TreeNode *newParamNode(void);
TreeNode *newStmtNode(StmtKind kind);
void printTree(TreeNode *tree);

#endif /*UTIL_H_*/
