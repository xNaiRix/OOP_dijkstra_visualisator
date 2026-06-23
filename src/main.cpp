#include "MainWindow.h"
#include <QApplication>
#include "Graph.h"
#include "DijkstraSolver.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Создаём граф и решатель
    Graph* graph = new Graph();
    DijkstraSolver* solver = new DijkstraSolver(graph);

    // Создаём главное окно
    MainWindow window;
    // Передаём граф и решатель в canvas
    window.setGraphAndSolver(graph, solver);

    window.show();
    int result = app.exec();

    // Очистка памяти
    delete solver;
    delete graph;

    return result;
}