#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QTableWidget>
#include <QGridLayout>
#include <QGroupBox>
#include "GraphCanvas.h"
#include "Graph.h"
#include "DijkstraSolver.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    void setGraphAndSolver(Graph* graph, DijkstraSolver* solver);

private slots:
    void onSetStartClicked();
    void onStartClicked();
    void onStepClicked();
    void onResetClicked();
    void onTimerTimeout();
    void onGenerateRandomClicked();

private:
    GraphCanvas* m_canvas;
    QPushButton* m_btnSetStart;
    QPushButton* m_btnStart;
    QPushButton* m_btnStep;
    QPushButton* m_btnReset;
    QPushButton* m_btnGenerateRandom;
    QLabel* m_statusLabel;
    QTableWidget* m_distanceTable;
    QTimer* m_timer;
    Graph* m_graph;
    DijkstraSolver* m_solver;

    void setupUI();
    void updateStatus();
    void updateDistanceTable();
    QWidget* createColorLegendWidget();
    void addColorRow(QGridLayout* layout, int row, const QColor& color, const QString& text);
};

#endif // MAINWINDOW_H