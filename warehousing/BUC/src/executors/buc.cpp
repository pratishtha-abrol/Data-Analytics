#include "global.h"
/**
 * @brief
 * SYNTAX: BUC relation_name
 */

bool syntacticParseBUC()
{
    logger.log("syntacticParseBUC");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = BUC;
    parsedQuery.loadRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseBUC()
{
    logger.log("semanticParseBUC");
    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeBUC()
{
    logger.log("executeBUC");
    Matrix *matrix = new Matrix(parsedQuery.loadRelationName);
    if (matrix->load())
    {
        matrixCatalogue.insertMatrix(matrix);
        cout << "Analysed Data. Column Count: " << matrix->columnCount << " Row Count: " << matrix->rowCount << endl;
    }
    return;
}
