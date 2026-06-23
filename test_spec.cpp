#include "src/Graph.h"
#include "src/DijkstraSolver.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    Graph* graph = new Graph();
    
    // Создаём вершины через graph (id назначается автоматически)
    Vertex* v0 = graph->addVertex(QPointF(0, 0));
    Vertex* v1 = graph->addVertex(QPointF(100, 0));
    Vertex* v2 = graph->addVertex(QPointF(200, 0));
    Vertex* v3 = graph->addVertex(QPointF(0, 100));
    Vertex* v4 = graph->addVertex(QPointF(100, 100));
    
    // Рёбра согласно спецификации (неориентированные)
    graph->addEdge(v0, v1, 11);
    graph->addEdge(v0, v2, 3);
    graph->addEdge(v1, v2, 6);
    graph->addEdge(v2, v3, 10);
    graph->addEdge(v2, v4, 45);
    graph->addEdge(v3, v4, 4);
    
    DijkstraSolver solver(graph);
    solver.setStartVertex(v3); // стартовая вершина 3
    
    qDebug() << "=== Starting Dijkstra algorithm (specification graph) ===";
    qDebug() << "Initial state:";
    for (Vertex* v : graph->getVertices()) {
        qDebug() << "Vertex" << v->getId() << "state:" << solver.getVertexState(v);
    }
    for (Edge* e : graph->getEdges()) {
        qDebug() << "Edge" << e->getFrom()->getId() << "-" << e->getTo()->getId() << "state:" << solver.getEdgeState(e);
    }
    
    int iteration = 0;
    while (!solver.isFinished()) {
        qDebug() << "\n--- Iteration" << ++iteration << "---";
        bool hasStep = solver.step();
        if (!hasStep) {
            qDebug() << "Step returned false (algorithm finished)";
            break;
        }
        // Вывод текущих состояний
        qDebug() << "Vertex states:";
        for (Vertex* v : graph->getVertices()) {
            DijkstraSolver::VertexState vs = solver.getVertexState(v);
            const char* stateStr = "?";
            switch (vs) {
                case DijkstraSolver::VS_WHITE: stateStr = "WHITE"; break;
                case DijkstraSolver::VS_ORANGE: stateStr = "ORANGE"; break;
                case DijkstraSolver::VS_YELLOW: stateStr = "YELLOW"; break;
                case DijkstraSolver::VS_GREEN: stateStr = "GREEN"; break;
            }
            qDebug() << "  " << v->getId() << ":" << stateStr;
        }
        qDebug() << "Edge states:";
        for (Edge* e : graph->getEdges()) {
            DijkstraSolver::EdgeState es = solver.getEdgeState(e);
            const char* stateStr = "?";
            switch (es) {
                case DijkstraSolver::ES_WHITE: stateStr = "WHITE"; break;
                case DijkstraSolver::ES_ORANGE: stateStr = "ORANGE"; break;
                case DijkstraSolver::ES_YELLOW: stateStr = "YELLOW"; break;
                case DijkstraSolver::ES_GREEN: stateStr = "GREEN"; break;
                case DijkstraSolver::ES_RED: stateStr = "RED"; break;
            }
            qDebug() << "  " << e->getFrom()->getId() << "-" << e->getTo()->getId() << ":" << stateStr;
        }
    }
    
    qDebug() << "\n=== Algorithm finished ===";
    qDebug() << "Distances from start vertex" << v3->getId() << ":";
    QMap<Vertex*, int> distances = solver.getDistances();
    for (Vertex* v : graph->getVertices()) {
        qDebug() << "Vertex" << v->getId() << "distance:" << distances.value(v, INT_MAX);
    }
    
    delete graph;
    return 0;
}