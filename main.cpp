#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QApplication>
#include "mainWindow.h"

#include "trafficSimulation.h"

/*int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/MapsQt/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}*/

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
    //TrafficSimulation sim;

    //sim.run();

    browseMap(argc, argv);
}
