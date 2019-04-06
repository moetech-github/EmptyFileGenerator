#include "mainwindow.h"
#include "QTranslator"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator *translator = new QTranslator();
    translator->load("./translations/" + locale);
    a.installTranslator(translator);

    MainWindow w;
    w.show();

    return a.exec();
}
