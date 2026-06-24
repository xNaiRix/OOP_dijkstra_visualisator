#include "DijkstraSolver.h"
#include <QDebug>
#include <QSet>

DijkstraSolver::DijkstraSolver(Graph* graph)
    : m_graph(graph),
      m_startVertex(nullptr),
      m_currentVertex(nullptr),
      m_previousYellowVertex(nullptr),
      m_finished(true),
      m_substep(0)
{
    if (m_graph) {
        reset();
    }
}

void DijkstraSolver::setStartVertex(Vertex* v) {
    m_startVertex = v;
    reset();
}

bool DijkstraSolver::step() {
    // Если алгоритм уже завершён, ничего не делаем
    if (m_finished) {
        qDebug() << "Algorithm already finished";
        return false;
    }

    qDebug() << "=== Step: substep" << m_substep;

    if (m_substep == 0) {
        // Фаза релаксации и правило 1
        // 1. Определяем текущую жёлтую вершину (должна быть ровно одна)
        Vertex* currentYellow = nullptr;
        for (Vertex* v : m_graph->getVertices()) {
            if (m_vertexState.value(v, VS_WHITE) == VS_YELLOW) {
                if (currentYellow) {
                    qWarning() << "More than one yellow vertex found!";
                }
                currentYellow = v;
            }
        }

        // Если жёлтой вершины нет, переходим к фазе выбора следующей вершины
        if (!currentYellow) {
            qDebug() << "No yellow vertex, switching to substep 1";
            m_substep = 1;
            // Повторяем шаг, чтобы сразу выполнить фазу выбора
            return step();
        }

        // Сохраняем старые цвета для применения правил (состояния до итерации)
        m_oldVertexState = m_vertexState;
        m_oldEdgeState = m_edgeState;

        qDebug() << "  Current yellow vertex" << currentYellow->getId();

        // 2. Релаксация рёбер из текущей жёлтой вершины (внутренний шаг алгоритма Дейкстры)
        int curDist = m_distances.value(currentYellow, INT_MAX);
        if (curDist != INT_MAX) {
            for (Edge* e : m_graph->getAdjacentEdges(currentYellow)) {
                Vertex* other = (e->getFrom() == currentYellow) ? e->getTo() : e->getFrom();
                int newDist = curDist + e->getWeight();
                if (newDist < m_distances.value(other, INT_MAX)) {
                    m_distances[other] = newDist;
                    m_predecessors[other] = currentYellow;
                    qDebug() << "  Relaxation: distance to" << other->getId() << "updated to" << newDist;
                }
            }
        }

        // 3. Правило 3.1: белое ребро -> оранжевое (инцидентное текущей жёлтой вершине)
        qDebug() << "  Rule 3.1: checking edges adjacent to yellow vertex" << currentYellow->getId();
        for (Edge* e : m_graph->getAdjacentEdges(currentYellow)) {
            if (m_oldEdgeState.value(e, ES_WHITE) == ES_WHITE) {
                setEdgeState(e, ES_ORANGE);
                qDebug() << "    edge" << e->getFrom()->getId() << "-" << e->getTo()->getId() << "set ORANGE";
            } else {
                qDebug() << "    edge" << e->getFrom()->getId() << "-" << e->getTo()->getId() << "old state not white:" << (int)m_oldEdgeState.value(e, ES_WHITE);
            }
        }

        // 4. Правило 3.2: белая вершина -> оранжевая (через новые оранжевые рёбра)
        QSet<Edge*> newOrangeEdges;
        for (Edge* e : m_graph->getEdges()) {
            if (m_edgeState.value(e, ES_WHITE) == ES_ORANGE && m_oldEdgeState.value(e, ES_WHITE) == ES_WHITE) {
                newOrangeEdges.insert(e);
            }
        }
        for (Edge* e : newOrangeEdges) {
            Vertex* u = e->getFrom();
            Vertex* v = e->getTo();
            if (m_oldVertexState.value(u, VS_WHITE) == VS_WHITE) {
                setVertexState(u, VS_ORANGE);
                qDebug() << "  Rule 3.2: vertex" << u->getId() << "set ORANGE";
            }
            if (m_oldVertexState.value(v, VS_WHITE) == VS_WHITE) {
                setVertexState(v, VS_ORANGE);
                qDebug() << "  Rule 3.2: vertex" << v->getId() << "set ORANGE";
            }
        }

        // 5. Правило 1: жёлтое -> зелёное (вершина и рёбра)
        qDebug() << "  Rule 1: checking vertices with old state YELLOW";
        for (Vertex* v : m_graph->getVertices()) {
            if (m_oldVertexState.value(v, VS_WHITE) == VS_YELLOW) {
                qDebug() << "    vertex" << v->getId() << "old state YELLOW -> setting GREEN";
                setVertexState(v, VS_GREEN);
            }
        }
        qDebug() << "  Rule 1: checking edges with old state YELLOW";
        for (Edge* e : m_graph->getEdges()) {
            if (m_oldEdgeState.value(e, ES_WHITE) == ES_YELLOW) {
                qDebug() << "    edge" << e->getFrom()->getId() << "-" << e->getTo()->getId() << "old state YELLOW -> setting GREEN";
                setEdgeState(e, ES_GREEN);
            }
        }

        // 6. Правило 4: оранжевое ребро -> красное (если соединяет зелёную и жёлтую вершины)
        // Используем m_oldVertexState (состояния вершин до итерации) и текущее состояние рёбер.
        // Проверяем все рёбра, которые сейчас оранжевые (включая только что созданные).
        qDebug() << "  Rule 4: checking orange edges for green-yellow condition";
        for (Edge* e : m_graph->getEdges()) {
            if (m_edgeState.value(e, ES_WHITE) == ES_ORANGE) {
                Vertex* u = e->getFrom();
                Vertex* v = e->getTo();
                VertexState su = m_oldVertexState.value(u, VS_WHITE);
                VertexState sv = m_oldVertexState.value(v, VS_WHITE);
                bool greenYellow = (su == VS_GREEN && sv == VS_YELLOW) ||
                                   (su == VS_YELLOW && sv == VS_GREEN);
                if (greenYellow) {
                    // Ребро не должно быть жёлтым (если стало жёлтым в этом же шаге, оставляем жёлтым)
                    if (m_edgeState.value(e, ES_WHITE) != ES_YELLOW) {
                        setEdgeState(e, ES_RED);
                        qDebug() << "    edge" << u->getId() << "-" << v->getId() << "set RED";
                    }
                }
            }
        }

        // Обновляем предыдущую жёлтую вершину для следующего шага
        m_previousYellowVertex = currentYellow;

        // Переключаемся на фазу выбора следующей жёлтой вершины
        m_substep = 1;
        return true;
    } else {
        // Фаза выбора следующей жёлтой вершины (правила 2.1-4)
        // Используем сохранённые состояния до итерации (m_oldVertexState, m_oldEdgeState)

        // Правила 3.1 и 3.2 теперь выполняются в substep 0

        // 4. Правило 2.1: оранжевое ребро -> жёлтое (выбор минимального по приоритету)
        // Используем текущее состояние рёбер (m_edgeState), чтобы учесть только что созданные оранжевые рёбра
        Edge* minOrangeEdge = nullptr;
        int minPriority = INT_MAX;
        for (Edge* e : m_graph->getEdges()) {
            if (m_edgeState.value(e, ES_WHITE) == ES_ORANGE) {
                Vertex* u = e->getFrom();
                Vertex* v = e->getTo();
                // Пропускаем рёбра, где обе вершины уже зелёные (обработаны)
                if (m_vertexState.value(u, VS_WHITE) == VS_GREEN && m_vertexState.value(v, VS_WHITE) == VS_GREEN) {
                    qDebug() << "  Rule 2.1: skipping edge" << u->getId() << "-" << v->getId() << "because both vertices are GREEN";
                    continue;
                }
                int du = m_distances.value(u, INT_MAX);
                int dv = m_distances.value(v, INT_MAX);
                int priority = qMin(du, dv) + e->getWeight();
                if (priority < minPriority) {
                    minPriority = priority;
                    minOrangeEdge = e;
                }
            }
        }
        if (minOrangeEdge) {
            setEdgeState(minOrangeEdge, ES_YELLOW);
            qDebug() << "  Rule 2.1: edge" << minOrangeEdge->getFrom()->getId() << "-" << minOrangeEdge->getTo()->getId() << "set YELLOW";
        }

        // 5. Правило 2.2: оранжевая вершина -> жёлтая (инцидентная выбранному жёлтому ребру)
        if (minOrangeEdge) {
            Vertex* u = minOrangeEdge->getFrom();
            Vertex* v = minOrangeEdge->getTo();
            // Если вершина оранжевая в текущем состоянии, становится жёлтой
            if (m_vertexState.value(u, VS_WHITE) == VS_ORANGE) {
                setVertexState(u, VS_YELLOW);
                m_currentVertex = u;
                qDebug() << "  Rule 2.2: vertex" << u->getId() << "set YELLOW";
            }
            if (m_vertexState.value(v, VS_WHITE) == VS_ORANGE) {
                setVertexState(v, VS_YELLOW);
                m_currentVertex = v;
                qDebug() << "  Rule 2.2: vertex" << v->getId() << "set YELLOW";
            }
        }

        // Если после применения правил нет жёлтой вершины и нет оранжевых вершин, алгоритм завершён
        bool hasYellow = false;
        bool hasOrange = false;
        for (Vertex* v : m_graph->getVertices()) {
            VertexState s = m_vertexState.value(v, VS_WHITE);
            if (s == VS_YELLOW) hasYellow = true;
            if (s == VS_ORANGE) hasOrange = true;
        }
        qDebug() << "=== DEBUG: Vertex states after rules ===";
        qDebug() << "  hasYellow:" << hasYellow << "hasOrange:" << hasOrange;
        if (!hasYellow && !hasOrange) {
            qDebug() << "No yellow or orange vertex, algorithm finished";
            // Проверим, остались ли оранжевые рёбра
            int orangeEdgesCount = 0;
            for (Edge* e : m_graph->getEdges()) {
                if (m_edgeState.value(e, ES_WHITE) == ES_ORANGE) {
                    orangeEdgesCount++;
                    Vertex* u = e->getFrom();
                    Vertex* v = e->getTo();
                    qDebug() << "    Remaining orange edge:" << u->getId() << "-" << v->getId();
                }
            }
            qDebug() << "  Total remaining orange edges:" << orangeEdgesCount;
            m_finished = true;
            return false;
        }

        // Переключаемся обратно на фазу релаксации
        m_substep = 0;
        return true;
    }
}

