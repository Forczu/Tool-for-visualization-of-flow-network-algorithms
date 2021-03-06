#include "mainwindow.h"
#include "VertexImage.h"
#include "GraphShapeDialog.h"
#include "Config.h"
#include "GraphView.h"
#include "GraphImage.h"
#include "GraphSerializer.h"
#include "Tool.h"
#include "VertexAddTool.h"
#include "EdgeAddTool.h"
#include "RemoveTool.h"
#include "PointTool.h"
#include "FlowNetworkAlgorithmWindow.h"
#include "Strings.h"
#include "WeightedEdgeStrategy.h"
#include "ZoomTool.h"
#include "CreateNewGraphDialog.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
	ui.setupUi(this);
	_graphTabs = new GraphTabWidget(this);
	ui.mainLayout->addWidget(_graphTabs, 0, 1, 4, 2);
	ui.mainLayout->setColumnStretch(0, 1);
	_graphTabs->hide();
	createActions();
	createButtonVector();
	checkPointerButton(true);
}

void MainWindow::createButtonVector()
{
	_buttons.push_back(ui.actionAddVertex);
	_buttons.push_back(ui.actionAddEdge);
	_buttons.push_back(ui.actionGrab);
	_buttons.push_back(ui.actionSelect);
	_buttons.push_back(ui.actionPointer);
	_buttons.push_back(ui.actionRemove);
	_buttons.push_back(ui.actionZoom);
}

MainWindow::~MainWindow()
{
	delete _graphTabs;
}

void MainWindow::newFile()
{
	int index = _graphTabs->count() + 1;
	QString graphName;
#ifndef DEBUG
	CreateNewGraphDialog dialog(index);
	dialog.show();
	if (dialog.exec() != QDialog::Accepted)
		return;
	graphName = dialog.getGraphName();
#else
	graphName = "Debug";
#endif
	GraphImage * graph = createGraph(FlowNetwork::getInstance, WeightedEdgeStrategy::getInstance);
	if (_graphTabs->isHidden())
		_graphTabs->show();
	_graphTabs->addTab(graph, graphName);
	graph->setName(graphName);
}

void MainWindow::open()
{
	std::string fileName;
#ifdef DEBUG
	fileName = "tmp.xml";
#else
	fileName = QFileDialog::getOpenFileName(this,
		tr("Open Graph File..."), QString(), tr("XML File (*.xml)")).toStdString();
	if (fileName.empty())
		return;
#endif
	GraphSerializer serializer;
	try
	{
		GraphImage * graph = serializer.deserialize(fileName);
		if (_graphTabs->isHidden())
			_graphTabs->show();
		_graphTabs->addTab(graph, graph->getName());
	}
	catch (std::runtime_error const & e)
	{
		QMessageBox mbx;
		mbx.setWindowTitle(Strings::Instance().get(CORRUPTED_XML_FILE_TITLE));
		mbx.setText((Strings::Instance().get(CORRUPTED_XML_FILE)));
		mbx.exec();
	}
}

void MainWindow::saveAs()
{
	if (_graphTabs->count() == 0)
		return;
	QString fileName;
#ifdef DEBUG
	fileName = "serialized_graph.xml";
#else
	fileName = QFileDialog::getSaveFileName(this,
		Strings::Instance().get(SAVE_GRAPH_TO_FILE), QString(), tr("XML File (*.xml)"));
#endif
	auto graph = _graphTabs->currentGraphView()->getGraphImage();
	GraphSerializer serializer;
	serializer.serialize(*graph, fileName.toStdString());
}

void MainWindow::close()
{
	delete _graphTabs;
	QApplication::quit();
}

void MainWindow::checkAddVertexButton(bool b)
{
	checkButton(&VertexAddTool::Instance(), ui.actionAddVertex, b);
	setCursorForWidget(_graphTabs->currentWidget(), Qt::ArrowCursor);
}

void MainWindow::setCursorForWidget(QWidget * widget, Qt::CursorShape shape)
{
	if (!widget)
		return;
	widget->setCursor(shape);
}

void MainWindow::checkAddEdgeButton(bool b)
{
	checkButton(&EdgeAddTool::Instance(), ui.actionAddEdge, b);
	GraphView * currentView = _graphTabs->currentGraphView();
	if (currentView == nullptr)
		return;
	currentView->setCursor(Qt::ArrowCursor);
	currentView->setEdgeFlag(EdgeFlag::Source);
}

void MainWindow::checkSelectionButton(bool b)
{
	checkButton(0, 0);
}

void MainWindow::checkPointerButton(bool b)
{
	checkButton(&PointTool::Instance(), ui.actionPointer, b);
	setCursorForWidget(_graphTabs->currentWidget(), Qt::ArrowCursor);
}

void MainWindow::checkRemoveButton(bool b)
{
	checkButton(&RemoveTool::Instance(), ui.actionRemove, b);
	setCursorForWidget(_graphTabs->currentWidget(), Qt::CrossCursor);
}

void MainWindow::openGraphShapeDialog()
{
	if (_graphTabs->isHidden())
		return;
	GraphImage * graph = _graphTabs->currentGraphView()->getGraphImage();
	GraphConfig * config = graph->getConfig();
	GraphShapeDialog graphShapeDialog = GraphShapeDialog(graph, config, this);
	graphShapeDialog.setModal(false);
	graphShapeDialog.exec();
	_graphTabs->currentGraphView()->scene()->update();
}

