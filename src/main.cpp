#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QLocale>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator translator;
    
    if (translator.load(QLocale(), "phoca", "_", ":/i18n")) {
        app.installTranslator(&translator);
    }

    app.setWindowIcon(QIcon(":/phoca.png"));
    #ifndef DESKTOP_FILE_NAME
    #define DESKTOP_FILE_NAME "phoca"
    #endif
    app.setDesktopFileName(DESKTOP_FILE_NAME);

    MainWindow window;

    window.show();

    return app.exec();
}