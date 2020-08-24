#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "qstring.h"
#include "mainwindow.h"
#include "BufferManager.h"
#include "IndexManager.h"
#include "RecordManager.h"
#include "CatalogManager.h"
#include "QTableWidget"

class api
{
public:
    MainWindow& mainwindow;
    QTableWidget &grid;
    api(MainWindow& main, QTableWidget &grid);
    void command(QString sql_command);
    void setGrid(Result res);
private:

};

#endif // INTERPRETER_H
