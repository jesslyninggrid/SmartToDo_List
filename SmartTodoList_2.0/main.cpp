#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("TodoList");
    app.setApplicationVersion("1.0");

    // Pastikan tray icon bisa tampil
    app.setQuitOnLastWindowClosed(false);

    MainWindow w;
    w.show();
    return app.exec();
}
