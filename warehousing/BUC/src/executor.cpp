#include "global.h"

void executeCommand()
{

    switch (parsedQuery.queryType)
    {
    case BUC:
        executeBUC();
        break;
    case MPRINT:
        executeMPRINT();
        break;
    default:
        cout << "PARSING ERROR" << endl;
    }

    return;
}

void printRowCount(int rowCount)
{
    cout << "\n\nRow Count: " << rowCount << endl;
    return;
}