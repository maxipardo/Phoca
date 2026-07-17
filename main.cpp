#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    // 1. Instanciamos el motor principal de la aplicación Qt
    QApplication app(argc, argv);

    // 2. Creamos un objeto de tu ventana principal
    MainWindow window;

    // 3. Le decimos que se muestre en pantalla
    window.show();

    // 4. Arrancamos el bucle de eventos
    return app.exec();
}