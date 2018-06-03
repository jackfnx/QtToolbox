#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QDateTime>
#include <QSystemTrayIcon>
#include <QCloseEvent>

namespace Ui {
class MainWindow;
}

typedef struct __tag_SAVE_ITEM
{
    QString fileName;
    QString baseName;
    QString no;
    QDateTime time;
    QString comment;
} SAVE_ITEM;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_leCloud_textChanged(const QString &);

    void on_leLocal_textChanged(const QString &);

    void on_btPull_clicked();

    void on_btPush_clicked();

    void on_twLocal_itemSelectionChanged();

    void on_twCloud_itemSelectionChanged();

    void onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason);
    void closeEvent(QCloseEvent *event);

private:
    void refresh_cloud();
    void refresh_local();

    Ui::MainWindow *ui;
    QSystemTrayIcon systray;
    QList<SAVE_ITEM> listLocal;
    QList<SAVE_ITEM> listCloud;
};

#endif // MAINWINDOW_H
