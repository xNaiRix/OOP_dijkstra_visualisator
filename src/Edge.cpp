#include "Edge.h"

Edge::Edge(Vertex* from, Vertex* to, int weight)
    : m_from(from), m_to(to), m_weight(weight),
      m_labelPos(0, 0), m_customLabelPos(false)
{
}

Vertex* Edge::getFrom() const
{
    return m_from;
}

Vertex* Edge::getTo() const
{
    return m_to;
}

int Edge::getWeight() const
{
    return m_weight;
}

void Edge::setWeight(int weight)
{
    m_weight = weight;
}

QPointF Edge::getLabelPos() const
{
    return m_labelPos;
}

void Edge::setLabelPos(const QPointF& pos)
{
    m_labelPos = pos;
    m_customLabelPos = true;
}

bool Edge::hasCustomLabelPos() const
{
    return m_customLabelPos;
}

void Edge::resetLabelPos()
{
    m_customLabelPos = false;
    m_labelPos = QPointF(0, 0);
}