#include "editor_err.h"

/**
  * Recieves error code and prints error message to stderr stream
  * Recieved error code is returned
  */


int printerr(int err)
{
    switch(err)
    {
    case 0:
        break;
    case E_STRRNG:
        fwprintf(stderr, L"\tIncorrect symbol position\n");
        break;
    case E_MALLOC:
        fwprintf(stderr, L"\tNot enough memory\n");
        break;
    case E_LNRNG:
        fwprintf(stderr, L"\tIncorrect line number\n");
        break;
    case E_C_WRNG:
        fwprintf(stderr, L"\tIncorrect command\n");
        break;
    case E_NOARG:
        fwprintf(stderr, L"\tMissing or invalid arguments\n");
        break;
    case E_NARROW:
        fwprintf(stderr, L"\tTreminal\'s width not enough\n");
        break;
    case E_IOFAIL:
        fwprintf(stderr, L"\tInput/output error has occured\n");
        break;
    case E_NOFILE:
        fwprintf(stderr, L"\tNo file specified\n");
        break;
    case E_NOTSAV:
        wprintf(L"\tFile not saved. Use %sexit force%s to exit anyway.\n", RED, RESET);
        break;
    case E_EOF:
        fwprintf(stderr, L"\tEnd of input reached. Terminating process.\n");
        break;
    default:
        fwprintf(stderr, L"\tWhat the...\n");
    }
    return err;
}