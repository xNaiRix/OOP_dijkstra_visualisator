#include "Graph.h"
#include <QDebug>
#include <QRandomGenerator>

Graph::Graph() : m_nextVertexId(0) {}

Graph::~Graph() {
    clear();
}

Vertex* Graph::addVertex(const QPointF& pos) {
    Vertex* v = new Vertex(m_nextVertexId++, pos);
    m_vertices.append(v);
    qDebug() << "[Graph] Добавлена вершина id=" << v->getId() << "pos=" << pos;
    qDebug() << "  Всего вершин:" << m_vertices.size() << "рёбер:" << m_edges.size();
    return v;
}

Edge* Graph::addEdge(Vertex* from, Vertex* to, int weight) {
    // Проверка, что ребро не существует
    for (Edge* e : m_edges) {
        if ((e->getFrom() == from && e->getTo() == to) ||
            (e->getFrom() == to && e->getTo() == from)) {
            qDebug() << "[Graph] Попытка добавить существующее ребро" << from->getId() << "->" << to->getId() << "вес" << weight;
            return nullptr; // уже существует
        }
    }
    Edge* e = new Edge(from, to, weight);
    m_edges.append(e);
    qDebug() << "[Graph] Добавлено ребро" << from->getId() << "->" << to->getId() << "вес" << weight;
    qDebug() << "  Всего вершин:" << m_vertices.size() << "рёбер:" << m_edges.size();
    return e;
}

void Graph::removeVertex(Vertex* v) {
    if (!m_vertices.contains(v)) return;

    qDebug() << "[Graph] Удаление вершины id=" << v->getId();
    // Удалить все рёбра, инцидентные вершине
    QList<Edge*> edgesToRemove;
    for (Edge* e : m_edges) {
        if (e->getFrom() == v || e->getTo() == v) {
            edgesToRemove.append(e);
        }
    }
    qDebug() << "  Удалено инцидентных рёбер:" << edgesToRemove.size();
    for (Edge* e : edgesToRemove) {
        m_edges.removeOne(e);
        delete e;
    }

    m_vertices.removeOne(v);
    delete v;
    qDebug() << "  Всего вершин:" << m_vertices.size() << "рёбер:" << m_edges.size();
}

void Graph::removeEdge(Edge* e) {
    if (m_edges.removeOne(e)) {
        qDebug() << "[Graph] Удаление ребра" << e->getFrom()->getId() << "->" << e->getTo()->getId() << "вес" << e->getWeight();
        delete e;
        qDebug() << "  Всего вершин:" << m_vertices.size() << "рёбер:" << m_edges.size();
    } else {
        qDebug() << "[Graph] Попытка удалить несуществующее ребро";
    }
}

QList<Vertex*> Graph::getVertices() const {
    return m_vertices;
}

QList<Edge*> Graph::getEdges() const {
    return m_edges;
}

QList<Edge*> Graph::getAdjacentEdges(Vertex* v) const {
    QList<Edge*> result;
    for (Edge* e : m_edges) {
        if (e->getFrom() == v || e->getTo() == v) {
            result.append(e);
        }
    }
    return result;
}

void Graph::clear() {
    qDebug() << "[Graph] Очистка графа. Удаление" << m_vertices.size() << "вершин и" << m_edges.size() << "рёбер.";
    for (Edge* e : m_edges) delete e;
    for (Vertex* v : m_vertices) delete v;
    m_edges.clear();
    m_vertices.clear();
    m_nextVertexId = 0;
    qDebug() << "[Graph] Граф очищен.";
}

void Graph::generateRandomGraph(int vertexCount, int edgeCount, const QRectF& area) {
    qDebug() << "[Graph] Генерация случайного графа: вершин =" << vertexCount << "рёбер =" << edgeCount << "область =" << area;
    // Очищаем текущий граф
    clear();

    if (vertexCount <= 0) return;
    if (edgeCount < vertexCount - 1) {
        edgeCount = vertexCount - 1; // минимальное количество рёбер для связности
    }
    int maxEdges = vertexCount * (vertexCount - 1) / 2;
    if (edgeCount > maxEdges) {
        edgeCount = maxEdges;
    }

    // Генератор случайных чисел
    QRandomGenerator* rng = QRandomGenerator::global();

    // Создаём вершины со случайными позициями в области area
    QList<Vertex*> vertices;
    for (int i = 0; i < vertexCount; ++i) {
        qreal x = area.left() + rng->bounded(static_cast<int>(area.width()));
        qreal y = area.top() + rng->bounded(static_cast<int>(area.height()));
        Vertex* v = addVertex(QPointF(x, y));
        vertices.append(v);
    }

    // Создаём остовное дерево для гарантии связности
    // Используем алгоритм: начинаем с первой вершины, добавляем каждую следующую,
    // соединяя со случайной уже добавленной вершиной
    QList<Vertex*> treeVertices;
    treeVertices.append(vertices.first());
    for (int i = 1; i < vertices.size(); ++i) {
        Vertex* newVertex = vertices.at(i);
        Vertex* connectTo = treeVertices.at(rng->bounded(treeVertices.size()));
        int weight = 1 + rng->bounded(100);
        addEdge(connectTo, newVertex, weight);
        treeVertices.append(newVertex);
    }

    // Добавляем оставшиеся рёбра случайным образом
    int edgesAdded = vertexCount - 1; // уже добавлено рёбер в дереве
    // Создаём список всех возможных пар вершин (без повторений)
    QList<QPair<int, int>> possiblePairs;
    for (int i = 0; i < vertexCount; ++i) {
        for (int j = i + 1; j < vertexCount; ++j) {
            possiblePairs.append(qMakePair(i, j));
        }
    }
    // Перемешиваем пары
    for (int i = 0; i < possiblePairs.size(); ++i) {
        int j = rng->bounded(possiblePairs.size());
        qSwap(possiblePairs[i], possiblePairs[j]);
    }

    // Добавляем рёбра, пока не достигнем нужного количества
    for (int idx = 0; idx < possiblePairs.size() && edgesAdded < edgeCount; ++idx) {
        int i = possiblePairs[idx].first;
        int j = possiblePairs[idx].second;
        Vertex* v1 = vertices.at(i);
        Vertex* v2 = vertices.at(j);
        // Проверяем, существует ли уже ребро
        bool exists = false;
        for (Edge* e : m_edges) {
            if ((e->getFrom() == v1 && e->getTo() == v2) ||
                (e->getFrom() == v2 && e->getTo() == v1)) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            int weight = 1 + rng->bounded(100);
            addEdge(v1, v2, weight);
            edgesAdded++;
        }
    }
}