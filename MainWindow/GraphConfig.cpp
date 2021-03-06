#include "GraphConfig.h"

GraphConfig::GraphConfig(VertexContext * vertexContext, EdgeContext * edgeContext,
	VertexContext * selectedVertexContext, EdgeContext * selectedEdgeContext)
	: _normalVertexContext(vertexContext), _normalEdgeContext(edgeContext),
	_selectedVertexContext(selectedVertexContext), _selectedEdgeContext(selectedEdgeContext)
{
}

GraphConfig::GraphConfig(GraphConfig const & other)
{
	_normalVertexContext = other.NormalVertexContext()->clone();
	_normalEdgeContext = other.NormalEdgeContext()->clone();
	_selectedVertexContext = other.SelectedVertexContext()->clone();
	_selectedEdgeContext = other.SelectedEdgeContext()->clone();
}

GraphConfig::~GraphConfig()
{
	delete _normalVertexContext;
	delete _normalEdgeContext;
	delete _selectedVertexContext;
	delete _selectedEdgeContext;
}
