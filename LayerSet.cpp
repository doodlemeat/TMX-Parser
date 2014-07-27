#include "LayerSet.hpp"
#include "SFML\Graphics\RenderTarget.hpp"


LayerSet::LayerSet(const sf::Texture& texture)
	: texture(texture)
{
}


LayerSet::~LayerSet()
{
}

TileQuad::Ptr LayerSet::addTile(sf::Vertex v0, sf::Vertex v1, sf::Vertex v2, sf::Vertex v3)
{
	vertices.push_back(v0);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);

	int i = vertices.size() - 4;
	quads.push_back(std::make_shared<TileQuad>(i, i + 1, i + 2, i + 3));

	return quads.back();
}

void LayerSet::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = &texture;
	target.draw(&vertices[0], (unsigned int)vertices.size(), sf::Quads, states);
}
