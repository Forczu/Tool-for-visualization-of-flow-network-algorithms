#include "BlockingFlowAlgoritm.h"
#include "VertexImage.h"
#include "FlowNetwork.h"
#include "EdgeImage.h"
#include "Strings.h"

/// <summary>
/// Ukrywa nadmiarowe wierzcho�ki, kt�re nie pojawi� si� w przep�ywie blokuj�cym.
/// Przeszukiwanie przep�ywu odbywa si� poprzez kraw�dzie, wi�c wierzcho�ki
/// wystarczy ukry�, nie trzeba ich usuwa�.
/// </summary>
/// <param name="residualNewtork">Sie� przep�ywowa.</param>
void BlockingFlowAlgoritm::hideRedundantVertices(FlowNetwork *& residualNewtork)
{
	for (auto vertex : residualNewtork->getVertices())
	{
		int vertexId = vertex->getId() - 1;
		if (_pathMatrix[_sourceId][_targetId] <= _pathMatrix[_sourceId][vertexId] && vertexId != _targetId)
		{
			_currentHiddenVertices.push_back(vertex);
			vertex->hide();
		}
	}
}

/// <summary>
/// Pdkrywa ods�oni�te wierzcho�ki.
/// </summary>
void BlockingFlowAlgoritm::showHiddenVertices()
{
	for (auto vertex : _currentHiddenVertices)
		vertex->show();
	_currentHiddenVertices.clear();
}

/// <summary>
/// Tworzy macierz najkr�tszych dr�g z ka�dego wierzcho�ka do innych.
/// D�ugo�ci dr�g obliczane s� algorytmem Floyda-Warshalla.
/// Poniewa� kraw�dzi w sieci przep�ywowej posiadaj� przepustowo�� i przep�yw,
/// zak�ada si�, �e ka�da droga mi�dzy s�siednimi wierzcho�kami jest r�wna 1.
/// </summary>
/// <param name="newtork">Siec przep�ywowa.</param>
void BlockingFlowAlgoritm::createShortestPathsMatrix(FlowNetwork *& newtork)
{
	const int n = newtork->getHighestVertexId();
	_pathMatrix = FloatMatrix(n);
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
	for (auto coord : newtork->getEdges().keys())
	{
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
/// Usuwa nadmiarowe kraw�dzie, kt�re nie bior� udzia�u w tworzeniu przep�ywu blokuj�cego.
/// </summary>
/// <param name="residualNewtork">Sie� przep�ywowa.</param>
void BlockingFlowAlgoritm::removeRedundantEdges(FlowNetwork *& residualNewtork)
{
	QList<EdgeImage*> edgesToRemove;
	for (auto edge : residualNewtork->getEdges())
	{
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
		for (auto edge : residualNewtork->getEdges())
		{
			if (edge->VertexFrom() == vertex || edge->VertexTo() == vertex)
			{
				residualNewtork->removeEdge(edge);
			}
		}
	}
}

/// <summary>
/// Usuwa wszystkie nadmiarowe elementy grafu, kt�re nie pojawi� si� w przep�ywie blokuj�cym.
/// </summary>
/// <param name="residualNewtork">Sie� przep�ywowa.</param>
/// <returns></returns>
int BlockingFlowAlgoritm::removeRedundantElements(FlowNetwork *& residualNewtork)
{
	_sourceId = residualNewtork->getSourceId() - 1;
	_targetId = residualNewtork->getTargetId() - 1;
	createShortestPathsMatrix(residualNewtork);
	hideRedundantVertices(residualNewtork);
	removeRedundantEdges(residualNewtork);
	float sourceTargetDistance = _pathMatrix[_sourceId][_targetId];
	return sourceTargetDistance == std::numeric_limits<float>::infinity() ? 0 : sourceTargetDistance;
}

QString BlockingFlowAlgoritm::getAugumentingPathFoundMessage(QList<EdgeImage*> const & path, int capacity) const
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

QString BlockingFlowAlgoritm::getResidualNetworkFinishedMessage(int value) const
{
	return Strings::Instance().get(LAYERED_RESIDUAL_NETWORK_CREATED).arg(value);
}
