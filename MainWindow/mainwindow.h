#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QListWidget>
#include "ui_mainwindow.h"
#include "Typedefs.h"
#include "GraphTabWidget.h"
#include "Tool.h"
#include "AlgorithmInfo.h"

class Config;
class EdgeImage;
class GraphShapeDialog;
class GraphView;
class LoopEdgeImage;
class StraightEdgeImage;
class Vertex;
class VertexImage;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	typedef std::vector<QAction*> ActionVector;
	ActionVector _buttons;
	GraphTabWidget * _graphTabs;
	AlgorithmInfo _algorithmInfo;

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindowClass ui;
	void setCursorForWidget(QWidget * widget, Qt::CursorShape shape);

private slots:
	void newFile();
	void open();
	void saveAs();
	void close();
	void checkAddVertexButton(bool b);
	void checkAddEdgeButton(bool b);
	void checkSelectionButton(bool b);
	void checkPointerButton(bool b);
	void checkRemoveButton(bool b);
	void checkZoomButton(bool b);
	void openGraphShapeDialog();
	void changeGraphInformation();
	void updateGraphStatus() const;
	void runAlgorithm(QListWidgetItem * item);
	void showAboutMessageBox();

private:
	void createActions() const;
	void createButtonVector();
	void checkButton(Tool * tool, QAction * action, bool b = true);
	void uncheckButtons(QAction const * action);
	GraphImage * createGraph(GraphCreateFunc func, EdgeStrategyCreateFunc strategyFunc);
};

#endif // MAINWINDOW_H
