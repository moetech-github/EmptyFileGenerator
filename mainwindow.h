#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnSelect_clicked();
    void on_btnGenerate_clicked();

private:
    Ui::MainWindow *ui;
    void ShowHelpInfo();
    bool CreateEmptyFile(double fileLenByte,QString fullFileName);
    QString GetUuidFileName(QString saveDir,double fileLen,QString fileLenUint);
    quint64 GetWindowsDiskFreeSpace(QString disk);
//    void SetUiLanguage();
};

#endif // MAINWINDOW_H