//геттеры
Vertex* DijkstraSolver::getCurrentVertex() const {return m_currentVertex;}

Vertex* DijkstraSolver::getStartVertex() const {return m_startVertex;}

QMap<Vertex*, int> DijkstraSolver::getDistances() const {return m_distances;}

QList<Vertex*> DijkstraSolver::getShortestPathTo(Vertex* target) const {
    QList<Vertex*> path;
    if (!m_distances.contains(target) || m_distances[target] == INT_MAX) {
        return path; // путь не существует
    }
    Vertex* cur = target;
    while (cur) {
        path.prepend(cur);
        cur = m_predecessors.value(cur, nullptr);
    }
    return path;
}

bool DijkstraSolver::isFinished() const {return m_finished;}

QList<Vertex*> DijkstraSolver::getUnvisited() const {return m_unvisited;}


DijkstraSolver::VertexState DijkstraSolver::getVertexState(Vertex* v) const {
    return m_vertexState.value(v, VS_WHITE);
}

DijkstraSolver::EdgeState DijkstraSolver::getEdgeState(Edge* e) const {
    return m_edgeState.value(e, ES_WHITE);
}

// сеттеры
void DijkstraSolver::setVertexState(Vertex* v, VertexState state) {
    m_vertexState[v] = state;
}

