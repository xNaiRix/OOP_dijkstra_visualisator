#ifndef EDGE_H
#define EDGE_H

#include "Vertex.h"
#include <QPointF>

class Edge {
public:
    Edge(Vertex* from, Vertex* to, int weight);
    Vertex* getFrom() const;
    Vertex* getTo() const;
    int getWeight() const;
    void setWeight(int weight);

    QPointF getLabelPos() const;
    void setLabelPos(const QPointF& pos);
    bool hasCustomLabelPos() const;
    void resetLabelPos();

private:
    Vertex* m_from;
    Vertex* m_to;
    int m_weight;
    QPointF m_labelPos;
    bool m_customLabelPos;
};

#endif // EDGE_H