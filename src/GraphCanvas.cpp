#include "GraphCanvas.h"
#include <QPainter>
#include <QDebug>
#include <QMessageBox>

const QColor COLOR_VERTEX_NORMAL = QColor(240, 240, 240); // светло-серый (белый)
const QColor COLOR_VERTEX_START = QColor(255, 165, 0);    // оранжевый (для стартовой вершины, которая в очереди)
const QColor COLOR_VERTEX_CURRENT = Qt::yellow;           // жёлтый
const QColor COLOR_VERTEX_VISITED = Qt::green;            // зелёный (финализирована)
const QColor COLOR_VERTEX_SELECTED = Qt::cyan;            // голубой для выделения (не меняем логику)
const QColor COLOR_EDGE_CONSIDERING = Qt::yellow;         // жёлтый (ребро ослабляется сейчас)
const QColor COLOR_EDGE_IN_QUEUE = QColor(255, 165, 0);   // оранжевый (ребро в очереди)
const QColor COLOR_EDGE_IN_PATH = Qt::green;              // зелёный (ребро в дереве ответов)
const QColor COLOR_EDGE_REJECTED = Qt::red;               // красный (ребро отвергнуто)
const qreal VERTEX_RADIUS = 20.0;

QColor GraphCanvas::getEdgeNormalColor() const
{
    // Определяем цвет фона виджета
    QColor bg = palette().color(QPalette::Window);
    // Вычисляем яркость (luminance) по формуле
    int lightness = bg.lightness(); // 0-255
    // Если фон темный (lightness < 128), ребра должны быть светлыми, иначе темными
    return lightness < 128 ? Qt::white : Qt::black;
}

QColor GraphCanvas::getContrastTextColor() const
{
    QColor bg = palette().color(QPalette::Window);
    int lightness = bg.lightness();
    qDebug() << "[getContrastTextColor] bg:" << bg.name() << "lightness:" << lightness << "returning" << (lightness < 128 ? "white" : "black");
    return lightness < 128 ? Qt::white : Qt::black;
}

GraphCanvas::GraphCanvas(QWidget *parent)
    : QWidget(parent),
      m_graph(nullptr),
      m_solver(nullptr),
      m_selectedVertex(nullptr),
      m_draggedVertex(nullptr),
      m_startVertex(nullptr),
      m_mode(Normal),
      m_edgeStartVertex(nullptr),
      m_draggedEdge(nullptr),
      m_dragLabelOffset(0, 0)
{
    setFocusPolicy(Qt::StrongFocus); // чтобы получать события клавиатуры
    setMouseTracking(true);
}

void GraphCanvas::setGraph(Graph* graph) {
    m_graph = graph;
    update();
}

void GraphCanvas::setSolver(DijkstraSolver* solver) {
    m_solver = solver;
    update();
}

void GraphCanvas::setStartVertex(Vertex* v) {
    m_startVertex = v;
    if (m_solver) {
        m_solver->setStartVertex(v);
    }
    update();
}

void GraphCanvas::startDijkstra() {
    if (m_solver && m_startVertex) {
        m_solver->reset();
        // можно запустить таймер для автоматических шагов
    }
}

bool GraphCanvas::stepDijkstra() {
    if (!m_solver) return false;
    if (!m_startVertex) {
        QMessageBox::information(this, "Информация", "Сначала установите стартовую вершину.");
        return false;
    }
    if (m_solver->step()) {
        update();
        return true;
    }
    return false;
}

void GraphCanvas::resetVisualization() {
    if (m_solver) {
        m_solver->reset();
    }
    update();
}

void GraphCanvas::setMode(Mode mode) {
    m_mode = mode;
    m_selectedVertices.clear();
    m_edgeStartVertex = nullptr;
    m_draggedEdge = nullptr;
}

void GraphCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Рисуем рёбра
    if (m_graph) {
        for (Edge* e : m_graph->getEdges()) {
            drawEdge(painter, e);
        }
    }

    // Рисуем вершины
    if (m_graph) {
        for (Vertex* v : m_graph->getVertices()) {
            drawVertex(painter, v);
        }
    }

    // Временная линия для добавления ребра
    if (m_mode == AddEdge && m_edgeStartVertex) {
        painter.setPen(QPen(Qt::darkGray, 2, Qt::DashLine));
        painter.drawLine(m_edgeStartVertex->getPosition(), mapFromGlobal(QCursor::pos()));
    }
}

