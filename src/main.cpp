#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QApplication>
#include "src/gui/mainWindow.h"

static int browseMap(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("Road Map Viewer");
    window.resize(800, 600);
    window.show();

    return app.exec();
}

int main(int argc, char *argv[])
{
    browseMap(argc, argv);
}
