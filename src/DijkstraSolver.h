#ifndef DIJKSTRASOLVER_H
#define DIJKSTRASOLVER_H

#include <QMap>
#include <QList>
#include <climits>
#include "Graph.h"

class DijkstraSolver {
public:
    // Состояния вершин
    enum VertexState {
        VS_WHITE,   // не достигнута, dist = inf
        VS_ORANGE,  // в очереди pq, имеет временное расстояние
        VS_YELLOW,  // обрабатывается на текущем шаге
        VS_GREEN    // финализирована, расстояние найдено
    };

    // Состояния рёбер
    enum EdgeState {
        ES_WHITE,   // ни разу не рассматривалось
        ES_ORANGE,  // в очереди на рассмотрение
        ES_YELLOW,  // рассматривается на текущем шаге (RELAXING)
        ES_GREEN,   // входит в дерево кратчайших путей
        ES_RED      // рассмотрено, отвергнуто навсегда
    };

    DijkstraSolver(Graph* graph);
    void setStartVertex(Vertex* v);
    bool step(); // выполнить один шаг алгоритма, возвращает false, если завершён
    void reset();
    
    //геттеры
    Vertex* getCurrentVertex() const;
    Vertex* getStartVertex() const;
    QMap<Vertex*, int> getDistances() const;
    bool isFinished() const;
    QList<Vertex*> getUnvisited() const;

    VertexState getVertexState(Vertex* v) const;
    EdgeState getEdgeState(Edge* e) const;

    //для тестов
    QList<Vertex*> getShortestPathTo(Vertex* target) const;//для тестов

private:
    Graph* m_graph;
    Vertex* m_startVertex;
    QMap<Vertex*, int> m_distances;
    QMap<Vertex*, Vertex*> m_predecessors;
    QList<Vertex*> m_unvisited; // вершины, ещё не обработанные (белые + оранжевые)
    Vertex* m_currentVertex; // текущая обрабатываемая вершина (жёлтая)
    Vertex* m_previousYellowVertex; // предыдущая жёлтая вершина (для правил визуализации)
    bool m_finished;
    // Хранилища состояний
    QMap<Vertex*, VertexState> m_vertexState;
    QMap<Edge*, EdgeState> m_edgeState;

    // Состояния до начала текущей итерации (для правил визуализации)
    QMap<Vertex*, VertexState> m_oldVertexState;
    QMap<Edge*, EdgeState> m_oldEdgeState;

    // Фаза шага: true = релаксация и правило 1, false = выбор следующей жёлтой вершины (правила 2.1-4)
    bool relacsationPhase;

    // сеттеры
    void setVertexState(Vertex* v, VertexState state);
    void setEdgeState(Edge* e, EdgeState state);

    //вспомогательные функции
    bool relacsation();
    bool nextYellow();
};

#endif // DIJKSTRASOLVER_H