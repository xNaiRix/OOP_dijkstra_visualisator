#ifndef GRAPH_H
#define GRAPH_H

#include <QList>
#include <QRectF>
#include "Vertex.h"
#include "Edge.h"

class Graph {
public:
    Graph();
    ~Graph();

    Vertex* addVertex(const QPointF& pos);
    Edge* addEdge(Vertex* from, Vertex* to, int weight);
    void removeVertex(Vertex* v);
    void removeEdge(Edge* e);
    QList<Vertex*> getVertices() const;
    QList<Edge*> getEdges() const;
    QList<Edge*> getAdjacentEdges(Vertex* v) const;
    void clear();
    void generateRandomGraph(int vertexCount, int edgeCount, const QRectF& area);

private:
    QList<Vertex*> m_vertices;
    QList<Edge*> m_edges;
    int m_nextVertexId;
};

#endif // GRAPH_H