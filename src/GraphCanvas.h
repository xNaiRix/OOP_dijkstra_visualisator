#ifndef GRAPHCANVAS_H
#define GRAPHCANVAS_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QMenu>
#include "Graph.h"
#include "DijkstraSolver.h"

class GraphCanvas : public QWidget {
    Q_OBJECT
public:
    enum Mode {
        Normal,
        AddVertex,
        AddEdge,
        SetStart,
        DragVertex,
        DragWeight
    };

    explicit GraphCanvas(QWidget *parent = nullptr);
    void setGraph(Graph* graph);
    void setSolver(DijkstraSolver* solver);
    void setStartVertex(Vertex* v);
    void startDijkstra();
    bool stepDijkstra();
    void resetVisualization();
    void setMode(Mode mode);
    Vertex* selectedVertex() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Graph* m_graph;
    DijkstraSolver* m_solver;
    Vertex* m_selectedVertex;
    Vertex* m_draggedVertex;
    Vertex* m_startVertex;
    QPointF m_dragOffset;
    Mode m_mode;
    Vertex* m_edgeStartVertex;
    QList<Vertex*> m_selectedVertices; // для Shift+выделения
    Edge* m_draggedEdge;               // ребро, метка которого перетаскивается
    QPointF m_dragLabelOffset;         // смещение мыши относительно центра метки
    Edge* m_selectedEdge;              // выделенное ребро (для удаления по del)

    void drawVertex(QPainter& painter, Vertex* v);
    void drawEdge(QPainter& painter, Edge* e);
    Vertex* vertexAt(const QPointF& pos) const;
    Edge* edgeAt(const QPointF& pos) const;
    Edge* edgeAtWeight(const QPointF& pos) const; // поиск ребра по клику на метке веса
    void addVertexAt(const QPointF& pos);
    void addEdgeBetween(Vertex* from, Vertex* to, int weight);
    void deleteSelected();
    QColor getEdgeNormalColor() const;
    QColor getContrastTextColor() const;
};

#endif // GRAPHCANVAS_H