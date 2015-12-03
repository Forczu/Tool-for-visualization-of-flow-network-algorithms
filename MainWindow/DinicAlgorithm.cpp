#include "DinicAlgorithm.h"
#include "EdgeImage.h"
#include "VertexImage.h"
#include "Strings.h"
#include "FlowNetworkAlgorithmWindow.h"

DinicAlgorithm * DinicAlgorithm::getInstance()
{
	return new DinicAlgorithm;
}

int DinicAlgorithm::makeResidualNetwork(FlowNetwork * network, FlowNetwork *& residualNewtork)
{
	for (auto vertex : _currentHiddenVertices)
		vertex->show();
	_currentHiddenVertices.clear();
	FlowNetworkAlgorithm::makeResidualNetwork(network, residualNewtork);
	int result = removeRedundantElements(residualNewtork);
	hideRedundantVertices(residualNewtork);
	return result;
}

QList<EdgeImage*> DinicAlgorithm::findAugumentingPath(FlowNetwork * residualNetwork, int & capacity)
{
	QList<EdgeImage*> path = FlowNetworkAlgorithm::findAugumentingPath(residualNetwork, capacity);
	_usedEdges.append(path);
	return path;
}

void DinicAlgorithm::increaseFlow(FlowNetwork *& network, QList<EdgeImage*> const & path, int increase)
{
	FlowNetworkAlgorithm::increaseFlow(network, path, increase);
} 

/// <summary>
/// Usuwa wszystkie nadmiarowe elementy grafu, kt�re nie pojawi� si� w przep�ywie blokuj�cym.
/// </summary>
/// <param name="residualNewtork">Sie� przep�ywowa.</param>
/// <returns></returns>
int DinicAlgorithm::removeRedundantElements(FlowNetwork *& residualNewtork)
{
	_sourceId = residualNewtork->getSourceId() - 1;
	_targetId = residualNewtork->getTargetId() - 1;
	createShortestPathsMatrix(residualNewtork);
	hideRedundantVertices(residualNewtork);
	removeRedundantEdges(residualNewtork);
	float sourceTargetDistance = _pathMatrix[_sourceId][_targetId];
	return sourceTargetDistance == std::numeric_limits<float>::infinity() ? 0 : sourceTargetDistance;
}

/// <summary>
/// Usuwa nadmiarowe kraw�dzie, kt�re nie bior� udzia�u w tworzeniu przep�ywu blokuj�cego.
/// </summary>
/// <param name="residualNewtork">Sie� przep�ywowa.</param>
void DinicAlgorithm::removeRedundantEdges(FlowNetwork *& residualNewtork)
{
	QList<EdgeImage*> edgesToRemove;
	for (auto item : residualNewtork->getEdges())
	{
		EdgeImage * edge = item.second;
		int from = edge->VertexFrom()->getId() - 1;
		int to = edge->VertexTo()->getId() - 1;
		int first = _pathMatrix[_sourceId][to];
		int second = _pathMatrix[_sourceId][from];
		if (first <= second)
			edgesToRemove.push_back(edge);
	}
	for (auto edge : edgesToRemove)
	{
		residualNewtork->removeEdge(edge);
	}
	for (auto vertex : _currentHiddenVertices)
	{
		for (auto item : residualNewtork->getEdges())
		{
			EdgeImage * edge = item.second;
			if (edge->VertexFrom() == vertex || edge->VertexTo() == vertex)
			{
				residualNewtork->removeEdge(edge);
			}
		}
	}
}

/// <summary>
/// Tworzy macierz najkr�tszych dr�g z ka�dego wierzcho�ka do innych.
/// D�ugo�ci dr�g obliczane s� algorytmem Floyda-Warshalla.
/// Poniewa� kraw�dzi w sieci przep�ywowej posiadaj� przepustowo�� i przep�yw,
/// zak�ada si�, �e ka�da droga mi�dzy s�siednimi wierzcho�kami jest r�wna 1.
/// </summary>
/// <param name="newtork">Siec przep�ywowa.</param>
void DinicAlgorithm::createShortestPathsMatrix(FlowNetwork *& newtork)
{
	const int n = newtork->vertexNumber();
	_pathMatrix = std::vector<std::vector<float>>(n);
	for (int i = 0; i < n; ++i)
	{
		_pathMatrix[i].resize(n);
	}
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			if (i != j)
				_pathMatrix[i][j] = std::numeric_limits<float>::infinity();
		}
	}
	std::pair<int, int> coord;
	for (auto item : newtork->getEdges())
	{
		coord = item.first;
		_pathMatrix[coord.first - 1][coord.second - 1] = 1.0f;
	}
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			if (_pathMatrix[i][k] != std::numeric_limits<float>::infinity())
			{
				for (int j = 0; j < n; ++j)
				{
					float first = _pathMatrix[i][j];
					float second = _pathMatrix[i][k] + _pathMatrix[k][j];
					_pathMatrix[i][j] = std::min(first, second);
				}
			}
		}
	}
}

/// <summary>
/// Ukrywa nadmiarowe wierzcho�ki, kt�re nie pojawi� si� w przep�ywie blokuj�cym.
/// Przeszukiwanie przep�ywu odbywa si� poprzez kraw�dzie, wi�c wierzcho�ki
/// wystarczy ukry�, nie trzeba ich usuwa�.
/// </summary>
/// <param name="residualNewtork">Sie� przep�ywowa.</param>
void DinicAlgorithm::hideRedundantVertices(FlowNetwork *& residualNewtork)
{
	for (auto item : residualNewtork->getVertices())
	{
		VertexImage * vertex = item.second;
		int vertexId = vertex->getId() - 1;
		if (_pathMatrix[_sourceId][_targetId] <= _pathMatrix[_sourceId][vertexId] && vertexId != _targetId)
		{
			_currentHiddenVertices.push_back(vertex);
			vertex->hide();
		}
	}
}

void DinicAlgorithm::addEdgeToPath(QList<EdgeImage*> & possibleEdges, EdgeImage * edge, VertexImage * currentVertex, VertexImage * source,
	QList<VertexImage*> const & visitedVertices, QList<VertexImage*> const & rejectedVertices)
{
	if (edge->VertexFrom() == currentVertex &&
		edge->VertexTo() != source &&
		!visitedVertices.contains(edge->VertexTo()) &&
		!rejectedVertices.contains(edge->VertexTo()) &&
		!_usedEdges.contains(edge))
	{
		possibleEdges.push_back(edge);
	}
}

void DinicAlgorithm::acceptNextStep(FlowNetworkAlgorithmWindow * window)
{
	window->visitDinicNextStep(this);
}

void DinicAlgorithm::acceptFindAugumentingPath(FlowNetworkAlgorithmWindow * window)
{
	window->visitDinicFindAugumentingPath(this);
}

QString DinicAlgorithm::resaidualNetworkFinishedMessage(int value)
{
	return Strings::Instance().get(LAYERED_RESIDUAL_NETWORK_CREATED).arg(value);
}

QString DinicAlgorithm::augumentingPathFoundMessage(QList<EdgeImage*> const & path, int capacity)
{
	QString numbers;
	numbers.push_back(QString::number(path.first()->VertexFrom()->getId()) + ' ');
	for (EdgeImage * edge : path)
	{
		edge->setSelected(true);
		numbers.push_back(QString::number(edge->VertexTo()->getId()) + ' ');
	}
	QString message = Strings::Instance().get(BLOCKING_FLOW_AUGUMENTING_PATH)
		.arg(numbers).arg(capacity);
	return message;
}