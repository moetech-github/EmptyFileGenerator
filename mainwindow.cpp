#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "QDir"
#include "windows.h"
#include "QString"
#include "QUuid"
#include "QtMath"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ShowHelpInfo();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//void MainWindow::SetUiLanguage()
//{
//    ui->btnGenerate->setText(tr("Generate"));
//       ui->btnSelect
//        ui->cmbUnit
//        ui->doubleSpinBoxCount
//         ui->doubleSpinBoxSize
//         ui->lblFileCountTip
//        ui->lblFilePath
//        ui->lblFileSizeTip
//        ui->lineEditFilePath
//         ui->textEditLog
//}

/*!
 * \brief MainWindow::ShowHelpInfo
 */
void MainWindow::ShowHelpInfo()
{
    ui->textEditLog->append(tr("Web: www.moetech.cn Ver:0.1.0.0\r\n"));

    ui->textEditLog->append(tr("User Manual："));
    ui->textEditLog->append(tr("1.Select empty file storage directory."));
    ui->textEditLog->append(tr("2.Set empty file length and number of files."));
    ui->textEditLog->append(tr("3.Click Generate buttton."));

    ui->textEditLog->append(tr("\r\nNote:"));
    ui->textEditLog->append(tr("1.Not support set file name."));
    ui->textEditLog->append(tr("2.We recommend that you use NTFS, otherwise slows the speed down."));
}

/*!
 * \brief MainWindow::GetUuidFileName
 * \param saveDir Empty files storage directory
 * \param fileLen Empty file len
 * \param fileLenUnit Empty file len's unit
 * \return
 */
QString MainWindow::GetUuidFileName(QString saveDir,double fileLen,QString fileLenUnit)
{
    QString uuidStr = QUuid::createUuid().toString();
    QString tempFilePath = QString("%0/EmptyFile-%1%2-%3.dat").arg(saveDir,QString::number(fileLen,10,3),fileLenUnit,uuidStr.mid(1,uuidStr.length()-2));
    return tempFilePath;
}

/*!
 * \brief Select empty file storage directory.
 */
void MainWindow::on_btnSelect_clicked()
{
    QString savePath =  QFileDialog::getExistingDirectory(this,tr("Select empty file storage directory."));
    if(!savePath.isNull() && savePath.length() > 0)
    {
        ui->lineEditFilePath->setText(savePath);
    }
    else
    {
        QMessageBox::information(this,tr("Info"),tr("Please select the correct storage directory."));
    }
}

/*!
 * \brief MainWindow::CreateEmptyFile
 * \param fileLenByte Empty file length, byte.
 * \param fullFileName Empty file full name.
 * \return Created result.
 */
bool MainWindow::CreateEmptyFile(double fileLenByte,QString fullFileName)
{
    HANDLE hFile,hMapFile;
    hFile = CreateFileW(fullFileName.toStdWString().c_str(),GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);

    if(hFile != nullptr)
    {
        unsigned long dwHigh = 0;
        unsigned long dwLow = 0;
        unsigned long sizeLimit = 4294967295;

        if(fileLenByte < sizeLimit)
        {
            dwHigh = 0;
            dwLow = static_cast<unsigned long>(fileLenByte);
        }
        else
        {
            double fourGBByte = 4.0 * 1024 * 1024 * 1024;
            dwHigh =static_cast<uint>(fileLenByte / fourGBByte);
            dwLow = static_cast<unsigned long>(fileLenByte - (dwHigh * fourGBByte));
        }


        hMapFile = CreateFileMapping(hFile,nullptr,PAGE_READWRITE,dwHigh,dwLow,nullptr);
        if(hMapFile == nullptr)
        {
            CloseHandle(hFile);
            return false;
        }
        else
        {
            CloseHandle(hMapFile);
            CloseHandle(hFile);
            return true;
        }
    }
    else
    {
        return false;
    }
}

/*!
 * \brief MainWindow::GetWindowsDiskFreeSpace
 * \param disk Like C:, D:...
 * \return
 */
quint64 MainWindow::GetWindowsDiskFreeSpace(QString disk)
{
    ULARGE_INTEGER freeDiskSpaceAvailable,totalDiskSpace,totalFreeDiskSpace;
    if(!GetDiskFreeSpaceExW(disk.toStdWString().c_str(),&freeDiskSpaceAvailable,&totalDiskSpace,&totalFreeDiskSpace))
    {
        return 0;
    }
    else
    {
        return totalFreeDiskSpace.QuadPart;
    }
}

/*!
 * \brief Generate files.
 */
void MainWindow::on_btnGenerate_clicked()
{
    //Check settings.
    if(ui->doubleSpinBoxSize->value() == 0.0)
    {
        QMessageBox::information(this,tr("Info"),tr("Please enter the correct file size."));
        return;
    }

    if(ui->doubleSpinBoxCount->value() == 0.0)
    {
        QMessageBox::information(this,tr("Info"),tr("Please enter the correct number of files."));
        return;
    }

    if(ui->lineEditFilePath->text().length() == 0)
    {
        QMessageBox::information(this,tr("Info"),tr("Please select the correct storage directory."));
        return;
    }

    QDir *saveDir = new QDir(ui->lineEditFilePath->text());
    if(!saveDir->exists())
    {
        QMessageBox::information(this,tr("Info"),tr("Directory does not exist, please select again."));
        return ;
    }

    QString disk = saveDir->path().mid(0,2);
    int fileCount = static_cast<int>(ui->doubleSpinBoxCount->value());
    double fileLenUnit = qPow(1024,ui->cmbUnit->currentIndex()); //0:Byte 1:KB 2:MB 3:GB 4:TB
    double fileLenInput =  QString::number( ui->doubleSpinBoxSize->value(),10,3).toDouble();
    double fileLenByte = fileLenInput * fileLenUnit;//Byte
    quint64 diskFreeSpace = GetWindowsDiskFreeSpace(disk);



    if(fileLenByte * fileCount > diskFreeSpace)
    {
        QMessageBox::information(this,tr("Info"),tr("Out of disk free space, please modify settings."));
        return ;
    }

    ui->textEditLog->clear();
    ui->textEditLog->append(tr("Starting..."));

    int faildCount = 0;

    for(int i = 0;i < fileCount; i++)
    {
        QString emptyFileFullName = GetUuidFileName(saveDir->path(),fileLenInput,ui->cmbUnit->currentText());
         bool createResult = CreateEmptyFile(fileLenByte,emptyFileFullName);
        if(!createResult)
            ++faildCount;

        ui->textEditLog->append(QString(tr("Empty file generated %0, %1 file(s) remaining to be processed")).arg(createResult ? tr("successed") : tr("faild"),QString::number(fileCount - i - 1)));
    }

    ui->textEditLog->append(QString(tr("Finished, %0 files faild.")).arg(QString::number(faildCount)));
}