void MainWindow::createActions() const
{
	ui.actionClose->setShortcuts(QKeySequence::Close);
	ui.actionClose->setStatusTip(tr("Zamyka program"));
	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(newFile()));
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui.actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(ui.actionClose, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.algorithmList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(runAlgorithm(QListWidgetItem*)));
	connect(ui.actionShape, SIGNAL(triggered()), this, SLOT(openGraphShapeDialog()));
	connect(_graphTabs, SIGNAL(currentChanged(int)), this, SLOT(changeGraphInformation()));
	connect(_graphTabs, SIGNAL(graphChanged()), this, SLOT(updateGraphStatus()));
	connect(ui.actionAddVertex, SIGNAL(triggered(bool)), this, SLOT(checkAddVertexButton(bool)));
	connect(ui.actionAddEdge, SIGNAL(triggered(bool)), this, SLOT(checkAddEdgeButton(bool)));
	connect(ui.actionSelect, SIGNAL(triggered(bool)), this, SLOT(checkSelectionButton(bool)));
	connect(ui.actionPointer, SIGNAL(triggered(bool)), this, SLOT(checkPointerButton(bool)));
	connect(ui.actionRemove, SIGNAL(triggered(bool)), this, SLOT(checkRemoveButton(bool)));
	connect(ui.actionZoom, SIGNAL(triggered(bool)), this, SLOT(checkZoomButton(bool)));
	connect(ui.actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAboutMessageBox()));
}

void MainWindow::checkButton(Tool * tool, QAction * action, bool b)
{
	if (!b) action->setChecked(true);
	Application::Config::Instance().changeCurrentTool(tool);
	uncheckButtons(action);
}

void MainWindow::uncheckButtons(QAction const * action)
{
	for (ActionVector::iterator it = _buttons.begin(); it != _buttons.end(); ++it)
	{
		if (*it != action)
		{
			(*it)->setChecked(false);
		}
	}
}

GraphImage * MainWindow::createGraph(GraphCreateFunc graphFunc, EdgeStrategyCreateFunc strategyFunc)
{
	GraphImage * graph;
	GraphConfig * config = new GraphConfig(
		Application::Config::Instance().DefaultVertexContext()->clone(),
		Application::Config::Instance().DefaultEdgeContext()->clone(),
		Application::Config::Instance().SelectedVertexContext()->clone(),
		Application::Config::Instance().SelectedEdgeContext()->clone());
	graph = graphFunc(config);
	graph->setWeightStrategy(strategyFunc());
	return graph;
}

void MainWindow::checkZoomButton(bool b)
{
	checkButton(&ZoomTool::Instance(), ui.actionZoom, b);
	setCursorForWidget(_graphTabs->currentWidget(), Qt::CrossCursor);
}

void MainWindow::runAlgorithm(QListWidgetItem * item)
{
	// wyci�gnij graf
	GraphImage * graph = _graphTabs->currentGraphView()->getGraphImage();
	// sprawd� czy si� nadaje do u�ytku
	CheckInfo info = _algorithmInfo.checkGraph(graph);
	// je�li tak, to uruchom okno z algorytmem
	if (info.didSucceeded())
	{
		// utworzenie nowego okna
		QDialog * windowPtr = _algorithmInfo.getDialog(graph, item->text());
		windowPtr->setWindowTitle(item->text());
		// zachowanie grafu
		auto copy = graph->clone();
		// wykonanie algorytmu
		windowPtr->exec();
		// zwolnienie okna z pami�ci
		delete windowPtr;
		auto view = _graphTabs->currentGraphView();
		// wyczyszczenie sceny
		view->scene()->clear();
		// przywr�cenie pierwotnego grafu
		view->setGraphImage(copy);
		view->createLabels();
		copy->updateScale(_graphTabs->currentGraphView()->getScale());
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle(Strings::Instance().get(ALGORITHM_ERROR));
		msgBox.setText(Strings::Instance().get(ERROR_MAIN_INFO));
		msgBox.setInformativeText(info.getInformation());
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.exec();
	}
}

void MainWindow::showAboutMessageBox()
{
	QMessageBox box(
		QMessageBox::Information,
		Strings::Instance().get(ABOUT_TITLE),
		Strings::Instance().get(ABOUT_INFO),
		QMessageBox::Ok);
	box.exec();
}

void MainWindow::updateGraphStatus() const
{
	GraphView * graphView = _graphTabs->currentGraphView();
	if (graphView == nullptr)
		return;
	auto graphImage = graphView->getGraphImage();
	if (!graphImage)
		return;
	Graph * graph = graphImage->getGraph();
	QString newStatus = Strings::Instance().get(GRAPH_STATUS)
		.arg(graph->VertexNumber()).arg(graph->EdgeNumber());
	ui.graphTextStatus->setText(newStatus);
}

void MainWindow::changeGraphInformation()
{
	GraphView * graphView = _graphTabs->currentGraphView();
	if (graphView == nullptr)
		return;
	auto graph = graphView->getGraphImage();
	_algorithmInfo.changeState(graph);
	QStringList algorithms = _algorithmInfo.getAlgorithmList();
	ui.algorithmList->clear();
	ui.algorithmList->addItems(algorithms);
	updateGraphStatus();
}