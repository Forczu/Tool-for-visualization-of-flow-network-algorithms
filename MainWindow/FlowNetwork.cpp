#include "FlowNetwork.h"
#include "QGraphicsSceneEvent"
#include "VertexImage.h"
#include "TextItem.h"
#include "EdgeImage.h"


FlowNetwork::FlowNetwork(GraphConfig * config)
: DirectedGraphImage(config), _source(0), _target(0)
{
	setAcceptedMouseButtons(Qt::MouseButton::RightButton);
	init();
}


FlowNetwork::FlowNetwork(FlowNetwork const & network)
: DirectedGraphImage(network)
{
	_source = network._source;
	_target = network._target;
	_labelFont = network._labelFont;
	_sourceLabel = new TextItem(*network._sourceLabel);
	_targetLabel = new TextItem(*network._targetLabel);
}

FlowNetwork::~FlowNetwork()
{
	delete _sourceLabel;
	delete _targetLabel;
}

void FlowNetwork::init()
{
	createFont();
	createLabel(_sourceLabel, "s", Qt::AlignLeft);
	createLabel(_targetLabel, "t", Qt::AlignRight);
}

void FlowNetwork::createFont()
{
	_labelFont.setBold(true);
	_labelFont.setItalic(true);
	_labelFont.setPointSize(48);
	_labelFont.setFamily(QString("Calibri"));
}

void FlowNetwork::createLabel(TextItem *& label, QString const & text, Qt::AlignmentFlag align)
{
	label = new TextItem(text);
	label->replaceFont(_labelFont);
	label->setAlignment(align);
	label->hide();
}

/// <summary>
/// Sprawdzenie warunku zachowania przepustowo�ci.
/// �adna kraw�d� nie mo�e mie� przep�ywu wi�kszego od przepustowo�ci.
/// </summary>
/// <returns>True, je�eli za�o�enia s� spe�nione, false je�li nie</returns>
bool FlowNetwork::checkCapacityCondition()
{
	EdgeImage * edge;
	return std::all_of(_edgeMap.begin(), _edgeMap.end(), [&](EdgeImagePair const & item)
	{
		edge = item.second;
		if (edge->getFlow() <= edge->getCapacity())
			return true;
		return false;
	});
}


/// <summary>
/// Sprawdzenie warunku zachowania przep�ywu.
/// Suma przep�yw�w wp�ywaj�cych do wierzcho�ka, nie b�d�cego �r�d�em ani uj�ciem, musi by� r�wna sumie wyp�ywaj�cych z niego.
/// </summary>
/// <returns>True, je�eli za�o�enia s� spe�nione, false je�li nie</returns>
bool FlowNetwork::checkFlowPreservation()
{
	VertexImage * vertex;
	EdgeImage * edge;
	return std::all_of(_vertexMap.begin(), _vertexMap.end(), [&](VertexImagePair const & item)
	{
		vertex = item.second;
		if (_source == vertex->getId() || _target == vertex->getId())
			return true;
		std::vector<EdgeImage*> inEdges, outEdges;
		std::for_each(_edgeMap.begin(), _edgeMap.end(), [&](EdgeImagePair const & edgeItem)
		{
			edge = edgeItem.second;
			if (edge->VertexTo() == vertex)
				inEdges.push_back(edge);
			if (edge->VertexFrom() == vertex)
				outEdges.push_back(edge);
		});
		int inSum = 0, outSum = 0;
		std::for_each(inEdges.begin(), inEdges.end(), [&](EdgeImage * edge)
		{
			inSum += edge->getFlow();
		});
		std::for_each(outEdges.begin(), outEdges.end(), [&](EdgeImage * edge)
		{
			outSum += edge->getFlow();
		});
		if (inSum != outSum)
			return false;
		return true;
	});
}

/// <summary>
/// Sprawdzenie czy budowa sieci spe�nia za�o�enie, �e z ka�dego wierzcho�ka, nie b�d�cego �r�d�em ani uj�ciem,
/// istnieje �cie�ka prowadz�ca do �r�d�a.
/// </summary>
/// <returns>True, je�eli struktura jest w porz�dku, je�li nie to false</returns>
bool FlowNetwork::checkStructure()
{
	VertexImage * vertex;
	EdgeImage * edge;
	return std::all_of(_vertexMap.begin(), _vertexMap.end(), [&](VertexImagePair const & item)
	{
		vertex = item.second;
		if (_source == vertex->getId() || _target == vertex->getId())
			return true;
		std::vector<EdgeImage*> edges;
		auto it = std::find_if(_edgeMap.begin(), _edgeMap.end(), [&](EdgeImagePair const & edgeItem)
		{
			edge = edgeItem.second;
			if (edge->VertexFrom() == vertex && edge->VertexTo() != _vertexMap[getSource()])
				return true;
			return false;
		});
		return it != _edgeMap.end();
	});
}

void FlowNetwork::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	auto items = childItems();
	for (QGraphicsItem * item : items)
	{
		QRectF itemRect = item->mapRectToScene(item->boundingRect());
		if (itemRect.contains(event->scenePos()))
		{
			VertexImage * vImg = dynamic_cast<VertexImage*>(item);
			if (NULL != vImg)
			{
				QString source = "Oznacz jako �r�d�o", target = "Oznacz jako uj�cie";
				QMenu menu;
				QAction * setAsSource = menu.addAction(source);
				QAction * setAsTarget = menu.addAction(target);
				QAction * selectedAction = menu.exec(event->screenPos());
				if (selectedAction == setAsSource)
					markSource(vImg->getVertex()->Id(), vImg);
				else if (selectedAction == setAsTarget)
					markTarget(vImg->getVertex()->Id(), vImg);
				delete setAsSource;
				delete setAsTarget;
			}
		}
	}
}

void FlowNetwork::markSource(int id, VertexImage * vertex)
{
	_source = id;
	_sourceLabel->setPos(
		- vertex->Context()->Size() - _sourceLabel->boundingRect().width() / 2.0f,
		- vertex->Context()->Size() - _sourceLabel->boundingRect().height());
	_sourceLabel->setParentItem(vertex);
	if (!_sourceLabel->isVisible()) 
		_sourceLabel->show();
}

void FlowNetwork::markSource(int id)
{
	VertexImage * vertex = _vertexMap[id];
	markSource(id, vertex);
}

void FlowNetwork::markTarget(int id, VertexImage * vertex)
{
	_target = id;
	_targetLabel->setPos(
		+ vertex->Context()->Size() - _targetLabel->boundingRect().width() / 2.0f,
		- vertex->Context()->Size() - _targetLabel->boundingRect().height());
	_targetLabel->setParentItem(vertex);
	if (!_targetLabel->isVisible())
		_targetLabel->show();
}

void FlowNetwork::markTarget(int id)
{
	VertexImage * vertex = _vertexMap[id];
	markTarget(id, vertex);
}

FlowNetwork * FlowNetwork::makeResidualNetwork()
{
	FlowNetwork * residualNetwork = new FlowNetwork(*this);
	residualNetwork->setParentItem(this);
	residualNetwork->moveBy(500, 0);
	return residualNetwork;
}
