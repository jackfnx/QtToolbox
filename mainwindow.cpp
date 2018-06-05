#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>

QList<SAVE_ITEM> loadLocalFolder(QString path)
{
    QList<SAVE_ITEM> list;
    QDir dir(path);
    foreach (QFileInfo f, dir.entryInfoList())
    {
        if (f.fileName() == "." || f.fileName() == "..")
            continue;

        SAVE_ITEM item;
        item.fileName = f.absoluteFilePath();
        item.baseName = f.fileName();

        int hyphen = item.fileName.lastIndexOf("-");
        int dot = item.fileName.lastIndexOf(".");
        if (f.suffix() != "sav" || hyphen < 0)
        {
            continue;
        }
        item.no = item.fileName.mid(hyphen + 1, dot - hyphen - 1);
        item.time = f.lastModified();

        list << item;
    }
    return list;
}

QList<SAVE_ITEM> loadCloudFolder(QString path)
{
    QList<SAVE_ITEM> list;
    QDir dir(path);
    foreach (QFileInfo f, dir.entryInfoList())
    {
        if (f.fileName() == "." || f.fileName() == "..")
            continue;

        SAVE_ITEM item;
        item.baseName = f.fileName();
        item.fileName.sprintf("%s/%s", f.absoluteFilePath().toLatin1().data(), f.fileName().toLatin1().data());

        int hyphen = item.fileName.lastIndexOf("-");
        int dot = item.fileName.lastIndexOf(".");
        if (f.suffix() != "sav" || hyphen < 0)
        {
            continue;
        }
        item.no = item.fileName.mid(hyphen + 1, dot - hyphen - 1);

        QString metaPath;
        metaPath.sprintf("%s/meta.json", f.absoluteFilePath().toLatin1().data());

        QFile metaIn(metaPath);
        if (metaIn.exists() && metaIn.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&metaIn);
            QString json = in.readAll();
            metaIn.close();

            QJsonDocument jsonIn = QJsonDocument::fromJson(json.toUtf8());
            item.time = QDateTime::fromTime_t(jsonIn.object().value("timestamp").toInt());
            item.comment = jsonIn.object().value("comment").toString();
        }
        else
        {
            continue;
        }

        list << item;
    }
    return list;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason)));

    ui->twLocal->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twLocal->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->twCloud->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twCloud->setSelectionMode(QAbstractItemView::SingleSelection);

    QString configFolderPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString configFilePath = configFolderPath + "/toolbox.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);

    QString pathLocal(settings.value("local", "E:/lzf/Sango7").toString());
    ui->leLocal->setText(pathLocal);

    QString pathCloud(settings.value("cloud", "C:/Users/hpjing/Dropbox").toString());
    ui->leCloud->setText(pathCloud);

    systray.setToolTip("sixue的工具箱");
    systray.setIcon(QIcon(":/png/toolbox.png"));

    QMenu *menu = new QMenu;
    QAction *quitAction = new QAction("quit", this);
    menu->addAction(quitAction);
    systray.setContextMenu(menu);
    systray.show();

    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitActionSlot()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::cloud_path() {
    return ui->leCloud->text() + "/GameSave/Sango7";
}

QString MainWindow::local_path() {
    return ui->leLocal->text() + "/Save";
}

void MainWindow::refresh_cloud()
{
    listCloud = loadCloudFolder(cloud_path());

    ui->twCloud->setRowCount(listCloud.size());
    for (int i = 0; i < listCloud.size(); i++)
    {
        auto item = listCloud[i];
        ui->twCloud->setItem(i, 0, new QTableWidgetItem(item.no));
        ui->twCloud->setItem(i, 1, new QTableWidgetItem(item.baseName));
        ui->twCloud->setItem(i, 2, new QTableWidgetItem(item.time.toString("yyyy-MM-dd HH:mm:ss")));
    }
}

void MainWindow::refresh_local()
{
    listLocal = loadLocalFolder(local_path());

    ui->twLocal->setRowCount(listLocal.size());
    for (int i = 0; i < listLocal.size(); i++)
    {
        auto item = listLocal[i];
        ui->twLocal->setItem(i, 0, new QTableWidgetItem(item.no));
        ui->twLocal->setItem(i, 1, new QTableWidgetItem(item.baseName));
        ui->twLocal->setItem(i, 2, new QTableWidgetItem(item.time.toString("yyyy-MM-dd HH:mm:ss")));
    }
}

void MainWindow::on_leCloud_textChanged(const QString &)
{
    QString configFolderPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString configFilePath = configFolderPath + "/toolbox.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    settings.setValue("cloud", ui->leCloud->text());
    settings.sync();
    refresh_cloud();
}

