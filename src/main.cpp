#include "mainwindow.h"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create your main window (but don't show it directly)
    MainWindow w;

    // Create graphics view and scene for rotation
    QGraphicsView *view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();
    view->setScene(scene);

    // Add the main window to the scene and rotate it 180 degrees
    QGraphicsProxyWidget *proxy = scene->addWidget(&w);
    proxy->setRotation(180);

    // Remove scroll bars for clean fullscreen
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFrameStyle(0);

    // SCALE TO FILL THE ENTIRE SCREEN
    view->scale(1.9, 1.6);  // Start with 1:1 scale
    view->setAlignment(Qt::AlignCenter);

    // Show the rotated view
    view->showFullScreen();


    return a.exec();
}
