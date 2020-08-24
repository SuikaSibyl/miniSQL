//
// Created by Haotian on 17/5/25.
//

#ifndef MINISQL_RECORDMANAGER_H
#define MINISQL_RECORDMANAGER_H

#include <vector>
#include <string>
#include "DataStructure.h"
#include "BufferManager.h"
#include "IndexManager.h"

using namespace std;
using namespace MINISQL_BASE;

class RecordManager {
public:
    RecordManager(BufferManager *bm, IndexManager *im) : bm(bm), im(im) {}

    ~RecordManager() = default;

    bool createTable(const string &table);

    bool dropTable(const string &table);

    bool createIndex(const Table &table, const SqlValueType &index);

    bool dropIndex(const Table &table, const string &index);

    int insertRecord(const Table &table, const Tuple &record);

     Result selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond);

    Result
    selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond, const IndexHint &indexHint,
                 bool isPrint = true);

    bool deleteRecord(const Table &table, const vector<Cond> &cond);

private:
    BufferManager *bm;
    IndexManager *im;

    void printResult(const Result &res) const;

    bool condsTest(const std::vector<Cond> &conds, const Tuple &tup, const std::vector<std::string> &attr);

    void convertToTuple(const char *blockBuffer, int offset, const std::vector<SqlValueType> &attrType, Tuple &tup);
};


#endif //MINISQL_RECORDMANAGER_H
