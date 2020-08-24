#include "CatalogManager.h"
#include <iostream>
#include <fstream>
#include <string>
#include <qdebug.h>
#include <RuntimeManager.h>

using namespace std;

void CatalogManager::CreateTable(const string &table_name,
                                 const vector<pair<string, SqlValueType>> &attributes_list,
                                 const string &primary_key
)
{
    //1: Create a table new_table, update <Name> & <attrCnt>
    Table new_table;
    new_table.Name = table_name;
    new_table.attrCnt = (int) attributes_list.size();
    uint32_t record_length{0};

     //2: For each attributes, update <recordLength> & <attrName> & <second> & <attrType>
    for (const auto &attribute: attributes_list)
    {
        record_length += attribute.second.getSize();
        new_table.attrNames.push_back(attribute.first);
        SqlValueType svt_temp = attribute.second;
        svt_temp.attrName = attribute.first;
        if (attribute.first == primary_key)
        {
            svt_temp.primary=true;
            svt_temp.unique=true;
        }
        new_table.attrType.push_back(svt_temp);
    }

    //3: For each attributes, update <recordLength> & <recordCnt>
    new_table.recordLength = record_length;
    new_table.recordCnt = 0;

    //4: For all the unique attribute, create its Index
    char index_name{'A'};
    for (SqlValueType &svt_attribute: new_table.attrType)
    {
        if (svt_attribute.unique && !svt_attribute.primary)
        {
            new_table.index.push_back(make_pair(svt_attribute.attrName, string("auto_ind_") + (index_name++)));
            runtimManager.rm->createIndex(new_table, svt_attribute);
        }
    }

    //5: Push the new table into <tables>
    tables.push_back(new_table);
}

bool CatalogManager::CheckTableExist(const string &table_name) const
{
    //This function use a Lambda Function to check whether there is a table named "table_name" in <tables>
    return find_if(tables.begin(), tables.end(), [&table_name](const Table &tb)
    {
        return (tb.Name == table_name);
    }) != tables.end();
}

bool CatalogManager::CheckIndexExists(const string &index_name) const
{
    for (const auto &t: tables)
    {
        for (auto &i: t.index)
        {
            if (i.second == index_name) return true;
        }
    }
    return false;
}

Table &CatalogManager::GetTable_byName(const string &table_name)
{
    //This function use a Lambda Function to find a table named "table_name" in <tables>
    return *find_if(tables.begin(), tables.end(), [&table_name](const Table &tb)
    {
        return (tb.Name == table_name);
    });
}

Table &CatalogManager::GetTable_byIndex(const string &index_name)
{
    for (auto &t: tables)
    {
        for (const auto &i: t.index)
        {
            if (i.second == index_name) return t;
        }
    }
}


bool CatalogManager::RemoveTable(const Table &table)
{
    if (find_if(tables.begin(), tables.end(), [&table](const Table &tb)
    { return tb.Name == table.Name; }) == tables.end())
        return false;
    tables.erase(find_if(tables.begin(), tables.end(), [&table](const Table &tb)
    { return tb.Name == table.Name; }));
    return true;
}

CatalogManager::CatalogManager()
        : tables(vector<Table>())
{
    LoadFromFile();
}

CatalogManager::~CatalogManager()
{
    Flush();
}


void CatalogManager::Flush() const
{
    ofstream fs(file_name);
    fs << tables.size() << endl;

    // For each table
    for (const auto &tb: tables)
    {
        fs << tb.Name << endl;
        fs << tb.recordCnt << endl;
        ofstream tfs(tb.Name + ".catalog");
        uint16_t i{0};

        // For each attributes
        tfs << tb.attrNames.size() << endl;
        for (const auto &attr_name: tb.attrNames)
        {
            tfs << attr_name << endl;
            const auto &attr = tb.attrType[i];
            switch (attr.type)
            {
                case SqlValueTypeBase::Integer:
                    tfs << "int" << endl;
                    break;
                case SqlValueTypeBase::Float:
                    tfs << "float" << endl;
                    break;
                case SqlValueTypeBase::String:
                    tfs << "char" << endl;
                    break;
            }
            if (attr.type == SqlValueTypeBase::String)
            {
                tfs << attr.charSize << endl;
            } else
            {
                tfs << 0 << endl;
            }
            tfs << (attr.primary ? 1 : 0) << endl;
            tfs << (attr.unique ? 1 : 0) << endl;

            // if find the attribute has a index
            auto ind = find_if(tb.index.begin(), tb.index.end(),
                                    [&attr_name](const pair<string, string> &p)
                                    {
                                        return p.first == attr_name;
                                    });
            if (ind != tb.index.end())
            {
                tfs << 1 << endl << ind->second << endl;
            } else
            {
                tfs << 0 << endl << "-" << endl;
            }
            ++i;
        }
        tfs.close();
    }
    fs.close();
}

void CatalogManager::LoadFromFile()
{
    // This function opens the file <tables.meta>

    ifstream if_meta(file_name);
    if (!if_meta.is_open())
    {
        // if tables.meta doesnt exists, create one
        ofstream create_meta(file_name);
        return;
    }
    // <tables_count>: refers to the number of tables
    int t_count=0;
    if_meta >> t_count;

    // Read each table
    for (auto i = 0; i < t_count; ++i)
    {
        // Read a table, open its catalog
        string t_name;
        if_meta >> t_name;
        string file_name = t_name + ".catalog";
        ifstream if_tabcata(file_name);

        //Create a table, read in its recordCnt from meta
        Table tb;
        tb.Name = t_name;
        vector<SqlValueType> ind_vec;
        if_meta >> tb.recordCnt;

        int v;
        if_meta >> v;

        uint16_t attr_cnts{0};
        uint16_t record_length{0};

        //read in the counts of attributes
        int attr_counts=0;
        if_tabcata >> attr_counts;

        //read in each attributes
        for (auto ci = 0; ci < attr_counts; ++ci)
        {
            // define temp attributes detail
            string attr_name, type_name, index_name;
            uint16_t isPri, isUni, isInd, size;
            SqlValueType svt;

            if_tabcata >> attr_name >> type_name >> size >> isPri >> isUni >> isInd >> index_name;
            ++attr_cnts;

            tb.attrNames.push_back(attr_name);
            // check the type name
            if (type_name == "int")
            {
                svt.type = SqlValueTypeBase::Integer;
            } else if (type_name == "char")
            {
                svt.type = SqlValueTypeBase::String;
                svt.charSize = size;
            } else if (type_name == "float")
            {
                svt.type = SqlValueTypeBase::Float;
            } else
            {
            }
            //record_length update
            record_length += svt.getSize();
            svt.primary = isPri != 0;
            svt.unique = isUni != 0;
            svt.attrName = attr_name;

            // append the attrType & index
            tb.attrType.push_back(svt);
            if (isInd)
            {
                auto ind = make_pair(attr_name, index_name);
                tb.index.push_back(ind);
                ind_vec.push_back(svt);
            }
        }
        tb.attrCnt = attr_cnts;
        tb.recordLength = record_length;
        tables.push_back(tb);

        for(auto &it: ind_vec)
        {
            runtimManager.rm->createIndex(tb, it);
        }
    }
}
