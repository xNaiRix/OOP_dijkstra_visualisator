#include "src/Graph.h"
#include "src/DijkstraSolver.h"
#include <QCoreApplication>
#include <QDebug>
#include <climits>
#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << "Hello, World!" << std::endl;
    QCoreApplication app(argc, argv);

    Graph* graph = new Graph();
    
    // Создаём вершины через Graph (addVertex возвращает Vertex*)
    Vertex* v0 = graph->addVertex(QPointF(0, 0));
    Vertex* v1 = graph->addVertex(QPointF(100, 0));
    Vertex* v2 = graph->addVertex(QPointF(200, 0));
    Vertex* v3 = graph->addVertex(QPointF(0, 100));
    Vertex* v4 = graph->addVertex(QPointF(100, 100));
    
    // Создаём рёбра с весами
    graph->addEdge(v0, v1, 10);
    graph->addEdge(v0, v3, 5);
    graph->addEdge(v1, v2, 1);
    graph->addEdge(v1, v4, 2);
    graph->addEdge(v2, v4, 4);
    graph->addEdge(v3, v4, 2);
    graph->addEdge(v3, v1, 3);
    
    DijkstraSolver solver(graph);
    solver.setStartVertex(v0);
    
    qDebug() << "=== Starting Dijkstra algorithm ===";
    int stepCount = 0;
    while (solver.step()) {
        qDebug() << "--- Step" << ++stepCount << "---";
    }
    
    qDebug() << "=== Algorithm finished ===";
    qDebug() << "Distances from start vertex" << v0 << ":";
    QMap<Vertex*, int> distances = solver.getDistances();
    for (Vertex* v : graph->getVertices()) {
        qDebug() << "Vertex" << v << "distance:" << distances.value(v, INT_MAX);
    }
    
    qDebug() << "Shortest path to v2:";
    QList<Vertex*> path = solver.getShortestPathTo(v2);
    for (Vertex* v : path) {
        qDebug() << " ->" << v;
    }
    
    // Проверка корректности расстояний
    // Ожидаемые расстояния:
    // v0: 0
    // v1: 8 (через v3)
    // v2: 9 (v0->v3->v1->v2)
    // v3: 5
    // v4: 7 (v0->v3->v4)
    bool correct = true;
    if (distances.value(v0, INT_MAX) != 0) {
        qDebug() << "ERROR: distance to v0 should be 0, got" << distances.value(v0, INT_MAX);
        correct = false;
    }
    if (distances.value(v1, INT_MAX) != 8) {
        qDebug() << "ERROR: distance to v1 should be 8, got" << distances.value(v1, INT_MAX);
        correct = false;
    }
    if (distances.value(v2, INT_MAX) != 9) {
        qDebug() << "ERROR: distance to v2 should be 9, got" << distances.value(v2, INT_MAX);
        correct = false;
    }
    if (distances.value(v3, INT_MAX) != 5) {
        qDebug() << "ERROR: distance to v3 should be 5, got" << distances.value(v3, INT_MAX);
        correct = false;
    }
    if (distances.value(v4, INT_MAX) != 7) {
        qDebug() << "ERROR: distance to v4 should be 7, got" << distances.value(v4, INT_MAX);
        correct = false;
    }
    
    if (correct) {
        qDebug() << "All distances are correct!";
    } else {
        qDebug() << "Some distances are incorrect.";
    }
    
    delete graph;
    return correct ? 0 : 1;
}