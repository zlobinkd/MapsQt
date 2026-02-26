#pragma once

#include <QMainWindow>
#include "mapGraphicsItem.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QStatusBar;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();

public slots:
    void onNodeClicked(id_t index);
};
