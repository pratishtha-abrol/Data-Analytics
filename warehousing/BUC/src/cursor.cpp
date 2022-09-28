#include "global.h"

Cursor::Cursor(string tableName, int pageIndex)
{
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(tableName, pageIndex);
    this->pagePointer = 0;
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->rowsCount = this->page.rowCount;
    this->colsCount = this->page.colCount;
}

/**
 * @brief This function reads the next row from the page. The index of the
 * current row read from the page is indicated by the pagePointer(points to row
 * in page the cursor is pointing to).
 *
 * @return vector<int>
 */
vector<string> Cursor::getNext()
{
    logger.log("Cursor::geNext");
    vector<string> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if (result.empty())
    {
        if (matrixCatalogue.isMatrix(this->tableName))
            matrixCatalogue.getMatrix(this->tableName)->getNextPage(this);
        if (!this->pagePointer)
        {
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    return result;
}
/**
 * @brief Function that loads Page indicated by pageIndex. Now the cursor starts
 * reading from the new page.
 *
 * @param pageIndex
 */
void Cursor::nextPage(int pageIndex)
{
    logger.log("Cursor::nextPage");
    this->page = bufferManager.getPage(this->tableName, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}

Page Cursor::getPage()
{
    return this->page;
}