#pragma once
#include "GraphView.h"
#include "Singleton.h"

class Tool;
class DrawingTool;

class Tool
{
public:
	virtual void handleMousePress(GraphView * graph, QPointF const & pos, QList<QGraphicsItem*> const & items) { }
	virtual void handleMouseMove(GraphView * graph, QPoint const & position) {}
	virtual void handleMouseRelease(GraphView * graph) {}
};

class DrawingTool
{
	static Tool * _tool;
	friend class Tool;

public:
	void changeTool(Tool * tool);
	void mousePressed(GraphView * graph, QPointF const & pos, QList<QGraphicsItem*> const & items);
	void mouseMoved(GraphView * graph, QPoint const & position);
	void mouseReleased(GraphView * graph);
};