#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "qstring.h"
#include "qstringlist.h"
#include "api.h"
#include "qfiledialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnSubmit_clicked()
{
    QFont font=ui->pteInput->font();
    ui->pteInput->setFont(font);

    QString str=ui->pteInput->toPlainText();
    QStringList list =str.split(";");

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
            api sql_command(*this, *(ui->Grid));
            //api sql_command(*this);
            sql_command.command(tmp);
        }
    }
    showMsg("<font color=\"#FFFFFF\">All Commands are done;</font>");
}

void MainWindow::showMsg(QString msg){    ui->tbStateList->append(msg);}

void MainWindow::on_btnFileBrow_clicked()
{
    //定义文件对话框类
    QFileDialog *fileDialog = new QFileDialog(this);
    //定义文件对话框标题
    fileDialog->setWindowTitle(QStringLiteral("Open a sql file"));
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置文件过滤器
    fileDialog->setNameFilter(tr("File(*.*)"));
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    //fileDialog->setFileMode(QFileDialog::ExistingFiles);
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //打印所有选择的文件的路径
    QStringList fileNames;
    if (fileDialog->exec()) {
        fileNames = fileDialog->selectedFiles();
    }
    if(fileNames.size()!=0)
    {
        QFile f(fileNames[0]);
        if (!f.open(QFile::ReadOnly | QFile::Text))
            return;
        int i =fileNames[0].lastIndexOf("/");
        QString loc = fileNames[0].left(i);
        file_location=loc;
        QTextStream in(&f);
        ui->pteInput->setPlainText(in.readAll());
    }
}

void MainWindow::on_btnFileBrow_2_clicked()
{
    QString path;
    path = QDir::currentPath();
    QString filename = QFileDialog::getSaveFileName(this, tr("Save SQL Script"), path, tr("SQL Files (*.sql)"));
    if(!filename.isNull())
    {
        QFile f(filename);
        if (!f.open(QFile::WriteOnly))
            return;
         QString str = ui->pteInput->toPlainText();		//将文本编辑框中的内容送给str
         f.write(str.toUtf8());
    }
    else
        return;
}
