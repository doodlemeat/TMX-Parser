#pragma once

#include <string>
#include <map>
#include <memory>
#include "LayerSet.hpp"

enum MapLayerType
{
	TileLayer,
	ObjectGroup,
	ImageLayer
};

class MapLayer: public sf::Drawable
{
public:
	MapLayer(MapLayerType layerType);
	~MapLayer();

	const MapLayerType& getLayerType();

public:
	std::string name;
	std::map<int, std::shared_ptr<LayerSet>> sets;
	MapLayerType type;
	bool visible;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