void GraphCanvas::drawVertex(QPainter& painter, Vertex* v) {
    QPointF center = v->getPosition();
    qreal radius = VERTEX_RADIUS;

    // Выбор цвета заливки
    QColor fillColor = COLOR_VERTEX_NORMAL;
    if (m_solver) {
        DijkstraSolver::VertexState state = m_solver->getVertexState(v);
        switch (state) {
            case DijkstraSolver::VS_WHITE:
                fillColor = COLOR_VERTEX_NORMAL; // белый
                break;
            case DijkstraSolver::VS_ORANGE:
                fillColor = COLOR_VERTEX_START; // оранжевый
                break;
            case DijkstraSolver::VS_YELLOW:
                fillColor = COLOR_VERTEX_CURRENT; // жёлтый
                break;
            case DijkstraSolver::VS_GREEN:
                fillColor = COLOR_VERTEX_VISITED; // зелёный
                break;
            default:
                fillColor = COLOR_VERTEX_NORMAL;
        }
    } else {
        // Без солвера используем старую логику для стартовой вершины
        if (v == m_startVertex) {
            fillColor = COLOR_VERTEX_START; // оранжевый
        }
    }

    // Определение цвета текста: для цветных вершин — черный, для белых — тоже черный (контрастный относительно вершины)
    QColor textColor = Qt::black;
    qDebug() << "[drawVertex] id:" << v->getId() << "fillColor:" << fillColor.name() << "textColor:" << textColor.name() << "isNormal:" << (fillColor == COLOR_VERTEX_NORMAL);

    // Подсветка выделенной вершины
    if (m_selectedVertices.contains(v) || v == m_selectedVertex) {
        painter.setPen(QPen(COLOR_VERTEX_SELECTED, 4));
    } else {
        painter.setPen(QPen(getContrastTextColor(), 2));
    }

    painter.setBrush(fillColor);
    painter.drawEllipse(center, radius, radius);

    // Текст: ID и расстояние
    QString text = QString::number(v->getId());
    if (m_solver) {
        int dist = m_solver->getDistances().value(v, INT_MAX);
        if (dist == INT_MAX) {
            text += QString("\n∞");
        } else {
            text += QString("\n%1").arg(dist);
        }
    }
    painter.setPen(textColor);
    painter.drawText(QRectF(center.x() - radius, center.y() - radius,
                            radius*2, radius*2), Qt::AlignCenter, text);
}

void GraphCanvas::drawEdge(QPainter& painter, Edge* e) {
    QPointF p1 = e->getFrom()->getPosition();
    QPointF p2 = e->getTo()->getPosition();

    // Определение цвета на основе состояния ребра
    QColor edgeColor = getEdgeNormalColor(); // белый/чёрный по умолчанию
    
    if (m_solver) {
        DijkstraSolver::EdgeState state = m_solver->getEdgeState(e);
        switch (state) {
            case DijkstraSolver::ES_WHITE:
                edgeColor = getEdgeNormalColor();
                break;
            case DijkstraSolver::ES_ORANGE:
                edgeColor = COLOR_EDGE_IN_QUEUE; // оранжевый
                break;
            case DijkstraSolver::ES_YELLOW:
                edgeColor = COLOR_EDGE_CONSIDERING; // жёлтый
                break;
            case DijkstraSolver::ES_GREEN:
                edgeColor = COLOR_EDGE_IN_PATH; // зелёный
                break;
            case DijkstraSolver::ES_RED:
                edgeColor = COLOR_EDGE_REJECTED; // красный
                break;
            default:
                edgeColor = getEdgeNormalColor();
        }
    }

    painter.setPen(QPen(edgeColor, 3));
    painter.drawLine(p1, p2);

    // Вес ребра
    QPointF labelPos;
    if (e->hasCustomLabelPos()) {
        labelPos = e->getLabelPos();
    } else {
        labelPos = (p1 + p2) / 2;
        // Можно сохранить как позицию по умолчанию, но не будем устанавливать флаг custom
    }
    
    QString weightText = QString::number(e->getWeight());
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(weightText);
    textRect.moveCenter(labelPos.toPoint());
    textRect.adjust(-4, -2, 4, 2); // отступы
    
    // Фон для лучшей читаемости
    painter.setBrush(palette().color(QPalette::Window));
    painter.setPen(Qt::NoPen);
    painter.drawRect(textRect);
    
    painter.setPen(getContrastTextColor());
    painter.drawText(labelPos, weightText);
}

