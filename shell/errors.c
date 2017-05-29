#include "errors.h"

void print_err(int err, char* message)
{
    switch(err)
    {
    case E_MALLOC:
        fprintf(stderr, "%s: out of memory", message);
        exit(err);
    case E_USR:
        fprintf(stderr, "%s: can't get username", message);
        exit(err);
    case E_CHDIR:
        fprintf(stderr, "%s: can't change directory", message);
        break;
    case E_REDIR:
        fprintf(stderr, "%s: redirection failure", message);
        break;
    case E_QBREAK:
        fprintf(stderr, "%s: missing single or double quote", message);
        break;
    case E_UNEOLN:
        fprintf(stderr, "%s: unexpected end of line", message);
        break;
    case E_DOLLAR:
        fprintf(stderr, "%s: incorrect $ usage", message);
        break;
    case E_ARGCOF:
        fprintf(stderr, "%s: incorrect argument's number", message);
        break;
    case E_PIPERR:
        fprintf(stderr, "%s: incorrect | usage", message);
        break;
    case E_DREDIR:
        fprintf(stderr, "%s: double input/output redirection", message);
        break;
    }
    fputs("\n", stderr);
}