#pragma once
#include "SFML\Graphics\Drawable.hpp"
#include "SFML\Graphics\Vertex.hpp"
#include <vector>
#include "TileQuad.hpp"

//drawable layer composed of vertiex representing a set of tiles on a layer
class LayerSet: public sf::Drawable
{
public:
	LayerSet(const sf::Texture& texture);
	~LayerSet();

	TileQuad::Ptr addTile(sf::Vertex v0, sf::Vertex v1, sf::Vertex v2, sf::Vertex v3);
	
private:
	const sf::Texture& texture;
	std::vector<TileQuad::Ptr> quads;
	std::vector<sf::Vertex> vertices;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

