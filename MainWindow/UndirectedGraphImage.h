#pragma once
#include "GraphImage.h"

class UndirectedGraphImage : public GraphImage
{
public:
	explicit UndirectedGraphImage(GraphConfig * config);
	UndirectedGraphImage(UndirectedGraphImage const & graph);
	~UndirectedGraphImage();

	EdgeImage * addEdge(int vertexId1, int vertexId2, int weight, EdgeType type, int flow = 0, float scale = 1.0f) override;
	static GraphImage * getInstance(GraphConfig * config)
	{
		return new UndirectedGraphImage(config);
	}
	GraphImage * clone() const override
	{
		return new UndirectedGraphImage(*this);
	}

protected:
	void updateVerticesDegree(VertexImage * vertexFrom, VertexImage * vertexTo);
	EdgeImage * createFullEdgeImage(Edge * edge, EdgeType type, int weight = 0, int flow = 0, float scale = 1.0f) override;
};

