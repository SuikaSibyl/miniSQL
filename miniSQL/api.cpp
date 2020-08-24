#include "api.h"
#include "qdebug.h"
#include "qstring.h"
#include "qstringlist.h"
#include "CatalogManager.h"
#include <stdio.h>
#include <io.h>
#include <string>
#include <cstdio>
#include <ctime>
#include "DataStructure.h"
#include "RuntimeManager.h"
#include "qfile.h"

api::api(MainWindow& main, QTableWidget& tw):mainwindow(main),grid(tw){}

void api::setGrid(Result res)
{
    if(res.row.size()!=0)
    {
        int jm=res.row[0].col.size();
        int im=res.row.size();
        grid.setColumnCount(jm);
        grid.setRowCount(im);
        for(int j=0;j<jm;j++)
        {
            for(int i=0;i<im;i++)
            {
                QString temp = QString::fromStdString(res.row[i].col[j]);
                grid.setItem(i,j,new QTableWidgetItem(temp));
            }
        }
    }
    else
    {
        grid.setColumnCount(0);
        grid.setRowCount(0);
    }
}

void api::command(QString sql_command)
{
    clock_t  time_kp = clock();    //clock_t和clock()均来自#include <ctime>
    int iSpace_1=sql_command.indexOf(" ");
    int iSpace_2=sql_command.indexOf(" ",iSpace_1+1);
    if(sql_command.left(iSpace_2)==QString("create table"))
    {
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------**************************************------------------------------------------------------------------------------
        //                                                                                                                           * COMMAND CASE 1 : Create Tble  *
        //----------------------------------------------------------------------------------**************************************------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        int iBrace_leftest=sql_command.indexOf("(",iSpace_2+1);
        int iBrace_rightest=sql_command.lastIndexOf(")");
        // 1.1 Read table name
        //--------------------------
        QString t_name=sql_command.mid(iSpace_2+1,iBrace_leftest-iSpace_2-1).trimmed();
        // 1.2 Test table is exists or not
        //-----------------------------------
        if(runtimManager.cm->CheckTableExist(t_name.toStdString()))
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            qDebug()<<1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Create Table: table already exists; || Duration "+d_time+" msec </font> ");
            return;
        }
        // 1.3: Read middle part
        //---------------------------
        QString t_contain = sql_command. mid(iBrace_leftest+1,iBrace_rightest-iBrace_leftest-1).trimmed();
        QStringList t_attributes=t_contain.split(",");
        // 1.3.1: Check primary Key
        bool havePK=false;
        QString PK;
        QString last_part = t_attributes.at(t_attributes.size()-1).trimmed();
        QString ind_name;
        SqlValueType primary_key_type;
        iBrace_leftest=last_part.indexOf("(",0);
        QString instruction = last_part.left(iBrace_leftest);
        if(instruction=="primary key")
        {
            iBrace_rightest= last_part.indexOf(")",0);
            havePK=true;
            PK= last_part.mid(iBrace_leftest+1,iBrace_rightest-iBrace_leftest -1).trimmed();
        }
        // 1.3.2: Read each attributes
        vector<std::pair<std::string, SqlValueType>> schema_list;
        for(int i=0;i<t_attributes.size();i++)
        {
            SqlValueType svtTemp;
            QString tmp = t_attributes.at(i).trimmed();
            iSpace_1=tmp.indexOf(" ");
            iSpace_2=tmp.indexOf(" ",iSpace_1+1);
            int iSpace_3=tmp.indexOf(" ",iSpace_2+1);
            QString a_name=tmp.left(iSpace_1);
            svtTemp.attrName=a_name.toStdString();
            QString a_type = tmp.mid(iSpace_1+1,iSpace_2-iSpace_1-1);
            if(a_name=="primary"&&a_type.left(3)=="key")
                continue;
            if(a_type=="int")
                svtTemp.type=MINISQL_BASE::SqlValueTypeBase::Integer;
            else if(a_type=="float")
                svtTemp.type=MINISQL_BASE::SqlValueTypeBase::Float;
            else if(a_type.left(4)=="char")
            {
                iBrace_leftest=a_type.indexOf("(");
                iBrace_rightest=a_type.lastIndexOf(")");
                QString QSlength=a_type.mid(iBrace_leftest+1,iBrace_rightest-iBrace_leftest-1);
                unsigned int l=QSlength.toUInt();
                if(l<1 || l>255)
                {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    qDebug()<<1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Create Table: Char length out of range; || Duration "+d_time+" msec </font> ");
                    return;
                }
                 svtTemp.type=MINISQL_BASE::SqlValueTypeBase::String;
                 svtTemp.charSize=l;
            }
            if(a_name==PK)
            {
                svtTemp.unique=true;
                svtTemp.primary=true;
                primary_key_type = svtTemp;
                ind_name = "pri_" + t_name + "_" + PK;
            }
            QString may_unique=tmp.mid(iSpace_2+1,iSpace_3-iSpace_2-1);
            if(may_unique=="unique")
                svtTemp.unique=true;
            schema_list.push_back(pair<string,SqlValueType>(svtTemp.attrName,svtTemp));
            }
        // 1.4: Create table to managers
        //------------------------------------
            bool create_success = runtimManager.rm->createTable(t_name.toStdString());
            if(create_success==false)
                return;
            runtimManager.cm->CreateTable(t_name.toStdString(), schema_list, PK.toStdString());
            auto &tb = runtimManager.cm->GetTable_byName(t_name.toStdString());
        // 1.5: Create index for primary key
        //----------------------------------------
            if (havePK)
            {
                runtimManager.rm->createIndex(tb, primary_key_type);
                tb.index.push_back(std::make_pair(PK.toStdString(), ind_name.toStdString()));
            }
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#009955\"> Success: Create Table: "+t_name+"; || Duration "+d_time+" msec </font> ");
            runtimManager.cm->Flush();
    }
    else if(sql_command.left(iSpace_2)==QString("drop table"))
    {
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //-----------------------------------------------------------------------------------************************************-------------------------------------------------------------------------------
        //                                                                                                                             * COMMAND CASE 2 : Drop Tble  *
        //-----------------------------------------------------------------------------------************************************-------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        int iSemicolon=sql_command.lastIndexOf(";");
        // 2.1: Read table name
        //-------------------------
        QString t_name=sql_command.mid(iSpace_2+1,iSemicolon-iSpace_2-1).trimmed();
        if (!runtimManager.cm->CheckTableExist(t_name.toStdString()))
               {
                   QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                   mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Drop Table: "+t_name+": Table doesnt exist; || Duration "+d_time+" msec </font> ");
                   return;
               }
        auto &tb =runtimManager.cm->GetTable_byName(t_name.toStdString());

        // 2.2: Clean all the Indexes
        //------------------------------
       for (auto &it: tb.index)
       {
           auto ifn = it.first;
           runtimManager.rm->dropIndex(tb, ifn);
       }
        //2.3: Clean the table
       //------------------------
       runtimManager. cm->RemoveTable(tb);
       runtimManager. cm->Flush();
       bool remov_ssucc = runtimManager.rm->dropTable(t_name.toStdString());

       QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
       qDebug()<<1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC;
        if(remov_ssucc==true)
            mainwindow.showMsg("<font color=\"#009955\">Success: Drop table "+t_name+": Success;|| Duration "+d_time+" msec </font> ");
        else
            mainwindow.showMsg("<font color=\"#FF0000\">Failure: Drop table "+t_name+": Failed, for system reason;|| Duration "+d_time+" msec </font> ");
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    }
    else if(sql_command.left(iSpace_2)==QString("create index"))
    {
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // COMMAND CASE 3 : CREATE INDEX
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        int iSpace_3=sql_command.indexOf(" ",iSpace_2+1);
        int iSpace_4=sql_command.indexOf(" ",iSpace_3+1);
        int iBrace_l=sql_command.indexOf("(",iSpace_4+1);
        int iBrace_r=sql_command.indexOf(")",iBrace_l+1);
        string index_name =sql_command.mid(iSpace_2+1,iSpace_3-iSpace_2-1).trimmed().toStdString();
        QString t_name = sql_command.mid(iSpace_4+1,iBrace_l-iSpace_4-1).trimmed();
        string table_name = t_name.toStdString();
        string attribute_name = sql_command.mid(iBrace_l+1,iBrace_r-iBrace_l-1).trimmed().toStdString();

        //3.2 check whether the Index exitsts
        //-------------------------------------------
        if (runtimManager.cm->CheckIndexExists(index_name))
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Create index on "+t_name+" : index name exists; || Duration "+d_time+" msec </font> ");
            return;
        }

        //3.3 check whether the table exitsts
        //-------------------------------------------
        if (!runtimManager.cm->CheckTableExist(table_name))
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Create index on "+t_name+" : table not found; || Duration "+d_time+" msec </font> ");
            return;
        }
        auto &tb = runtimManager.cm->GetTable_byName(table_name);

        //3.4 check whether the index already exitsts
        //----------------------------------------------------
        for (auto &i: tb.index)
        {
            if (i.first == attribute_name)
            {
                if (i.second.find("auto_ind") == 0)
                {
                    i.second = index_name;
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#009955\"> Success: Create index: the table "+t_name +"; || Duration "+d_time+" msec </font> ");
                    return;
                }
                QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Create index on "+t_name+" : index on attributes already exists; || Duration "+d_time+" msec </font> ");
                return;
            }
        }

        //3.5 update table and check whether unique
        //----------------------------------------------------
        SqlValueType type;
        size_t i{0};
        for (; i < tb.attrNames.size(); ++i)
        {
            if (tb.attrNames[i] == attribute_name)
            {
                if (tb.attrType[i].unique)
                {
                    type = tb.attrType[i];
                    type.attrName = tb.attrNames[i];
                    break;
                } else
                {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Create index on "+t_name+" : Not a unique attribute; || Duration "+d_time+" msec </font> ");
                    return;
                }
            }
        }

        //3.6 update the RecordManager & CatalogManager
        //--------------------------------------------------------------
        auto b1 = runtimManager.rm->createIndex(tb, type);
        tb.index.push_back(std::make_pair(attribute_name, index_name));
        runtimManager.cm->Flush();
        if (b1)
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#009955\"> Success: Create index: the table "+t_name +"; || Duration "+d_time+" msec </font> ");
            return;
        } else
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Create index on "+t_name+" : System error; || Duration "+d_time+" msec </font> ");
            return;
        }
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    }
    else if(sql_command.left(iSpace_2)==QString("drop index"))
    {
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // COMMAND CASE 4 : DROP INDEX
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //4.1 Get index name
        //------------------------
        int iSpace_3=sql_command.indexOf(" ",iSpace_2+1);
        QString i_name=sql_command.mid(iSpace_2+1,iSpace_3-iSpace_2-1).trimmed();
        string index_name =i_name.toStdString();
        //4.2 Check whther the index exists
        //----------------------------------------
        bool e = runtimManager.cm->CheckIndexExists(index_name);
        if (!e)
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Drop index "+i_name+" : index not found; || Duration "+d_time+" msec </font> ");
            return;
        }
        //4.3 Delete the index in recordManager and CatalogManager
        //-------------------------------------------------------------------------
        auto &tb = runtimManager.cm->GetTable_byIndex(index_name);
        for (auto &ind: tb.index)
        {
            if (ind.second == index_name)
            {
                runtimManager.rm->dropIndex(tb, ind.first);
                tb.index.erase(std::find_if(tb.index.begin(), tb.index.end(),
                                            [&index_name](const std::pair<std::string, std::string> &it)
                                            { return it.second == index_name; }));
                runtimManager.cm->Flush();
                QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                mainwindow.showMsg("<font color=\"#009955\"> Success: Drop index "+i_name+" ; || Duration "+d_time+" msec </font> ");
                runtimManager.cm->Flush();
                return;
            }
        }
        return;
    }
    else if(sql_command.left(iSpace_1)==QString("select"))
    {
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // COMMAND CASE 5 : SELECT
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        int iSpace_3=sql_command.indexOf(" ",iSpace_2+1);
        int iSpace_4=sql_command.indexOf(" ",iSpace_3+1);
        QString t_name=sql_command.mid(iSpace_3+1,iSpace_4-iSpace_3-1).trimmed();
        string table_name=t_name.toStdString();
        int may_where=sql_command.indexOf("where");

        //5.2 Check whether the table exists
        //------------------------------------------
        if (!runtimManager.cm->CheckTableExist(table_name))
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Select from "+t_name+" : table not found; || Duration "+d_time+" msec </font> ");
            return;
        }
        auto &tb = runtimManager.cm->GetTable_byName(table_name);

        //5.3 Get <condition_list>
        //------------------------------------------
        vector<Condition> condition_list;
        if(may_where!=-1)
        {
            int iSpace_5=sql_command.indexOf(" ",iSpace_4+1);
            QString conditions = sql_command.right(sql_command.size()-iSpace_5-1);
            QStringList conditionsList = conditions.split("and");
            for(int i=0 ; i<conditionsList.size();i++)
            {
                QString condition = conditionsList.at(i).trimmed();
                QString QSvalue;
                Condition temp;
                int unequal =condition.indexOf("<>");
                int belowequal = condition.indexOf("<=");
                int greaterequal = condition.indexOf(">=") ;
                int greater = condition.indexOf(">") ;
                int equal = condition.indexOf("=") ;
                int less = condition.indexOf("<") ;
                if ( unequal!=-1) // case <>
                {
                    QSvalue=condition.right(condition.size()-unequal-2).trimmed();
                    temp.op=Operator::NE_OP;
                    temp.name=condition.left(unequal).trimmed().toStdString();
                }
                else if( belowequal != -1 ) // case <=
                {
                    qDebug()<<belowequal;
                    QSvalue=condition.right(condition.size()-belowequal-2).trimmed();
                    temp.op=Operator::LE_OP;
                    temp.name=condition.left(belowequal).trimmed().toStdString();
                }
                else if( greaterequal != -1 ) // case >=
                {
                    QSvalue=condition.right(condition.size()-greaterequal-2).trimmed();
                    temp.op=Operator::GE_OP;
                    temp.name=condition.left(greaterequal).trimmed().toStdString();
                }
                else if( equal != -1 ) // case =
                {
                    QSvalue=condition.right(condition.size()-equal-1).trimmed();
                    temp.op=Operator::EQ_OP;
                    temp.name=condition.left(equal).trimmed().toStdString();
                }
                else if( greater != -1 ) // case >
                {
                    QSvalue=condition.right(condition.size()-greater-1).trimmed();
                    temp.op=Operator::GT_OP;
                    temp.name=condition.left(greater).trimmed().toStdString();
                }
                else if( less != -1 ) // case <
                {
                    QSvalue=condition.right(condition.size()-less-1).trimmed();
                    temp.op=Operator::LT_OP;
                    temp.name=condition.left(less).trimmed().toStdString();
                }
                else
                {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Select from "+t_name+"; Unreadable condition; || Duration "+d_time+" msec </font> ");
                    return;
                }

                for(auto iter= tb.attrType.begin();iter!=tb.attrType.end();iter++)
                {
                    if(iter->attrName==temp.name)
                    {
                        temp.val.type=*iter;
                        if(temp.val.type.type==MINISQL_BASE::SqlValueTypeBase::String)
                        {
                            int quote_left = QSvalue.indexOf("'");
                            int quote_right = QSvalue.lastIndexOf("'");
                            QString QSS=QSvalue.mid(quote_left+1,quote_right-quote_left-1);
                            temp.val.str=QSS.toStdString();
                        }
                        else if(temp.val.type.type==MINISQL_BASE::SqlValueTypeBase::Integer)
                            temp.val.i=QSvalue.toInt();
                        else if(temp.val.type.type==MINISQL_BASE::SqlValueTypeBase::Float)
                            temp.val.r=QSvalue.toFloat();
                    }
                }
                condition_list.push_back(temp);
                // For each condition end --------------------------
            }
        }

        //5.4 Check <condition_list> with <tb>
        //---------------------------------------------
        for (const auto &cond: condition_list)
        {
            auto it = std::find(tb.attrNames.begin(), tb.attrNames.end(), cond.name);
            if (it == tb.attrNames.end())
            {
                QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Select from "+t_name+"; Attribute in conditions mismatch; || Duration "+d_time+" msec </font> ");
                return;
            }
            auto type = tb.attrType[it - tb.attrNames.begin()];
            if (type.type != cond.val.type.type)
            {
                QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Select from "+t_name+"; Type in conditions mismatch; || Duration "+d_time+" msec </font> ");
                return;
            }
        }

        //5.4 make <attr_list> & <cond_list>
        //---------------------------------------------
        vector<std::string>& attr_list(tb.attrNames);
        auto cond_list = std::vector<Cond>();
        for (const auto &it: condition_list)
        {
            cond_list.push_back(it);
        }

        //5.5 get records
        //------------------
        if (tb.index.size() == 0 || cond_list.size() == 0)
        {
           Result res = runtimManager.rm->selectRecord(tb, attr_list, cond_list);
           QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
           mainwindow.showMsg("<font color=\"#009955\"> Success: Select: the table "+t_name +"; || "+QString::number(res.row.size())+" row in set.; || Duration "+d_time+" msec </font> ");
           setGrid(res);
           return;
        }
        else
        {
            for (const auto &ind: tb.index)
            {
                for (const auto &cond: cond_list)
                {
                    if (ind.first == cond.attr)
                    {
                        if(QString::fromStdString(ind.second).left(9)=="auto_ind_")
                        {
                            Result res = runtimManager.rm->selectRecord(tb, attr_list, cond_list);
                            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                            mainwindow.showMsg("<font color=\"#009955\"> Success: Select: the table "+t_name +"; || "+QString::number(res.row.size())+" row in set.; || Duration "+d_time+" msec </font> ");
                            setGrid(res);
                            return;
                        }
                        IndexHint hint;
                        hint.attrName = cond.attr;
                        hint.cond = cond;
                        hint.attrType = cond.value.type.M();
                        Result res = runtimManager.rm->selectRecord(tb, attr_list, cond_list, hint);
                        QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                        mainwindow.showMsg("<font color=\"#009955\"> Success: Select: the table "+t_name +"; || "+QString::number(res.row.size())+" row in set; || Duration "+d_time+" msec </font> ");
                        setGrid(res);
                        return;
                    }
                }
            }
            Result res = runtimManager.rm->selectRecord(tb, attr_list, cond_list);
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#009955\"> Success: Select: the table "+t_name +"; || "+QString::number(res.row.size())+" row  in set; || Duration "+d_time+" msec </font> ");
            setGrid(res);
            return;
        }
    }
    else if(sql_command.left(iSpace_2)==QString("insert into"))
    {
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // COMMAND CASE 6 : INSERT
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // 6.1 Get table name
        //------------------------
        int iSpace_3=sql_command.indexOf(" ",iSpace_2+1);
        int iSpace_4=sql_command.indexOf(" ",iSpace_3+1);
        QString t_name=sql_command.mid(iSpace_2+1,iSpace_3-iSpace_2-1);
        int bran_left = sql_command.indexOf("(",iSpace_4+1);
        int bran_right = sql_command.lastIndexOf(")");
        vector<SqlValue> value_list;
        QString datas = sql_command.mid(bran_left+1,bran_right-bran_left-1);
        QStringList dataList = datas.split(",");
        for(int i = 0; i< dataList.size();i++)
        {
            QString data =  dataList.at(i).trimmed();
            SqlValue temp;

            int quote_left = data.indexOf("'");
            int quote_right = data.lastIndexOf("'");
            if(quote_left!=-1)
            {
                temp.type.type = MINISQL_BASE::SqlValueTypeBase::String;
                temp.str=data.mid(quote_left+1,quote_right-quote_left-1).toStdString();
            }
            else
            {
                int dot=data.indexOf(".");
                if(dot!=-1)
                {
                    temp.type.type = MINISQL_BASE::SqlValueTypeBase::Float;
                    temp.r=data.mid(quote_left+1,quote_right-quote_left-1).toFloat();
                }
                else
                {
                    temp.type.type = MINISQL_BASE::SqlValueTypeBase::Integer;
                    temp.i=data.mid(quote_left+1,quote_right-quote_left-1).toInt();
                }
            }
            value_list.push_back(temp);
        }

        //6.2 check whether the table exist
        //----------------------------------------
        if (!runtimManager.cm->CheckTableExist(t_name.toStdString()))
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Insert: the table "+t_name +" dosen't exist; || Duration "+d_time+" msec </font> ");
            return;
        }
        auto &tb = runtimManager.cm->GetTable_byName(t_name.toStdString());

        //6.3 check the number of attributes
        //------------------------------------------
        if (tb.attrNames.size() != value_list.size())
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Insert: the table "+t_name +" attribute count doesn't fit; || Duration "+d_time+" msec </font> ");
            return;
        }

        //6.4 update the <value_list> by comparing with tb.attrType
        //-----------------------------------------------------------------------
        for (size_t i = 0; i < value_list.size(); i++)
        {
            //6.4.1 update the type
            if (value_list[i].type.type != tb.attrType[i].type)
            {
                // A convert can be done from int to float
                if (value_list[i].type.type == SqlValueTypeBase::Integer &&
                    tb.attrType[i].type == SqlValueTypeBase::Float)
                {
                    value_list[i].type.type = SqlValueTypeBase::Float;
                    value_list[i].r = value_list[i].i;
                } else
                {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Insert: the table "+t_name +" attribute type doesn't fit; || Duration "+d_time+" msec </font> ");
                    return;
                }
            }
            //6.4.2 update attrName
            value_list[i].type.attrName = tb.attrNames[i];
            //6.4.3 check whether charSize fit the need.
            if (value_list[i].type.type == SqlValueTypeBase::String)
            {
                if (value_list[i].str.length() > tb.attrType[i].charSize)
                {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Insert: the table "+t_name +" char attribute is too long; || Duration "+d_time+" msec </font> ");
                    return;
                }
                value_list[i].type.charSize = tb.attrType[i].charSize;
            }
        }

        //6.5 check the <value_list> is unique or not
        //---------------------------------------------------
        IndexHint uniqueTest;
        std::vector<Cond> conds;
        conds.emplace_back();
        Cond &cond = conds[0];
        cond.cond = MINISQL_COND_EQUAL;

        for (auto &index : tb.index) {
            for (auto &val : value_list) {
                if (val.type.attrName != index.first) { continue; }
                cond.value = val;
                cond.attr = val.type.attrName;
                uniqueTest.attrName = cond.attr;
                uniqueTest.cond = cond;
                uniqueTest.attrType = cond.value.type.M();
                Result res = (runtimManager.rm->selectRecord(tb, tb.attrNames, conds, uniqueTest, false));
                if (res.row.size()>0) {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Insert: the table "+t_name +" attributeduplicate; || Duration "+d_time+" msec </font> ");
                    return;
                }
            }
        }

        // 6.6 insert records to recordManager
        //--------------------------------
        Tuple t;
        t.element = value_list;
        auto offset = runtimManager.rm->insertRecord(tb, t);

        // 6.7 insert records to indexManager
        //-------------------------------------------
        for (const auto &id: tb.index)
        {
            auto it = std::find(tb.attrNames.begin(), tb.attrNames.end(), id.first);
            bool insertSuccess = runtimManager.im->insert(indexFile(tb.Name, id.first),
                       value_list[it - tb.attrNames.begin()], offset);
//            if(!insertSuccess){
//                QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
//                mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Insert: Duplicate key; || Duration "+d_time+" msec </font> ");
//                return;
//            }
        }

        // 6.8 update the <recordCnt>
        //-----------------------------------
        tb.recordCnt++;
        QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
        mainwindow.showMsg("<font color=\"#009955\"> Success: Insert: the table "+t_name +"; || 1 row affected.; || Duration "+d_time+"msec </font> ");
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    }
    else if(sql_command.left(iSpace_2)==QString("delete from"))
    {
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // COMMAND CASE 7 : DELETE
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // 7.1 get <t_table> from input
        //------------------------------------
        int iSpace_3=sql_command.indexOf(" ",iSpace_2+1);
        int iSpace_4=sql_command.indexOf(" ",iSpace_3+1);
        QString t_name=sql_command.mid(iSpace_2+1,iSpace_3-iSpace_2-1);

        // 7.2 Check whether the table exists
        //------------------------------------------
        if (!runtimManager.cm->CheckTableExist(t_name.toStdString()))
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Delet From: the table "+t_name +" dosen't exist; || Duration "+d_time+" msec </font> ");
            return;
        }
        auto &tb = runtimManager.cm->GetTable_byName(t_name.toStdString());

        // 7.3 get <condition_list> from input
        //--------------------------------------------
        vector<Condition> condition_list;
        QString may_where=sql_command.mid(iSpace_3+1,iSpace_4-iSpace_3-1);
        // 7.1.1 if "where" exisits
        if(may_where=="where")
        {
            QString conditions = sql_command.right(sql_command.size()-iSpace_4);
            Condition temp;
            QStringList conditionsList = conditions.split("and");
            // For each condition, get it
            for(int i=0 ; i<conditionsList.size();i++)
            {
                QString condition = conditionsList.at(i).trimmed();
                QString QSvalue;
                int unequal =condition.indexOf("<>");
                int belowequal = condition.indexOf("<=");
                int greaterequal = condition.indexOf(">=") ;
                int greater = condition.indexOf(">") ;
                int equal = condition.indexOf("=") ;
                int less = condition.indexOf("<") ;
                if ( unequal!=-1) // case <>
                {
                    QSvalue=condition.right(condition.size()-unequal-2).trimmed();
                    temp.op=Operator::NE_OP;
                    temp.name=condition.left(unequal).trimmed().toStdString();
                }
                else if( belowequal != -1 ) // case <=
                {
                    qDebug()<<belowequal;
                    QSvalue=condition.right(condition.size()-belowequal-2).trimmed();
                    temp.op=Operator::LE_OP;
                    temp.name=condition.left(belowequal).trimmed().toStdString();
                }
                else if( greaterequal != -1 ) // case >=
                {
                    QSvalue=condition.right(condition.size()-greaterequal-2).trimmed();
                    temp.op=Operator::GE_OP;
                    temp.name=condition.left(greaterequal).trimmed().toStdString();
                }
                else if( equal != -1 ) // case =
                {
                    QSvalue=condition.right(condition.size()-equal-1).trimmed();
                    temp.op=Operator::EQ_OP;
                    temp.name=condition.left(equal).trimmed().toStdString();
                }
                else if( greater != -1 ) // case >
                {
                    QSvalue=condition.right(condition.size()-greater-1).trimmed();
                    temp.op=Operator::GT_OP;
                    temp.name=condition.left(greater).trimmed().toStdString();
                }
                else if( less != -1 ) // case <
                {
                    QSvalue=condition.right(condition.size()-less-1).trimmed();
                    temp.op=Operator::LT_OP;
                    temp.name=condition.left(less).trimmed().toStdString();
                }
                else
                {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Delete From "+t_name+"; Unreadable condition; || Duration "+d_time+" msec </font> ");
                    return;
                }

                for(auto iter= tb.attrType.begin();iter!=tb.attrType.end();iter++)
                {
                    qDebug()<<QSvalue;
                    if(iter->attrName==temp.name)
                    {
                        temp.val.type=*iter;
                        if(temp.val.type.type==MINISQL_BASE::SqlValueTypeBase::String)
                        {
                            int quote_left = QSvalue.indexOf("'");
                            int quote_right = QSvalue.lastIndexOf("'");
                            QString QSS=QSvalue.mid(quote_left+1,quote_right-quote_left-1);
                            temp.val.str=QSS.toStdString();
                        }
                        else if(temp.val.type.type==MINISQL_BASE::SqlValueTypeBase::Integer)
                            temp.val.i=QSvalue.toInt();
                        else if(temp.val.type.type==MINISQL_BASE::SqlValueTypeBase::Float)
                            temp.val.r=QSvalue.toFloat();
                    }
                }
                condition_list.push_back(temp);
                // For each condition end --------------------------
            }

            // Find the attribute, and check the type fit or not
            for (const auto &cond: condition_list)
            {
                auto it = std::find(tb.attrNames.begin(), tb.attrNames.end(), cond.name);
                if (it == tb.attrNames.end())
                {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Delet From: the table "+t_name +" : Attribute in conditions mismatch; || Duration "+d_time+" msec </font> ");
                    return;
                }
                auto type = tb.attrType[it - tb.attrNames.begin()];
                if (type.type != cond.val.type.type)
                {
                    QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
                    mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Delet From: the table "+t_name +" : Type in conditions mismatch; || Duration "+d_time+" msec </font> ");
                    return;
                }
            }
        }

        // 7.4 Create a <cond_list>
        //------------------------------
        auto cond_list = std::vector<Cond>();
        for (const auto &it: condition_list)
        {
            cond_list.push_back(it);
        }

        // 7.5 Delete the records in RecordManager
        //---------------------------------------------------
        bool r=true;
        r = runtimManager.rm->deleteRecord(tb, cond_list);
        if (r)
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#009955\"> Success: Delete From "+t_name+"; || Duration "+d_time+" msec </font> ");
        } else
        {
            QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
            mainwindow.showMsg("<font color=\"#009955\"> Failure: Delete From "+t_name+"; System Reason || Duration "+d_time+" msec </font> ");
        }
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    }
    else if(sql_command.left(iSpace_1)==QString("quit"))
    {
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------******************************-----------------------------------------------------------------------------------
        //                                                                                                                                 * COMMAND CASE 8 : quit  *
        //--------------------------------------------------------------------------------------******************************-----------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        runtimManager.bm->flushAllBlocks();
        mainwindow.showMsg("<font color=\"#009955\">Success: Quit ;</font>");
        mainwindow.close();
    }
    else if(sql_command.left(iSpace_1)==QString("execfile"))
    {
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------******************************-----------------------------------------------------------------------------------
        //                                                                                                                                 * COMMAND CASE  :   *
        //--------------------------------------------------------------------------------------******************************-----------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        QString filename = sql_command.mid(iSpace_1+1, iSpace_2-iSpace_1-1);
        QString filepath = mainwindow.file_location+"/"+filename;
        QFile f(filepath);
        if (!f.open(QFile::ReadOnly | QFile::Text))
            return;
        QTextStream in(&f);
        QString sqlCommands = in.readAll();
        QStringList list =sqlCommands.split(";");

        for(int i=0;i<list.size();i++)
        {
            qDebug()<<list.size();
            QString tmp = list.at(i);
            tmp.replace(QChar('\n'), QChar(' '));
            //tmp.replace(QChar('\s'), QChar(' '));
            tmp=tmp.trimmed();
            tmp=tmp.simplified();
            if(tmp!=QString("") && tmp!=QString(" ") )
            {
                tmp=tmp.toLower();
                api sql_command(mainwindow, grid );
                //api sql_command(*this);
                sql_command.command(tmp);
            }
        }
        mainwindow.showMsg("<font color=\"#009955\">Success: Execfile ;</font>");
    }
    else
    {
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------******************************-----------------------------------------------------------------------------------
        //                                                                                                                                 * COMMAND CASE 9 : else  *
        //--------------------------------------------------------------------------------------******************************-----------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        QString d_time=QString::number(1000 * (clock() - time_kp) / (double)CLOCKS_PER_SEC) ;
        mainwindow.showMsg("<font color=\"#FF0000\"> Failure: Syntax error, can't understand your command; || Duration "+d_time+" msec </font> ");
    }
}
