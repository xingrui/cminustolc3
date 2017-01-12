#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include "symtab.h"
#include "code.h"

/* allocate global variables */
int lineno = 0;
FILE* source;
FILE* listing;
FILE* code;

/* allocate and set tracing flags */
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;

int main(int argc, char* argv[])
{
    TreeNode* t;
    Symtab* symtab;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    source = fopen(argv[1], "r");

    if (source == NULL) {
        fprintf(stderr, "File %s not found\n", argv[1]);
        exit(1);
    }

    code = fopen("code/output.asm", "w");
    //code = stdout;
    listing = stdout; /* send listing to screen */
    t = parse();

    if (TraceParse) {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(t);
    }

    symtab = buildSymtab(t);
    createLiteral();

    if (TraceAnalyze) {
        fprintf(listing, "\nSymbol table:\n");
        printSymtab(symtab);
    }

    codeGen(t, symtab);
    return EXIT_SUCCESS;
}

