#ifndef CATALOG_H
#define CATALOG_H

#include <string>
#include <vector>
#include <map>
#include <DataStructure.h>

using namespace MINISQL_BASE;
using namespace std;

class CatalogManager
{
public:
    void CreateTable(const string &table_name,
                     const vector<pair<string, SqlValueType>> &attributes_list,
                     const string &primary_key
    );

    bool CheckTableExist(const string &table_name) const;
    bool CheckIndexExists(const string &index_name) const;

    Table &GetTable_byName(const string &table_name);
    Table &GetTable_byIndex(const string &index_name);

    bool RemoveTable(const Table &table);

    void Flush() const;

    void LoadFromFile();

    CatalogManager();
    ~CatalogManager();

private:
    vector<Table> tables;

    static constexpr auto file_name="tables.meta";

};

#endif // CATALOG_H
