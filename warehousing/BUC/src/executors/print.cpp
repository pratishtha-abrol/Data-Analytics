#include "global.h"
/**
 * @brief
 * SYNTAX: PRINT relation_name
 */

bool syntacticParseMPRINT()
{
    logger.log("syntacticParseMPRINT");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = MPRINT;
    parsedQuery.printRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseMPRINT()
{
    logger.log("semanticParseMPRINT");
    if (!matrixCatalogue.isMatrix(parsedQuery.printRelationName))
    {
        cout << "SEMANTIC ERROR: Data doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeMPRINT()
{
    logger.log("executeMPRINT");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.printRelationName);
    matrix->print();
    return;
}
