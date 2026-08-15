#include <QApplication>
#include <QIcon>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/phoca.png"));
    app.setDesktopFileName("phoca.desktop");

    MainWindow window;

    window.show();

    return app.exec();
}