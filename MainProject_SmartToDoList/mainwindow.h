#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QVector>
#include <QDateTimeEdit>
#include "task.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showPageCatat();
    void showPageLihat();
    void showPageHistory();
    void onTambahTask();
    void onNotifTick();
    void refreshLihat();
    void refreshHistory();

private:
    // Layout utama
    QWidget        *centralWidget;
    QHBoxLayout    *rootLayout;

    // Sidebar
    QFrame         *sidebar;
    QPushButton    *btnCatat;
    QPushButton    *btnLihat;
    QPushButton    *btnHistory;
    QLabel         *lblClock;
    QLabel         *lblStats;

    // Pages
    QStackedWidget *pages;
    QWidget        *pageCatat;
    QWidget        *pageLihat;
    QWidget        *pageHistory;

    // Catat page widgets
    QWidget        *buildPageCatat();
    class QLineEdit  *inputCatatan;
    QDateTimeEdit    *inputWaktu;
    class QComboBox  *inputPriority;

    // Lihat page
    QWidget        *buildPageLihat();
    QVBoxLayout    *lihatLayout;
    QWidget        *lihatContainer;

    // History page
    QWidget        *buildPageHistory();
    QVBoxLayout    *historyLayout;
    QWidget        *historyContainer;

    // Tray & Timer
    QSystemTrayIcon *trayIcon;
    QTimer          *notifTimer;
    QTimer          *clockTimer;

    // Data
    QVector<Task>   tasks;
    void            loadData();
    void            saveData();
    void            saveHistoryTelat(const Task &t);
    void            updateStats();
    void            setSidebarActive(QPushButton *btn);

    QString         appStyleSheet();
};

#endif
