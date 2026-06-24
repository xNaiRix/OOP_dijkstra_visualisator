#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QGridLayout>
#include <climits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_graph(nullptr),
      m_solver(nullptr)
{
    setupUI();
    updateStatus();
}

void MainWindow::setGraphAndSolver(Graph* graph, DijkstraSolver* solver) {
    m_graph = graph;
    m_solver = solver;
    if (m_canvas) {
        m_canvas->setGraph(graph);
        m_canvas->setSolver(solver);
    }
}

void MainWindow::setupUI() {
    // Создание виджетов
    m_canvas = new GraphCanvas(this);
    m_btnSetStart = new QPushButton("Установить старт", this);
    m_btnStart = new QPushButton("Запуск", this);
    m_btnStep = new QPushButton("Шаг", this);
    m_btnReset = new QPushButton("Сброс", this);
    m_btnGenerateRandom = new QPushButton("Случайный граф", this);
    m_statusLabel = new QLabel("Готово", this);
    m_distanceTable = new QTableWidget(this);
    m_timer = new QTimer(this);
    m_timer->setInterval(1000); // 1 секунда

    // Настройка таблицы расстояний
    m_distanceTable->setColumnCount(2);
    m_distanceTable->setHorizontalHeaderLabels({"Вершина", "Расстояние"});
    m_distanceTable->horizontalHeader()->setStretchLastSection(true);
    m_distanceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_distanceTable->setMaximumWidth(300);

    // Горизонтальная панель кнопок
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_btnSetStart);
    buttonLayout->addWidget(m_btnStart);
    buttonLayout->addWidget(m_btnStep);
    buttonLayout->addWidget(m_btnReset);
    buttonLayout->addWidget(m_btnGenerateRandom);
    buttonLayout->addStretch();

    // Левая часть: кнопки, canvas, статус
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addLayout(buttonLayout);
    leftLayout->addWidget(m_canvas, 1);
    leftLayout->addWidget(m_statusLabel, 0);

    // Правая часть: таблица расстояний
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(new QLabel("Таблица расстояний", this));
    rightLayout->addWidget(m_distanceTable);
    // Добавляем пояснения цветов
    rightLayout->addWidget(createColorLegendWidget());

    // Основной горизонтальный layout
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout, 3); // 3 части ширины
    mainLayout->addLayout(rightLayout, 1); // 1 часть ширины

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Соединение сигналов
    connect(m_btnSetStart, &QPushButton::clicked, this, &MainWindow::onSetStartClicked);
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_btnStep, &QPushButton::clicked, this, &MainWindow::onStepClicked);
    connect(m_btnReset, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(m_btnGenerateRandom, &QPushButton::clicked, this, &MainWindow::onGenerateRandomClicked);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);
}

void MainWindow::updateStatus() {
    if (!m_canvas || !m_solver) {
        m_statusLabel->setText("Готово");
        updateDistanceTable();
        return;
    }
    
    QString status;
    if (m_solver->isFinished()) {
        status = "Алгоритм завершён.";
    } else {
        Vertex* cur = m_solver->getCurrentVertex();
        if (cur) {
            int dist = m_solver->getDistances().value(cur, INT_MAX);
            status = QString("Текущая вершина: %1, расстояние: %2")
                        .arg(cur->getId())
                        .arg(dist == INT_MAX ? "∞" : QString::number(dist));
        } else {
            status = "Алгоритм не запущен.";
        }
    }
    m_statusLabel->setText(status);
    updateDistanceTable();
}

void MainWindow::onSetStartClicked() {
    Vertex* selected = m_canvas->selectedVertex();
    if (selected) {
        m_canvas->setStartVertex(selected);
        m_statusLabel->setText(QString("Стартовая вершина установлена: %1").arg(selected->getId()));
    } else {
        m_statusLabel->setText("Ошибка: не выбрана вершина. Щёлкните на вершине левой кнопкой мыши.");
    }
}

void MainWindow::onStartClicked() {
    m_timer->start();
    m_statusLabel->setText("Алгоритм выполняется автоматически...");
}

void MainWindow::onStepClicked() {
    if (m_canvas->stepDijkstra()) {
        updateStatus();
    } else {
        m_statusLabel->setText("Алгоритм завершён.");
    }
}

void MainWindow::onResetClicked() {
    m_timer->stop();
    m_canvas->resetVisualization();
    m_statusLabel->setText("Сброс выполнен.");
}

