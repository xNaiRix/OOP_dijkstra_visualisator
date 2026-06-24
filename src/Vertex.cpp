#include "Vertex.h"

Vertex::Vertex(int id, const QPointF& position)
    : m_id(id), m_position(position), m_label(QString::number(id)){}

int Vertex::getId() const {return m_id;}

QPointF Vertex::getPosition() const {return m_position;}

void Vertex::setPosition(const QPointF& pos){m_position = pos;}

QString Vertex::getLabel() const {return m_label;}

void Vertex::setLabel(const QString& label){m_label = label;}
