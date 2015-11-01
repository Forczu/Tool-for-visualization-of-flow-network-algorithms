#include "VertexAddTool.h"

VertexAddTool * VertexAddTool::_pInstance = 0;

void VertexAddTool::handleMousePress(GraphView * graph, QPointF const & pos, QList<QGraphicsItem*> const & items)
{
	graph->buildVertex(pos, items);
}

