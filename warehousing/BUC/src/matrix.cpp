#include "global.h"

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded
 * @return false if an error occurred
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractColumnRowInfo(line))
        {
            if (this->blockify())
                return true;
        }
    }
    fin.close();
    return false;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file.
 *
 * @param line
 * @return true if column names successfully extracted 
 * @return false otherwise
 */
bool Matrix::extractColumnRowInfo(string firstLine)
{
    logger.log("Matrix::extractColumnRowInfo");
    unordered_set<string> columnNames;
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        if (columnNames.count(word))
            return false;
        columnNames.insert(word);
        this->columns.emplace_back(word);
    }
    this->columnCount = this->columns.size();
    // this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}
/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<string> row(this->columnCount-1);
    int pageCounter = 0;
    map<string, int> dummy;
    dummy.clear();
    this->nn.assign(this->columnCount, 0);
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    this->MaxlenValuesPerColumnCount.assign(this->columnCount, 0);
    this->MaxlenValuesPerColumn.assign(this->columnCount, "");
    vector<vector<string>> rowsAllPage;
    getline(fin, line);
    int colnum = 0;
    this->rowCount = 0;
    while (getline(fin, line))
    {
        stringstream s(line);
        int cc = 0;
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            // if(columnCounter == 6)
            // {
            //     if (!getline(s, word, ','))
            //         return false;
            //     int num = stoi(word)/15 + 1;
            //     string s = to_string(num);
            //     row[cc] = s;
            //     cc++;
            // }
            if(columnCounter == 7 || columnCounter==8)
            {
                if (!getline(s, word, ','))
                    return false;
                continue;
            }
            if(columnCounter == 9)
            {
                if (!getline(s, word, '"'))
                    return false;
                if (!getline(s, word, '"'))
                    return false;
                string temp =  word;
                temp.erase(std::remove(temp.begin(), temp.end(), ','), temp.end());
                row[cc] = temp;
                cc++;
                if (!getline(s, word, ','))
                    return false;
            }
            else
            {
                if (!getline(s, word, ','))
                    return false;
                row[cc] = word;
                cc++;
            }
        }
        this->updateStatistics(row);
        rowsAllPage.push_back(row);
    }
    int len = 0;
    this->maxColsPerBlock = -1;
    this->columnCount -= 2;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if(columnCounter>=7)
        {
            cout<<"No. of distinct values in Attribute named "<<this->columns[columnCounter+2]<<" is "<<this->distinctValuesPerColumnCount[columnCounter]<<endl;
            len += MaxlenValuesPerColumnCount[columnCounter]; 
        }
        else
        {
            cout<<"No. of distinct values in Attribute named "<<this->columns[columnCounter]<<" is "<<this->distinctValuesPerColumnCount[columnCounter]<<endl;
            len += MaxlenValuesPerColumnCount[columnCounter]; 
        }
        if((uint)((BLOCK_SIZE * 1000) / len) < 1 && this->maxColsPerBlock==-1)
        {
            this->maxRowsPerBlock = 1;
            this->maxColsPerBlock = columnCounter-1;
        }
    }
    if(this->maxColsPerBlock==-1)
    {
        this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (len+4));
        this->maxColsPerBlock = this->columnCount+1;
    }
    vector<vector<string>> rowsInPage(this->maxRowsPerBlock, vector<string>(this->maxColsPerBlock));
    this->count =0;
    auto start = high_resolution_clock::now();
    for(int i=0;i<this->columnCount;i++)
    {
        vector<string>v(this->columnCount, "*");
        buc(rowsAllPage, i, this->columnCount, rowsAllPage.size(), v);
    }
    while (this->blockCount*maxRowsPerBlock+pageCounter<this->ans.size())
    {

        stringstream s(line);
        string word;
        int count = 0;
        int flag = 0;
        int cc = 0;
        pageCounter++;
        colnum = 0;
        this->pageCount = 0;
        if (pageCounter == this->maxRowsPerBlock)
        {
            int size = (this->columnCount+1)/this->maxColsPerBlock;
            while(colnum<size)
            {
                vector<vector<string>>maxcolstore(this->maxRowsPerBlock, vector<string>(this->maxColsPerBlock));
                for(int i=0;i<pageCounter;i++)
                {
                    for(int j=0;j<this->maxColsPerBlock;j++)
                    {
                        if(j==this->maxColsPerBlock-1)
                            maxcolstore[i][j] = to_string(this->ans_i[i]);
                        else
                            maxcolstore[i][j] = this->ans[i+this->maxRowsPerBlock*this->blockCount][colnum*this->maxColsPerBlock+j];
                    }
                }
                colnum++;
                this->pageCount++;
                bufferManager.writePage(this->matrixName, this->blockCount, maxcolstore, pageCounter);
                this->blockCount++;
                this->rowsPerBlockCount.emplace_back(pageCounter);
                this->colsPerBlockCount.emplace_back(maxRowsPerBlock);
            }
            if(colnum*this->maxRowsPerBlock < this->columnCount)
            {
                int n = this->columnCount+1-colnum*this->maxColsPerBlock;
                vector<vector<string>>maxcolstore(this->maxRowsPerBlock, vector<string> (n));
                for(int i=0;i<pageCounter;i++)
                {
                    for(int j=0;j<n;j++)
                    {
                        if(j==this->maxColsPerBlock-1)
                            maxcolstore[i][j] = to_string(this->ans_i[i]);
                        else
                            maxcolstore[i][j] = this->ans[i+this->maxRowsPerBlock*this->blockCount][colnum*this->maxColsPerBlock+j];
                    }
                }
                colnum++;
                this->pageCount++;
                bufferManager.writePage(this->matrixName, this->blockCount, maxcolstore, pageCounter);
                this->blockCount++;
                this->rowsPerBlockCount.emplace_back(pageCounter);
                this->colsPerBlockCount.emplace_back(n);
            }
            pageCounter = 0;
        }
    }
    if (pageCounter)
    {
        colnum = 0;
        int size = (this->columnCount+1)/this->maxColsPerBlock;
        while(colnum<size)
        {
            vector<vector<string>>maxcolstore(pageCounter, vector<string>(this->maxColsPerBlock));
            for(int i=0;i<pageCounter;i++)
            {
                for(int j=0;j<this->maxColsPerBlock;j++)
                {
                    if(j==this->maxColsPerBlock-1)
                        maxcolstore[i][j] = to_string(this->ans_i[i]);
                    else
                        maxcolstore[i][j] = rowsAllPage[i+this->maxRowsPerBlock*this->blockCount][colnum*this->maxColsPerBlock+j];
                }
            }
            colnum++;
            this->pageCount++;
            bufferManager.writePage(this->matrixName, this->blockCount, maxcolstore, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            this->colsPerBlockCount.emplace_back(maxRowsPerBlock);
        }
        if(colnum*this->maxRowsPerBlock < this->columnCount)
        {
            int n = this->columnCount +1 -colnum*this->maxColsPerBlock;
            vector<vector<string>>maxcolstore(pageCounter, vector<string> (n));
            for(int i=0;i<pageCounter;i++)
            {
                for(int j=0;j<n;j++)
                {
                    if(j==this->maxColsPerBlock-1)
                            maxcolstore[i][j] = to_string(this->ans_i[i]);
                    else
                        maxcolstore[i][j] = rowsAllPage[i+this->maxRowsPerBlock*this->blockCount][colnum*this->maxColsPerBlock+j];
                }
            }
            colnum++;
            this->pageCount++;
            bufferManager.writePage(this->matrixName, this->blockCount, maxcolstore, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            this->colsPerBlockCount.emplace_back(n);
        }
        this->extraPage = pageCounter;
        pageCounter = 0;
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Time taken by function: "<< duration.count() << " microseconds" << endl;
    if (this->rowCount == 0)
        return false;
    return true;

}

void Matrix::buc(vector<vector<string>>data, int dim, int end_dim, int data_dim, vector<string>v)
{
    this->count++;
    // for(int columnCounter = dim; columnCounter < end_dim; columnCounter++)
    // {
    
    if(dim>=end_dim)
        return;
    int columnCounter = dim;
    int distinct = this->distinctValuesPerColumnCount[columnCounter];
    vector<int>freq(distinct, 0);
    // cout<<"here "<<columnCounter<<" "<<data[0][columnCounter]<<" "<<distinct<<endl;
    data = partition(data, columnCounter, distinct, freq, data_dim);
    // cout<<"hereee1\n";
    for(int i=0; i<distinct; i++)
    {
        int sup = freq[i];
        if(sup>=min_supp)
        {
            for(auto &it: this->distinctValuesInColumns[columnCounter])
            {
                if(it.second == i)
                    v[columnCounter] = it.first;
            }
            this->ans.push_back(v);
            this->ans_i.push_back(sup);
            int sum_part = 0;
            vector<vector<string>>sub_data(data.size(), vector<string>(this->columnCount));
            for(int part = 0; part<i; part++)
            {
                sum_part+= freq[part];
            }
            // cout<<"hereee4\n";
            for(int j=0; j<sup;j++)
            {
                for(int k=0;k<this->columnCount;k++)
                {
                    sub_data[j][k] = data[sum_part+j][k];
                }
            }
            // cout<<data[sum_part+1][dim]<<" "<<sup<<" "<<sub_data[0][0]<<" "<<this->count<<endl;
            buc(sub_data, dim+1, this->columnCount, sup, v);
            // cout<<"hereee5 "<<dim<<endl;
        }
    }
    // }
}

vector<vector<string>> Matrix::partition(vector<vector<string>>data, int dim, int c, vector<int> &freq, int data_dim)
{
    int count_freq[c+1] = {0};
    vector<vector<string>> sorted_data(data_dim, vector<string>(this->columnCount));
    for(int i=0;i<c;i++)
    {
        freq[i] = 0;
    }
    for(int i=0;i<data_dim;i++)
    {
        int temp_num = this->distinctValuesInColumns[dim][data[i][dim]];
        freq[temp_num]++;
    }

    count_freq[0] = freq[0];
    for(int i=1;i<c;i++)
    {
        count_freq[i]=count_freq[i-1]+freq[i];
    }

    for(int i=0;i<c;i++)
    {
        count_freq[i]-=1;
    }
    for(int i=0;i<data_dim;i++)
    {
        for(int j=0;j<this->columnCount;j++)
        {
            sorted_data[count_freq[this->distinctValuesInColumns[dim][data[i][dim]]]][j] = data[i][j];
        }
        count_freq[this->distinctValuesInColumns[dim][data[i][dim]]]--;
    }
    return sorted_data;
}
/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row
 */
void Matrix::updateStatistics(vector<string> row)
{
    this->rowCount++;
    for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(make_pair(row[columnCounter],nn[columnCounter]));
            nn[columnCounter]++;
            this->distinctValuesPerColumnCount[columnCounter]++;
            if(this->MaxlenValuesPerColumnCount[columnCounter]<row[columnCounter].length())
                this->MaxlenValuesPerColumnCount[columnCounter] = row[columnCounter].length();
            // if(this->MaxlenValuesPerColumn[columnCount]<row[columnCount])
            //     this->MaxlenValuesPerColumn[columnCount] = row[columnCount];
        }
    }
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print()
{
    logger.log("Matrix::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    //print headings
    // this->writeRow(this->columns, cout);

    vector<vector<string>> Totalrows(count, vector<string>(count));
    int prevPageIdx = 0;
    int rowCounter = 0, colPage = 0;
    int numofPages = count/this->maxRowsPerBlock;
    for(int row=0;row<numofPages;row++)
    {
        Cursor cursor(this->matrixName, row*this->pageCount);
        int n = min(this->maxRowsPerBlock,count - row*this->maxRowsPerBlock);
        for(int i=0;i<numofPages;i++)
        {
            vector<string> rows(this->maxRowsPerBlock);
            for(int rowCounter=0;rowCounter<n;rowCounter++)
            {
                rows = cursor.getNext();
                for(int j=0;j<rows.size();j++)
                {
                    if(i*maxRowsPerBlock +j<count && row*this->maxRowsPerBlock + rowCounter<count)
                    {
                        Totalrows[row*this->maxRowsPerBlock + rowCounter][i*this->maxRowsPerBlock+j] = rows[j];
                    }
                }
            }
        }
        if(numofPages*this->maxRowsPerBlock<count)
        {
            vector<string> rows(count - numofPages*this->maxRowsPerBlock);
            for(int rowCounter=0;rowCounter<n;rowCounter++)
            {
                rows = cursor.getNext();
                for(int j=0;j<rows.size();j++)
                {
                    if(numofPages*maxRowsPerBlock +j<count && row*this->maxRowsPerBlock + rowCounter<count)
                        Totalrows[row*this->maxRowsPerBlock + rowCounter][numofPages*this->maxRowsPerBlock+j] = rows[j];
                }
            }
        }
    }
    if(numofPages*this->maxRowsPerBlock<count)
    {
        Cursor cursor(this->matrixName, numofPages*this->pageCount);
        int n = min(this->maxRowsPerBlock,count - numofPages*this->maxRowsPerBlock);
        int m;
        if(count > this->maxRowsPerBlock)
            m = this->maxRowsPerBlock;
        else
            m = count;
        for(int i=0;i<numofPages;i++)
        {
            vector<string> rows(m);
            int pgIdx = cursor.pageIndex;
            
            for(int rowCounter=0;rowCounter<n;rowCounter++)
            {
                rows = cursor.getNext();
                for(int j=0;j<rows.size();j++)
                {
                    if(i*maxRowsPerBlock +j<count && numofPages*this->maxRowsPerBlock + rowCounter<count)
                    {
                        Totalrows[numofPages*this->maxRowsPerBlock + rowCounter][i*this->maxRowsPerBlock+j] = rows[j];
                    }
                }
            }
            cursor.nextPage(pgIdx+1);
        }
        if(numofPages*this->maxRowsPerBlock<count)
        {
            vector<string> rows(count-numofPages*this->maxRowsPerBlock);
            for(int rowCounter=0;rowCounter<n;rowCounter++)
            {
                rows = cursor.getNext();
                for(int j=0;j<rows.size();j++)
                {
                    if(numofPages*maxRowsPerBlock +j<count && numofPages*this->maxRowsPerBlock + rowCounter<count)
                    {    
                        // cout<<rows[j]<<endl;
                        Totalrows[numofPages*this->maxRowsPerBlock + rowCounter][numofPages*this->maxRowsPerBlock+j] = rows[j];
                    }
                }
            }
        }
    }
    for(int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        this->writeRow(Totalrows[rowCounter], cout);
    }
    printRowCount(count);
}

/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");

    if (cursor->pageIndex < this->blockCount - 1)
    {
        cursor->nextPage(cursor->pageIndex + 1);
    }
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
// void Matrix::makePermanent()
// {
//     logger.log("Matrix::makePermanent");
//     if (!this->isPermanent())
//         bufferManager.deleteFile(this->sourceFileName);
//     string newSourceFile = "../data/" + this->matrixName + ".csv";
//     ofstream fout(newSourceFile, ios::out);

//     // print headings
//     // this->writeRow(this->columns, fout);

//     uint count = this->rowCount;

//     //print headings
//     // this->writeRow(this->columns, cout);

//     vector<vector<string>> Totalrows(count, vector<string>(count));
//     int prevPageIdx = 0;
//     int rowCounter = 0, colPage = 0;
//     int numofPages = count/this->maxRowsPerBlock;
//     for(int row=0;row<numofPages;row++)
//     {
//         Cursor cursor(this->matrixName, row*this->pageCount);
//         int n = min(this->maxRowsPerBlock,count - row*this->maxRowsPerBlock);
//         for(int i=0;i<numofPages;i++)
//         {
//             vector<int> rows(this->maxRowsPerBlock);
//             for(int rowCounter=0;rowCounter<n;rowCounter++)
//             {
//                 rows = cursor.getNext();
//                 for(int j=0;j<rows.size();j++)
//                 {
//                     if(i*maxRowsPerBlock +j<count && row*this->maxRowsPerBlock + rowCounter<count)
//                     {
//                         Totalrows[row*this->maxRowsPerBlock + rowCounter][i*this->maxRowsPerBlock+j] = rows[j];
//                     }
//                 }
//             }
//         }
//         if(numofPages*this->maxRowsPerBlock<count)
//         {
//             vector<int> rows(count - numofPages*this->maxRowsPerBlock);
//             for(int rowCounter=0;rowCounter<n;rowCounter++)
//             {
//                 rows = cursor.getNext();
//                 for(int j=0;j<rows.size();j++)
//                 {
//                     if(numofPages*maxRowsPerBlock +j<count && row*this->maxRowsPerBlock + rowCounter<count)
//                         Totalrows[row*this->maxRowsPerBlock + rowCounter][numofPages*this->maxRowsPerBlock+j] = rows[j];
//                 }
//             }
//         }
//     }
//     if(numofPages*this->maxRowsPerBlock<count)
//     {
//         Cursor cursor(this->matrixName, numofPages*this->pageCount);
//         int n = min(this->maxRowsPerBlock,count - numofPages*this->maxRowsPerBlock);
//         int m;
//         if(count > this->maxRowsPerBlock)
//             m = this->maxRowsPerBlock;
//         else
//             m = count;
//         for(int i=0;i<numofPages;i++)
//         {
//             vector<int> rows(m);
//             int pgIdx = cursor.pageIndex;
            
//             for(int rowCounter=0;rowCounter<n;rowCounter++)
//             {
//                 rows = cursor.getNext();
//                 for(int j=0;j<rows.size();j++)
//                 {
//                     if(i*maxRowsPerBlock +j<count && numofPages*this->maxRowsPerBlock + rowCounter<count)
//                     {
//                         Totalrows[numofPages*this->maxRowsPerBlock + rowCounter][i*this->maxRowsPerBlock+j] = rows[j];
//                     }
//                 }
//             }
//             cursor.nextPage(pgIdx+1);
//         }
//         if(numofPages*this->maxRowsPerBlock<count)
//         {
//             vector<int> rows(count-numofPages*this->maxRowsPerBlock);
//             for(int rowCounter=0;rowCounter<n;rowCounter++)
//             {
//                 rows = cursor.getNext();
//                 for(int j=0;j<rows.size();j++)
//                 {
//                     if(numofPages*maxRowsPerBlock +j<count && numofPages*this->maxRowsPerBlock + rowCounter<count)
//                     {    
//                         // cout<<rows[j]<<endl;
//                         Totalrows[numofPages*this->maxRowsPerBlock + rowCounter][numofPages*this->maxRowsPerBlock+j] = rows[j];
//                     }
//                 }
//             }
//         }
//     }
//     for(int rowCounter = 0; rowCounter < count; rowCounter++)
//     {
//         this->writeRow(Totalrows[rowCounter], fout);
//     }
//     fout.close();
// }

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Matrix::unload()
{
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 *
 * @return Cursor
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0);
    return cursor;
}