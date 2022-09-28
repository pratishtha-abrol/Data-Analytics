#include "global.h"

bool semanticParse()
{
    logger.log("semanticParse");
    switch (parsedQuery.queryType)
    {
    case BUC:
        return semanticParseBUC();
    case MPRINT:
        return semanticParseMPRINT();
    default:
        cout << "SEMANTIC ERROR" << endl;
    }

    return false;
}