void MainWindow::onGenerateRandomClicked() {
    if (!m_graph || !m_canvas) return;

    // Диалог для ввода количества вершин
    bool ok;
    int vertexCount = QInputDialog::getInt(this, "Генерация случайного графа",
                                           "Количество вершин:", 10, 1, 100, 1, &ok);
    if (!ok) return;

    // Максимальное количество рёбер для простого графа
    int maxEdges = vertexCount * (vertexCount - 1) / 2;
    int edgeCount = QInputDialog::getInt(this, "Генерация случайного графа",
                                         "Количество рёбер:", vertexCount * 2, vertexCount - 1, maxEdges, 1, &ok);
    if (!ok) return;

    // Определяем область размещения вершин (с отступами)
    QRectF area = m_canvas->rect().adjusted(50, 50, -50, -50);
    if (area.width() <= 0 || area.height() <= 0) {
        area = QRectF(50, 50, 500, 400);
    }

    // Генерируем граф
    m_graph->generateRandomGraph(vertexCount, edgeCount, area);

    // Сбрасываем визуализацию
    m_canvas->resetVisualization();
    m_canvas->update();

    // Обновляем статус и таблицу
    updateStatus();
    m_statusLabel->setText(QString("Сгенерирован случайный граф: %1 вершин, %2 рёбер.")
                           .arg(vertexCount).arg(edgeCount));
}

void MainWindow::onTimerTimeout() {
    if (!m_canvas->stepDijkstra()) {
        m_timer->stop();
        m_statusLabel->setText("Алгоритм завершён.");
    }
    updateStatus();
}

void MainWindow::updateDistanceTable() {
    if (!m_solver || !m_graph) {
        m_distanceTable->setRowCount(0);
        return;
    }
    
    QList<Vertex*> vertices = m_graph->getVertices();
    m_distanceTable->setRowCount(vertices.size());
    
    QMap<Vertex*, int> distances = m_solver->getDistances();
    for (int i = 0; i < vertices.size(); ++i) {
        Vertex* v = vertices[i];
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(v->getId()));
        int dist = distances.value(v, INT_MAX);
        QTableWidgetItem* distItem = new QTableWidgetItem(
            dist == INT_MAX ? "∞" : QString::number(dist));
        
        // Цветовое выделение
        if (v == m_solver->getCurrentVertex()) {
            idItem->setBackground(Qt::blue);
            distItem->setBackground(Qt::blue);
        } else if (!m_solver->getUnvisited().contains(v)) {
            idItem->setBackground(QColor(100, 200, 255)); // светло-голубой
            distItem->setBackground(QColor(100, 200, 255));
        } else if (v == m_solver->getStartVertex()) {
            idItem->setBackground(Qt::green);
            distItem->setBackground(Qt::green);
        }
        
        m_distanceTable->setItem(i, 0, idItem);
        m_distanceTable->setItem(i, 1, distItem);
    }
}

void MainWindow::addColorRow(QGridLayout* layout, int row, const QColor& color, const QString& text) {
    QLabel* colorLabel = new QLabel();
    colorLabel->setFixedSize(16, 16);
    colorLabel->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
    layout->addWidget(colorLabel, row, 0);

    QLabel* textLabel = new QLabel(text);
    layout->addWidget(textLabel, row, 1);
}

QWidget* MainWindow::createColorLegendWidget() {
    QWidget* widget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(10);

    // Раздел "Цвета вершин"
    QGroupBox* vertexGroup = new QGroupBox("Цвета вершин", widget);
    QGridLayout* vertexLayout = new QGridLayout(vertexGroup);
    vertexLayout->setVerticalSpacing(4);
    vertexLayout->setHorizontalSpacing(10);

    addColorRow(vertexLayout, 0, QColor(240, 240, 240), "Обычная вершина");
    addColorRow(vertexLayout, 1, QColor(255, 165, 0), "Стартовая вершина");
    addColorRow(vertexLayout, 2, Qt::yellow, "Текущая вершина");
    addColorRow(vertexLayout, 3, Qt::green, "Посещённая вершина");
    addColorRow(vertexLayout, 4, Qt::cyan, "Выделенная вершина");

    vertexGroup->setLayout(vertexLayout);
    mainLayout->addWidget(vertexGroup);

    // Раздел "Цвета рёбер"
    QGroupBox* edgeGroup = new QGroupBox("Цвета рёбер", widget);
    QGridLayout* edgeLayout = new QGridLayout(edgeGroup);
    edgeLayout->setVerticalSpacing(4);
    edgeLayout->setHorizontalSpacing(10);

    addColorRow(edgeLayout, 0, Qt::yellow, "Ребро ослабляется сейчас");
    addColorRow(edgeLayout, 1, QColor(255, 165, 0), "Ребро в очереди");
    addColorRow(edgeLayout, 2, Qt::green, "Ребро в дереве ответов");
    addColorRow(edgeLayout, 3, Qt::red, "Ребро отвергнуто");
    addColorRow(edgeLayout, 4, Qt::white, "Обычное ребро");

    edgeGroup->setLayout(edgeLayout);
    mainLayout->addWidget(edgeGroup);

    mainLayout->addStretch();
    return widget;
}