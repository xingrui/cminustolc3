#include "globals.h"
#include "util.h"
#include "scan.h"
#define MAXTOKENLEN 40

typedef enum {
    START,
    INNUM,
    INID,
    INLESS,
    INMORE,
    INEQUAL,
    INUNEQUAL,
    INOVER,
    INCOMMENT,
    INSTARCOMMENT,
    ENDSTARCOMMENT,
    DONE
} StateType;

char tokenString[MAXTOKENLEN + 1];

#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
 from lineBuf, reading in a new line if lineBuf is
 exhausted */
static int getNextChar(void)
{
    if (!(linepos < bufsize)) {
        lineno++;

        if (fgets(lineBuf, BUFLEN - 1, source)) {
            if (EchoSource)
                fprintf(listing, "%4d: %s", lineno, lineBuf);

            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        } else {
            EOF_flag = TRUE;
            return EOF;
        }
    } else
        return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
 in lineBuf */
static void ungetNextChar(void)
{
    if (!EOF_flag)
        linepos--;
}

/* lookup table of reserved words */
static struct {
    char* str;
    TokenType tok;
} reservedWords[MAXRESERVED] = { { "if", IF }, { "else", ELSE }, { "int", INT },
    { "return", RETURN }, { "void", VOID }, { "while", WHILE },
};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup(char* s)
{
    int i;

    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;

    return ID;
}

TokenType getToken(void)
{
    /* index for storing into tokenString */
    int tokenStringIndex = 0;
    /* holds current token to be returned */
    TokenType currentToken;
    /* current state - always begins at START */
    StateType state = START;
    /* flag to indicate save to tokenString */
    int save;

    while (state != DONE) {
        int c = getNextChar();
        save = TRUE;

        switch (state) {
            case START:

                if (isdigit(c))
                    state = INNUM;
                else if (isalpha(c))
                    state = INID;
                else if (c == '<')
                    state = INLESS;
                else if (c == '>')
                    state = INMORE;
                else if (c == '!')
                    state = INUNEQUAL;
                else if (c == '=')
                    state = INEQUAL;
                else if (c == '/')
                    state = INOVER;
                else if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
                    save = FALSE;
                else {
                    state = DONE;

                    switch (c) {
                        case EOF:
                            save = FALSE;
                            currentToken = ENDFILE;
                            break;

                        case '+':
                            currentToken = PLUS;
                            break;

                        case '-':
                            currentToken = MINUS;
                            break;

                        case '*':
                            currentToken = TIMES;
                            break;

                        case ';':
                            currentToken = SEMI;
                            break;

                        case ',':
                            currentToken = COMMA;
                            break;

                        case '(':
                            currentToken = LPAREN;
                            break;

                        case ')':
                            currentToken = RPAREN;
                            break;

                        case '[':
                            currentToken = LBRACKET;
                            break;

                        case ']':
                            currentToken = RBRACKET;
                            break;

                        case '{':
                            currentToken = LBRACE;
                            break;

                        case '}':
                            currentToken = RBRACE;
                            break;

                        default:
                            currentToken = ERROR;
                            break;
                    }
                }

                break;

            case INNUM:

                if (!isdigit(c)) { /* backup in the input */
                    ungetNextChar();
                    save = FALSE;
                    state = DONE;
                    currentToken = NUM;
                }

                break;

            case INID:

                if (!isalpha(c)) { /* backup in the input */
                    ungetNextChar();
                    save = FALSE;
                    state = DONE;
                    currentToken = ID;
                }

                break;

            case INLESS:
                state = DONE;

                if (c == '=') {
                    currentToken = LESSEQUAL;
                } else {
                    ungetNextChar();
                    save = FALSE;
                    currentToken = LESS;
                }

                break;

            case INMORE:
                state = DONE;

                if (c == '=') {
                    currentToken = MOREEQUAL;
                } else {
                    ungetNextChar();
                    save = FALSE;
                    currentToken = MORE;
                }

                break;

            case INEQUAL:
                state = DONE;

                if (c == '=') {
                    currentToken = EQUAL;
                } else {
                    ungetNextChar();
                    save = FALSE;
                    currentToken = ASSIGN;
                }

                break;

            case INUNEQUAL:
                state = DONE;

                if (c == '=') {
                    currentToken = UNEQUAL;
                } else {
                    ungetNextChar();
                    save = FALSE;
                    currentToken = ERROR;
                }

                break;

            case INOVER:

                if (c == '/') {
                    state = INCOMMENT;
                    save = FALSE;
                } else if (c == '*') {
                    state = INSTARCOMMENT;
                    save = FALSE;
                } else {
                    state = DONE;
                    ungetNextChar();
                    save = FALSE;
                    currentToken = OVER;
                }

                break;

            case ENDSTARCOMMENT:

                if (c == '/') {
                    save = FALSE;
                    tokenStringIndex = 0;
                    state = START;
                } else if (c == '*') {
                    save = FALSE;
                } else {
                    save = FALSE;
                    state = INSTARCOMMENT;
                }

                break;

            case INSTARCOMMENT:

                if (c != '*') {
                    save = FALSE;
                } else {
                    state = ENDSTARCOMMENT;
                    save = FALSE;
                }

                break;

            case INCOMMENT:

                if (c != '\n') {
                    save = FALSE;
                } else {
                    save = FALSE;
                    tokenStringIndex = 0;
                    state = START;
                }

                break;

            case DONE:
            default: /* should never happen */
                fprintf(listing, "Scanner Bug: state= %d\n", state);
                state = DONE;
                currentToken = ERROR;
                break;
        }

        if ((save) && (tokenStringIndex <= MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char) c;

        if (state == DONE) {
            tokenString[tokenStringIndex] = '\0';

            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }

    if (TraceScan) {
        fprintf(listing, "\t%d: ", lineno);
        printToken(currentToken, tokenString);
    }

    return currentToken;
}
