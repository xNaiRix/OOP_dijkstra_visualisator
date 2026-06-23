#ifndef VERTEX_H
#define VERTEX_H

#include <QPointF>
#include <QString>

class Vertex {
public:
    Vertex(int id, const QPointF& position);
    int getId() const;
    QPointF getPosition() const;
    void setPosition(const QPointF& pos);
    QString getLabel() const;
    void setLabel(const QString& label);

private:
    int m_id;
    QPointF m_position;
    QString m_label;
};

#endif // VERTEX_H