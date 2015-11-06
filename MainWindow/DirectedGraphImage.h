#pragma once
#include "GraphImage.h"

class DirectedGraphImage : public GraphImage
{
public:
	DirectedGraphImage(GraphConfig * config);
	DirectedGraphImage(DirectedGraphImage const & graph);

	~DirectedGraphImage();

	EdgeImage * addEdge(int vertexId1, int vertexId2, int weight, EdgeType type, int flow = 0) override;

protected:
	void updateVerticesDegree(VertexImage * vertexFrom, VertexImage * vertexTo);
	EdgeImage * createFullEdgeImage(Edge * edge, EdgeType type, int weight = 0, int flow = 0) override;

};

