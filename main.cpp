#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


//    auto m_systemTray = new QSystemTrayIcon(this);
//    m_systemTray->setIcon(QIcon(":/SystemTray/Resources/ico.png"));
//    m_systemTray->setToolTip("SystemTray Program");

    return a.exec();
}
