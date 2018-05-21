#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
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
        item.no = f.fileName();

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

    ui->twLocal->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twLocal->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->twCloud->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twCloud->setSelectionMode(QAbstractItemView::SingleSelection);

    QString pathLocal("E:/lzf/Sango7/Save");
    ui->leLocal->setText(pathLocal);

    QString pathCloud("C:/Users/hpjing/Dropbox/GameSave/Sango7");
    ui->leCloud->setText(pathCloud);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh_cloud()
{
    listCloud = loadCloudFolder(ui->leCloud->text());

    ui->twCloud->setRowCount(listCloud.size());
    for (int i = 0; i < listCloud.size(); i++)
    {
        auto item = listCloud[i];
        ui->twCloud->setItem(i, 0, new QTableWidgetItem(item.no));
        ui->twCloud->setItem(i, 1, new QTableWidgetItem(item.time.toString("yyyy-MM-dd HH:mm:ss")));
        ui->twCloud->setItem(i, 2, new QTableWidgetItem(item.comment));
    }
}

void MainWindow::refresh_local()
{
    listLocal = loadLocalFolder(ui->leLocal->text());

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
    refresh_cloud();
}

void MainWindow::on_leLocal_textChanged(const QString &)
{
    refresh_local();
}

void MainWindow::on_btPush_clicked()
{
    qDebug() << "push_clicked";
    if (ui->twLocal->selectedItems().empty())
    {
        return;
    }

    auto sel = ui->twLocal->selectedItems().at(0);
    SAVE_ITEM item = listLocal.at(sel->row());

    QString pathCloud = ui->leCloud->text();
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
    qDebug() << "pull_clicked";
    if (ui->twCloud->selectedItems().empty())
    {
        return;
    }

    auto sel = ui->twCloud->selectedItems().at(0);
    SAVE_ITEM item = listCloud.at(sel->row());

    QString pathLocal = ui->leLocal->text();
    QString localFilePath;
    localFilePath.sprintf("%s/%s", pathLocal.toLatin1().data(), item.baseName.toLatin1().data());

    if (QFile::exists(localFilePath))
    {
        QFile::remove(localFilePath);
    }
    QFile::copy(item.fileName, localFilePath);

    refresh_local();
}
