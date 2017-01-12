#ifndef SYMTAB_H_
#define SYMTAB_H_
#include "globals.h"

typedef struct SParam {
    char* name;
    ParamKind paramType;
    int location;
    struct SParam* next;
} Param;

typedef enum {
    VAR = 0, ARRAY = 1, FUN = 2, UNKNOWN = 3
} DecType;

typedef enum {
    GLOBAL = 0, LOCAL = 1
} Scope;

typedef struct {
    char* name;
    int location;
} IntVar;

typedef struct {
    char* name;
    int location;
    int size;
} ArrayVar;

struct SSymtab;

typedef struct {
    char* name;
    SpecKind retType;
    Param* first;
    struct SSymtab* symtab;
} Fun;

typedef struct {
    DecType t;
    union {
        IntVar intVar;
        ArrayVar arrayVar;
        Fun fun;
    } item;
} DeclItem;

typedef struct SSymtabItem {
    DeclItem p;
    Scope scope;
    struct SSymtabItem* next;
} SymtabItem;

typedef struct SSymtab {
    SymtabItem* first;
    Scope scope;
    struct SSymtab* parent;
    int offset;
} Symtab;

Symtab* CreateNewSymtab();
int st_insert(Symtab* symtab, DeclItem declItem);
SymtabItem* st_lookup(Symtab* symtab, char* name);
void printSymtab(Symtab* symtab);
Symtab* buildSymtab(TreeNode*);

typedef struct SLiteralItem {
    int num;
    struct SLiteralItem* next;
} LiteralItem;

typedef struct SLiteral {
    LiteralItem* first;
} Literal;

void createLiteral();
int l_insert(int num);
void printLiteral();

#endif /*SYMTAB_H_*/