Vertex* GraphCanvas::vertexAt(const QPointF& pos) const {
    if (!m_graph) return nullptr;
    const qreal threshold = VERTEX_RADIUS * 2;
    for (Vertex* v : m_graph->getVertices()) {
        QPointF diff = v->getPosition() - pos;
        if (diff.x() * diff.x() + diff.y() * diff.y() < threshold * threshold) {
            return v;
        }
    }
    return nullptr;
}

Edge* GraphCanvas::edgeAt(const QPointF& pos) const {
    if (!m_graph) return nullptr;
    
    const qreal threshold = 10.0; // пикселей
    Edge* closestEdge = nullptr;
    qreal minDist = threshold;
    
    for (Edge* e : m_graph->getEdges()) {
        QPointF p1 = e->getFrom()->getPosition();
        QPointF p2 = e->getTo()->getPosition();
        
        // Вычисление расстояния от точки до отрезка
        qreal A = pos.x() - p1.x();
        qreal B = pos.y() - p1.y();
        qreal C = p2.x() - p1.x();
        qreal D = p2.y() - p1.y();
        
        qreal dot = A * C + B * D;
        qreal lenSq = C * C + D * D;
        qreal param = (lenSq != 0) ? dot / lenSq : -1;
        
        qreal xx, yy;
        if (param < 0) {
            xx = p1.x();
            yy = p1.y();
        } else if (param > 1) {
            xx = p2.x();
            yy = p2.y();
        } else {
            xx = p1.x() + param * C;
            yy = p1.y() + param * D;
        }
        
        qreal dx = pos.x() - xx;
        qreal dy = pos.y() - yy;
        qreal dist = sqrt(dx * dx + dy * dy);
        
        if (dist < minDist) {
            minDist = dist;
            closestEdge = e;
        }
    }
    
    return closestEdge;
}

Edge* GraphCanvas::edgeAtWeight(const QPointF& pos) const {
    if (!m_graph) return nullptr;
    
    const qreal threshold = 20.0; // пикселей, размер метки
    Edge* closestEdge = nullptr;
    qreal minDist = threshold;
    
    for (Edge* e : m_graph->getEdges()) {
        QPointF p1 = e->getFrom()->getPosition();
        QPointF p2 = e->getTo()->getPosition();
        QPointF labelPos;
        if (e->hasCustomLabelPos()) {
            labelPos = e->getLabelPos();
        } else {
            labelPos = (p1 + p2) / 2;
        }
        
        // Вычисляем расстояние от точки до центра метки
        qreal dx = pos.x() - labelPos.x();
        qreal dy = pos.y() - labelPos.y();
        qreal dist = sqrt(dx * dx + dy * dy);
        
        if (dist < minDist) {
            minDist = dist;
            closestEdge = e;
        }
    }
    
    return closestEdge;
}

void GraphCanvas::addVertexAt(const QPointF& pos) {
    if (m_graph) {
        m_graph->addVertex(pos);
        update();
    }
}

void GraphCanvas::addEdgeBetween(Vertex* from, Vertex* to, int weight) {
    if (m_graph && from && to && from != to) {
        Edge* e = m_graph->addEdge(from, to, weight);
        if (e) {
            update();
        } else {
            QMessageBox::warning(this, "Ошибка", "Ребро уже существует");
        }
    }
}

void GraphCanvas::deleteSelected() {
    if (!m_graph) return;
    for (Vertex* v : m_selectedVertices) {
        m_graph->removeVertex(v);
    }
    m_selectedVertices.clear();
    update();
}

