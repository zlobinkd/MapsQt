#include "mainwindow.h"
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QRandomGenerator>
#include <QGraphicsView>

MainWindow::MainWindow() {
    QGraphicsScene* scene = new QGraphicsScene(this);
    QGraphicsView* view = new QGraphicsView(scene);

    // IMPORTANT: Remove or disable view transformations
    view->setDragMode(QGraphicsView::NoDrag);  // Changed from ScrollHandDrag
    view->setRenderHint(QPainter::Antialiasing);

    // Optional: Set a large scene rect to allow panning
    scene->setSceneRect(0, 0, GuiRepresentation::ImageSize, GuiRepresentation::ImageSize);

    auto* lineItem = new MapGraphicsItem();
    scene->addItem(lineItem);

    setCentralWidget(view);
}

void MainWindow::onNodeClicked(id_t index) {
    qDebug() << "Node" << index << "clicked";
}
