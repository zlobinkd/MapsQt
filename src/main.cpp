#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QApplication>
#include "src/gui/mainWindow.h"
#include "src/utils/benchmark.h"
#include "src/map/map.h"

static int browseMap(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("Road Map Viewer");
    window.resize(800, 600);
    window.show();

    return app.exec();
}

static int benchmark() {
	auto map = executeAndShowElapsedTime<Map>([]() { return Map{}; }, "Map creation");
	executeAndShowElapsedTime([&map]() { map.shortestPath(359797, 645321); }, "Longest path calculation");
	return 0;
}

int main(int argc, char *argv[])
{
    browseMap(argc, argv);
    //benchmark();
}