void GraphCanvas::mousePressEvent(QMouseEvent* event) {
    QPointF pos = event->pos();
    Vertex* clickedVertex = vertexAt(pos);

    if (event->button() == Qt::RightButton) {
        // ПКМ -> добавить вершину, только если в окрестности нет другой вершины
        if (!clickedVertex) {
            addVertexAt(pos);
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        // Сначала проверяем, не кликнули ли на метку веса ребра
        Edge* clickedEdgeWeight = edgeAtWeight(pos);
        if (clickedEdgeWeight) {
            // Начинаем перетаскивание метки веса
            m_draggedEdge = clickedEdgeWeight;
            QPointF labelPos = clickedEdgeWeight->hasCustomLabelPos()
                ? clickedEdgeWeight->getLabelPos()
                : (clickedEdgeWeight->getFrom()->getPosition() + clickedEdgeWeight->getTo()->getPosition()) / 2;
            m_dragLabelOffset = pos - labelPos;
            m_mode = DragWeight;
            update();
            return;
        }
        
        if (clickedVertex) {
            if (event->modifiers() & Qt::ShiftModifier) {
                // Shift+клик -> выделение вершины для создания ребра
                m_selectedVertices.append(clickedVertex);
                if (m_selectedVertices.size() == 2) {
                    // Запрос веса и создание ребра
                    bool ok;
                    int weight = QInputDialog::getInt(this, "Вес ребра",
                        "Введите вес ребра:", 1, 1, 1000, 1, &ok);
                    if (ok) {
                        addEdgeBetween(m_selectedVertices[0], m_selectedVertices[1], weight);
                    }
                    m_selectedVertices.clear();
                }
                update();
            } else {
                // Обычный клик -> выделение одной вершины
                m_selectedVertices.clear();
                m_selectedVertices.append(clickedVertex);
                m_draggedVertex = clickedVertex;
                m_dragOffset = pos - clickedVertex->getPosition();
                update();
            }
        } else {
            // Клик по пустому месту -> сброс выделения
            m_selectedVertices.clear();
            m_draggedVertex = nullptr;
            update();
        }
    }
}

void GraphCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (m_draggedVertex) {
        m_draggedVertex->setPosition(event->pos() - m_dragOffset);
        update();
    } else if (m_draggedEdge && m_mode == DragWeight) {
        QPointF newLabelPos = event->pos() - m_dragLabelOffset;
        m_draggedEdge->setLabelPos(newLabelPos);
        update();
    }
}

void GraphCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_draggedVertex = nullptr;
        if (m_draggedEdge) {
            m_draggedEdge = nullptr;
            m_mode = Normal;
        }
    }
}

void GraphCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->pos();
        Edge* edge = edgeAt(pos);
        if (edge) {
            // Изменение веса ребра
            bool ok;
            int newWeight = QInputDialog::getInt(this, "Вес ребра",
                "Новый вес:", edge->getWeight(), 1, 1000, 1, &ok);
            if (ok) {
                edge->setWeight(newWeight);
                update();
            }
            return;
        }
        Vertex* vertex = vertexAt(pos);
        if (vertex) {
            // Двойной щелчок по вершине - можно сделать что-то, например, выделить
            m_selectedVertices.clear();
            m_selectedVertices.append(vertex);
            update();
        }
    }
}

void GraphCanvas::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete) {
        deleteSelected();
    } else if (event->key() == Qt::Key_Escape) {
        m_selectedVertices.clear();
        update();
    }
}

void GraphCanvas::contextMenuEvent(QContextMenuEvent* event) {
    QPoint pos = event->pos();
    Vertex* vertex = vertexAt(pos);
    Edge* edge = edgeAt(pos);

    QMenu menu(this);
    if (vertex) {
        menu.addAction("Удалить вершину", [this, vertex]() {
            m_graph->removeVertex(vertex);
            update();
        });
        menu.addAction("Сделать стартовой", [this, vertex]() {
            setStartVertex(vertex);
        });
    } else if (edge) {
        menu.addAction("Удалить ребро", [this, edge]() {
            m_graph->removeEdge(edge);
            update();
        });
        menu.addAction("Изменить вес", [this, edge]() {
            bool ok;
            int newWeight = QInputDialog::getInt(this, "Вес ребра",
                "Новый вес:", edge->getWeight(), 1, 1000, 1, &ok);
            if (ok) {
                edge->setWeight(newWeight);
                update();
            }
        });
    } else {
        menu.addAction("Добавить вершину", [this, pos]() {
            addVertexAt(pos);
        });
    }
    menu.exec(event->globalPos());
}

Vertex* GraphCanvas::selectedVertex() const {
    if (!m_selectedVertices.isEmpty()) {
        return m_selectedVertices.first();
    }
    return nullptr;
}