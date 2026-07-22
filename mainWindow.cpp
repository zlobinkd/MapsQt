#include "mainwindow.h"
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QRandomGenerator>
#include <QGraphicsView>
#include <thread>

#include "dynamicMapGraphicsItem.h"
#include "trafficSimulation.h"

MainWindow::MainWindow() {
    QGraphicsScene* scene = new QGraphicsScene(this);
    QGraphicsView* view = new QGraphicsView(scene);

    // IMPORTANT: Remove or disable view transformations
    view->setDragMode(QGraphicsView::NoDrag);  // Changed from ScrollHandDrag
    view->setRenderHint(QPainter::Antialiasing);

    // Optional: Set a large scene rect to allow panning
    scene->setSceneRect(0, 0, MapData::instance().imageSizeX(), MapData::instance().imageSizeY());

    auto* lineItem = new MapGraphicsItem();
    scene->addItem(lineItem);

    auto* pointItem = new DynamicMapGraphicsItem(lineItem);
    scene->addItem(pointItem);

    std::thread simThread([pointItem]() {
        TrafficSimulation sim(pointItem);
        sim.run();  // This will run in the background
    });
    simThread.detach();

    setCentralWidget(view);
}

void MainWindow::onNodeClicked(id_t index) {
    qDebug() << "Node" << index << "clicked";
}
