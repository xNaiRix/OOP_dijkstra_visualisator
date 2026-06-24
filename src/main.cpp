#include "MainWindow.h"
#include <QApplication>
#include "Graph.h"
#include "DijkstraSolver.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Graph* graph = new Graph();
    DijkstraSolver* solver = new DijkstraSolver(graph);


    MainWindow window;
    window.setGraphAndSolver(graph, solver);
    window.show();
    
    int result = app.exec();

    delete solver;
    delete graph;

    return result;
}