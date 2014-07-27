#pragma once

#include <SFML/Graphics.hpp>

#include "pugixml.hpp"
#include <memory>
#include <boost/filesystem.hpp>
#include <array>
#include "MapLayer.hpp"
#include <sstream>
#include "LayerSet.hpp"

class TMXParser : public sf::Drawable
{
private: //definitions
	struct TileInfo
	{
		TileInfo(const sf::IntRect& rect, const sf::Vector2f& size, int tileset_id);
		std::array<sf::Vector2f, 4> texture_coordinates;
		sf::Vector2f size;
		int tileset_id;
	};

	typedef std::map<std::string, std::string> Properties;
	typedef std::map<std::string, std::shared_ptr<sf::Texture>> TextureCache;
	typedef std::vector<TileInfo> TileInfoContainer;
	typedef std::vector<sf::Texture> TilesetTextures;
	typedef std::vector<MapLayer> Layers;

public: //public member functions
	TMXParser(const std::string &filepath = "");
	~TMXParser();

	bool parseFile(const std::string& file);
	void addSearchPath(const std::string& path);

private: //private member functions
	bool parseMapAttributes(const pugi::xml_node& node);
	void parseMapProperties(const pugi::xml_node& node);
	bool parseTilesets(const pugi::xml_node& node);
	sf::Texture& getAndCacheImage(const std::string& filename);
	bool parseLayer(const pugi::xml_node& node);
	TileQuad::Ptr addTileToLayer(MapLayer& layer, int x, int y, int gid);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private: //private member variables
	std::string filepath;
	std::string lastFile;
	int width;
	int height;
	int tileWidth;
	int tileHeight;
	Properties properties;
	TextureCache cachedImages;
	TileInfoContainer tileInfo;
	TilesetTextures tilesetTextures;
	Layers layers;
	std::vector<std::string> searchpaths;
};
