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
    app.setDesktopFileName("phoca");

    MainWindow window;

    window.show();

    return app.exec();
}