void MainWindow::on_leLocal_textChanged(const QString &)
{
    QString configFolderPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString configFilePath = configFolderPath + "/toolbox.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    settings.setValue("local", ui->leLocal->text());
    settings.sync();
    refresh_local();
}

void MainWindow::on_btPush_clicked()
{
    if (ui->twLocal->selectedItems().empty())
    {
        return;
    }

    auto sel = ui->twLocal->selectedItems().at(0);
    SAVE_ITEM item = listLocal.at(sel->row());

    QString pathCloud = cloud_path();
    QString imageDirPath;
    imageDirPath.sprintf("%s/%s", pathCloud.toLatin1().data(), item.baseName.toLatin1().data());
    QString imageFilePath;
    imageFilePath.sprintf("%s/%s/%s", pathCloud.toLatin1().data(), item.baseName.toLatin1().data(), item.baseName.toLatin1().data());
    QString metaPath;
    metaPath.sprintf("%s/%s/meta.json", pathCloud.toLatin1().data(), item.baseName.toLatin1().data());

    QDir imageDir(imageDirPath);
    if (!imageDir.exists())
    {
        imageDir.mkpath(imageDirPath);
    }

    qint64 timestamp = item.time.toTime_t();
    QString comment = "";

    QFile metaIn(metaPath);
    if (metaIn.exists() && metaIn.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString json;
        QTextStream in(&metaIn);
        while (!metaIn.atEnd())
        {
            json += in.readLine();
        }
        metaIn.close();
        QJsonDocument jsonIn = QJsonDocument::fromJson(json.toUtf8());
        comment = jsonIn.object().value("comment").toString();
    }

    QFile metaOut(metaPath);
    if (metaOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QJsonDocument jsonOut;
        QJsonObject jsonObj;
        jsonObj.insert("timestamp", timestamp);
        jsonObj.insert("comment", comment);
        jsonOut.setObject(jsonObj);

        QTextStream out(&metaOut);
        out << jsonOut.toJson() << "\n";
        metaOut.close();
    }

    if (QFile::exists(imageFilePath))
    {
        QFile::remove(imageFilePath);
    }
    QFile::copy(item.fileName, imageFilePath);

    refresh_cloud();
}

void MainWindow::on_btPull_clicked()
{
    if (ui->twCloud->selectedItems().empty())
    {
        return;
    }

    auto sel = ui->twCloud->selectedItems().at(0);
    SAVE_ITEM item = listCloud.at(sel->row());

    QString pathLocal = local_path();
    QString localFilePath;
    localFilePath.sprintf("%s/%s", pathLocal.toLatin1().data(), item.baseName.toLatin1().data());

    if (QFile::exists(localFilePath))
    {
        QFile::remove(localFilePath);
    }
    QFile::copy(item.fileName, localFilePath);

    refresh_local();
}

void MainWindow::on_twLocal_itemSelectionChanged()
{
    if (ui->twLocal->selectedItems().empty())
    {
        return;
    }

    auto selLocal0 = ui->twLocal->selectedItems().at(0);
    SAVE_ITEM item = listLocal.at(selLocal0->row());
    auto selClouds = ui->twCloud->selectedItems();
    for (int i = 0; i < selClouds.size(); i++)
    {
        if (listCloud.at(selClouds.at(i)->row()).no == item.no)
        {
            return;
        }
    }
    ui->twCloud->clearSelection();
    for (int i = 0; i < listCloud.size(); i++)
    {
        if (listCloud.at(i).no == item.no)
        {
            ui->twCloud->selectRow(i);
            return;
        }
    }
}

void MainWindow::on_twCloud_itemSelectionChanged()
{
    if (ui->twCloud->selectedItems().empty())
    {
        return;
    }

    auto selCloud0 = ui->twCloud->selectedItems().at(0);
    SAVE_ITEM item = listCloud.at(selCloud0->row());
    auto selLocals = ui->twLocal->selectedItems();
    for (int i = 0; i < selLocals.size(); i++)\
    {
        if (listLocal.at(selLocals.at(i)->row()).no == item.no)
        {
            return;
        }
    }
    ui->twCloud->clearSelection();
    for (int i = 0; i < listLocal.size(); i++)
    {
        if (listLocal.at(i).no == item.no)
        {
            ui->twLocal->selectRow(i);
            return;
        }
    }
}

void MainWindow::onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
    {
        //判断点击或双击时，恢复显示主窗口
        this->showNormal();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //重写closeEvent虚函数，实现关闭主窗口时退到托盘最小化
    this->hide();
    event->ignore();
}

void MainWindow::quitActionSlot()
{
    QApplication::exit(0);
}
