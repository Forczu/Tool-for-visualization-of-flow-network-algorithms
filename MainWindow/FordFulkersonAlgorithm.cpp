#include "FordFulkersonAlgorithm.h"
#include "FlowNetworkAlgorithmWindow.h"
#include "FlowNetwork.h"
#include "Typedefs.h"
#include "Graph.h"
#include "Edge.h"
#include "EdgeImage.h"
#include "Vertex.h"
#include "VertexImage.h"
#include "WeightedEdgeStrategy.h"

FordFulkersonAlgorithm * FordFulkersonAlgorithm::getInstance()
{
	return new FordFulkersonAlgorithm;
}

void FordFulkersonAlgorithm::run(GraphImage * graph)
{
}

FlowNetwork * FordFulkersonAlgorithm::makeResidualNetwork(FlowNetwork * network)
{
	FlowNetwork * residualNewtork = new FlowNetwork(network->getConfig()->clone());
	residualNewtork->setWeightStrategy(WeightedEdgeStrategy::getInstance());
	// skopiuj wierzcho�ki
	VertexImageMap vertices = network->getVertices();
	for (VertexImageMap::iterator it = vertices.begin(); it != vertices.end(); ++it)
	{
		VertexImage * vertex = (*it).second;
		int id = vertex->getId();
		QPointF position = vertex->scenePos();
		auto points = vertex->getPoints();
		residualNewtork->addVertex(id, position, points);
	}
	// analiza kraw�dzi i utworzenie sieci residualnej
	EdgeImageMap edges = network->getEdges();
	QList<EdgeImage*> visitedNeighbours;
	for (EdgeImageMap::iterator it = edges.begin(); it != edges.end(); ++it)
	{
		EdgeImage * edge = (*it).second;
		int capacity = edge->getCapacity();
		int flow = edge->getFlow();
		int vertexFromId = edge->VertexFrom()->getId();
		int vertexToId = edge->VertexTo()->getId();
		int residualCapacity = capacity - flow;
		EdgeImage * neighbor;
		if (edge->hasNeighbor() && !visitedNeighbours.contains(neighbor = network->edgeAt(vertexToId, vertexFromId)))
		{
			visitedNeighbours.push_back(neighbor);
			visitedNeighbours.push_back(edge);
			int neighborFlow = neighbor->getFlow();
			int neighborCapacity = neighbor->getCapacity();
			residualCapacity = capacity - flow + neighborFlow;
			int neighborResidualCapacity = neighborCapacity - neighborFlow + flow;
			if (residualCapacity != 0)
				residualNewtork->addEdge(vertexFromId, vertexToId, residualCapacity, EdgeType::StraightLine);
			if (neighborResidualCapacity != 0)
				residualNewtork->addEdge(vertexToId, vertexFromId, neighborResidualCapacity, EdgeType::StraightLine);
		}
		else
		{
			if (flow != 0)
				residualNewtork->addEdge(vertexToId, vertexFromId, flow, EdgeType::StraightLine);
			if (residualCapacity != 0)
				residualNewtork->addEdge(vertexFromId, vertexToId, residualCapacity, EdgeType::StraightLine);
		}
	}
	residualNewtork->markSource(network->getSource());
	residualNewtork->markTarget(network->getTarget());
	return residualNewtork;
}

QList<EdgeImage*> FordFulkersonAlgorithm::findAugumentingPath(FlowNetwork * residualNetwork, int & capacity)
{
	QList<EdgeImage*> augumentingPath;
	int sourceId = residualNetwork->getSource();
	VertexImage * source = residualNetwork->vertexAt(sourceId);
	EdgeImageMap edges = residualNetwork->getEdges();
	bool augumentationExists = std::any_of(edges.begin(), edges.end(), [&](EdgeImagePair item)
	{
		EdgeImage * edge = item.second;
		if (edge->VertexFrom() == source)
			return true;
		return false;
	});
	if (!augumentationExists)
		return augumentingPath;
	srand(time(NULL));



	bool finished = false;
	VertexImage * currentVertex = source;
	VertexImage * target = residualNetwork->vertexAt(residualNetwork->getTarget());
	int currentMaxCapacity = INT_MAX;
	QList<VertexImage*> visitedVertices;
	QList<VertexImage*> rejectedVertices;
	EdgeImage * lastEdge = nullptr;
	while (!finished)
	{
		QList<EdgeImage*> possibleEdges;
		for (auto item : edges)
		{
			EdgeImage * edge = item.second;
			if (edge->VertexFrom() == currentVertex &&
				edge->VertexTo() != source &&
				!visitedVertices.contains(edge->VertexTo()) &&
				!rejectedVertices.contains(edge->VertexTo()))
			{
				possibleEdges.push_back(edge);
			}
		}
		if (possibleEdges.empty())
		{
			if (lastEdge == nullptr)
			{
				currentMaxCapacity = 0;
				finished = true;
				break;
			}
			else
			{
				rejectedVertices.push_back(currentVertex);
				augumentingPath.pop_back();
				if (!augumentingPath.empty())
				{
					lastEdge = augumentingPath.last();
					currentVertex = lastEdge->VertexTo();
				}
				else
				{
					lastEdge = nullptr;
					currentVertex = source;
				}
				continue;
			}
		}
		EdgeImage * chosenEdge = possibleEdges.at(rand() % possibleEdges.size());
		int newCapacity = chosenEdge->getCapacity();
		currentMaxCapacity = std::min(newCapacity, currentMaxCapacity);
		VertexImage * nextVertex = chosenEdge->VertexTo();
		visitedVertices.push_back(nextVertex);
		augumentingPath.push_back(chosenEdge);
		currentVertex = nextVertex;
		lastEdge = chosenEdge;
		if (currentVertex == target)
			finished = true;
	}
	capacity = currentMaxCapacity;
	_currentMaxFlow += capacity;
	return augumentingPath;
}

void FordFulkersonAlgorithm::increaseFlow(QList<EdgeImage*> const & path)
{

}
