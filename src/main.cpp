/*
    SPDX-FileCopyrightText: 2026 Máximo Pardo
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QLocale>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    #ifdef Q_OS_LINUX
    QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP").toLower();
    if (!desktop.contains("kde")) {
        app.setStyle(QStyleFactory::create("Fusion"));
    }
    #endif



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