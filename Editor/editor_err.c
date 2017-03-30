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
        fwprintf(stderr, L"Incorrect symbol position\n");
        break;
    case E_MALLOC:
        fwprintf(stderr, L"Not enough memory\n");
        break;
    case E_LNRNG:
        fwprintf(stderr, L"Incorrect line number\n");
        break;
    case E_C_WRNG:
        fwprintf(stderr, L"Incorrect command\n");
        break;
    case E_NOARG:
        fwprintf(stderr, L"Missing or invalid arguments\n");
        break;
    case E_NARROW:
        fwprintf(stderr, L"Treminal\'s width not enough\n");
        break;
    case E_IOFAIL:
        fwprintf(stderr, L"Input/output error has occured\n");
        break;
    case E_NOFILE:
        fwprintf(stderr, L"No file specified\n");
        break;
    case E_NOTSAV:
        wprintf(L"File not saved. Use %sexit force%s to exit anyway.\n", RED, RESET);
        break;
    default:
        fwprintf(stderr, L"What the...");
    }
    return err;
}