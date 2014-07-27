#include "MapLayer.hpp"
#include "SFML\Graphics\RenderTarget.hpp"


MapLayer::MapLayer(MapLayerType layerType)
	: type(layerType),
	visible(true)
{
}


MapLayer::~MapLayer()
{
}

const MapLayerType& MapLayer::getLayerType()
{
	return type;
}

void MapLayer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (!visible) return;
	for (auto &set : sets)
	{
		target.draw(*set.second, states);
	}
}