void DijkstraSolver::setEdgeState(Edge* e, EdgeState state) {
    // Терминальные состояния (зелёные и красные) не меняются
    EdgeState current = m_edgeState.value(e, ES_WHITE);
    if (current == ES_GREEN || current == ES_RED) {
        return;
    }
    m_edgeState[e] = state;
}

void DijkstraSolver::reset() {
    qDebug() << "=== reset() called";
    m_distances.clear();
    m_predecessors.clear();
    m_vertexState.clear();
    m_edgeState.clear();
    m_oldVertexState.clear();
    m_oldEdgeState.clear();
    m_unvisited = m_graph->getVertices();
    m_finished = false;
    m_currentVertex = nullptr;
    m_previousYellowVertex = nullptr;
    m_substep = 0;

    // Инициализация расстояний и состояний вершин
    for (Vertex* v : m_unvisited) {
        m_distances[v] = INT_MAX;
        m_predecessors[v] = nullptr;
        m_vertexState[v] = VS_WHITE;
    }

    // Инициализация состояний рёбер
    for (Edge* e : m_graph->getEdges()) {
        m_edgeState[e] = ES_WHITE;
    }

    // Если задана стартовая вершина, устанавливаем её состояние
    if (m_startVertex) {
        m_distances[m_startVertex] = 0;
        m_vertexState[m_startVertex] = VS_YELLOW; // итерация 0: стартовая вершина жёлтая
        m_currentVertex = m_startVertex;
        m_previousYellowVertex = nullptr;
        qDebug() << "Start vertex" << m_startVertex->getId() << "set YELLOW";
    }

    qDebug() << "Reseting complete";
}

