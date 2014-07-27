#include "TMXParser.hpp"


TMXParser::TileInfo::TileInfo(const sf::IntRect& rect, const sf::Vector2f& size, int tileset_id)
	: size(size),
	tileset_id(tileset_id)
{
	texture_coordinates[0] = sf::Vector2f((float)rect.left, (float)rect.top);
	texture_coordinates[1] = sf::Vector2f((float)(rect.left + rect.width), (float)rect.top);
	texture_coordinates[2] = sf::Vector2f((float)(rect.left + rect.width), (float)(rect.top + rect.height));
	texture_coordinates[3] = sf::Vector2f((float)rect.left, (float)(rect.top + rect.height));
}

TMXParser::TMXParser(const std::string &filepath /*= ""*/)
{
	this->filepath = filepath;
}

TMXParser::~TMXParser()
{

}

bool TMXParser::parseFile(const std::string& file)
{
	std::string levelPath = filepath + file;

	//try to load map file
	pugi::xml_document document;
	pugi::xml_parse_result result = document.load_file(levelPath.c_str());
	if (!result)
	{
		std::cerr << "Failed to open " << levelPath << std::endl;
		std::cerr << "Reason: " << result.description() << std::endl;
		return false;
	}

	//get root node
	pugi::xml_node mapNode = document.child("map");
	if (!mapNode)
	{
		std::cerr << "Map node not found. Map " << levelPath << " not loaded." << std::endl;
		return false;
	}

	//parse map attributes
	if (!parseMapAttributes(mapNode))
	{
		std::cerr << "Failed to parse map attributes. Map " << levelPath << " not loaded." << std::endl;
		return false;
	}

	//parse properties
	parseMapProperties(mapNode);

	//parse tilesets
	if (!parseTilesets(mapNode))
	{
		std::cerr << "Failed to parse tilesets. Map " << levelPath << " not loaded." << std::endl;
		return false;
	}

	//parse layers
	auto currentNode = mapNode.first_child();
	while (currentNode)
	{
		std::string name = currentNode.name();
		if (name == "layer")
		{
			if (!parseLayer(currentNode))
			{
				std::cerr << "Failed to parse " << name << ". Map " << levelPath << " not loaded." << std::endl;
				return false;
			}
		}
		currentNode = currentNode.next_sibling();
	}

	return true;
}

bool TMXParser::parseMapAttributes(const pugi::xml_node& node)
{
	pugi::xml_attribute width_attr = node.attribute("width");
	pugi::xml_attribute height_attr = node.attribute("height");
	pugi::xml_attribute tileWidth_attr = node.attribute("tilewidth");
	pugi::xml_attribute tileHeight_attr = node.attribute("tileheight");

	//validate width attribute
	if (!width_attr)
	{
		std::cerr << "Failed to find the attribute width." << std::endl;
		return false;
	}
	else
	{
		width = width_attr.as_int();
	}

	//validate height attribute
	if (!height_attr)
	{
		std::cerr << "Failed to find the attribute height." << std::endl;
		return false;
	}
	else
	{
		height = height_attr.as_int();
	}

	//validate tilewidth attribute
	if (!tileWidth_attr)
	{
		std::cerr << "Failed to find the attribute tilewidth." << std::endl;
		return false;
	}
	else
	{
		tileWidth = tileWidth_attr.as_int();
	}

	//validate tileheight attribute
	if (!tileHeight_attr)
	{
		std::cerr << "Failed to find the attribute tileheight." << std::endl;
		return false;
	}
	else
	{
		tileHeight = tileHeight_attr.as_int();
	}

	return true;
}

void TMXParser::parseMapProperties(const pugi::xml_node& node)
{
	pugi::xml_node propertiesNode = node.child("properties");
	if (propertiesNode)
	{
		pugi::xml_node propertyNode = propertiesNode.child("property");
		while (propertyNode)
		{
			std::string name = propertyNode.name();
			std::string value = propertyNode.value();
			properties[name] = value;
			propertyNode = propertyNode.next_sibling();
		}
	}
}

bool TMXParser::parseTilesets(const pugi::xml_node& node)
{
	auto tilesetNode = node.child("tileset");
	if (!tilesetNode)
	{
		std::cerr << "No tilesets found." << std::endl;
	}

	while (tilesetNode)
	{
		int tileWidth;
		int tileHeight;

		auto tileWidthAttr = tilesetNode.attribute("tilewidth");
		auto tileHeightAttr = tilesetNode.attribute("tileheight");

		if (!tileWidthAttr || !tileHeightAttr)
		{
			std::cerr << "No tileset data found." << std::endl;
			return false;
		}
		else
		{
			tileWidth = tileWidthAttr.as_int();
			tileHeight = tileHeightAttr.as_int();
		}

		auto imageNode = tilesetNode.child("image");
		auto sourceAttr = imageNode.attribute("source");
		if (!imageNode || !sourceAttr)
		{
			std::cerr << "Missing image data in tileset node." << std::endl;
			return false;
		}

		//create texture from image
		auto path = boost::filesystem::path(sourceAttr.as_string());
		sf::Texture tileset = getAndCacheImage(path.filename().generic_string());
		tilesetTextures.push_back(tileset);

		//slice into tiles
		int columns = tileset.getSize().x / tileWidth;
		int rows = tileset.getSize().y / tileWidth;

		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < columns; x++)
			{
				sf::IntRect rect;
				rect.top = y * tileHeight;
				rect.height = tileHeight;
				rect.left = x * tileWidth;
				rect.width = tileWidth;

				tileInfo.push_back(TileInfo(rect, sf::Vector2f((float)tileWidth, (float)tileHeight), tilesetTextures.size() - 1));
			}
		}

		tilesetNode = tilesetNode.next_sibling("tileset");
	}

	return true;
}

void TMXParser::addSearchPath(const std::string& path)
{
	searchpaths.push_back(path);
}

sf::Texture& TMXParser::getAndCacheImage(const std::string& filename)
{
	//find out if image has already been cached
	for each (const auto& path in searchpaths)
	{
		const auto iter = cachedImages.find(path + filename);
		if (iter != cachedImages.cend())
		{
			return *iter->second;
		}
	}

	//else create a new image
	std::shared_ptr<sf::Texture> newImage = std::make_shared<sf::Texture>();

	//try all paths for the image
	bool foundImage = false;
	std::string path;

	for (const auto& p : searchpaths)
	{
		path = p + filename;
		if (newImage->loadFromFile(path))
		{
			foundImage = true;
			break;
		}
	}

	if (!foundImage)
	{
		std::cerr << "Image " << filename << " was not found. Creating a placeholder image." << std::endl;
		newImage->create(64, 64);
		path = "placeholder";
	}

	cachedImages[path] = newImage;
	return *cachedImages[path];
}

bool TMXParser::parseLayer(const pugi::xml_node& node)
{
	MapLayer layer(TileLayer);

	//set layer name
	auto nameAttr = node.attribute("name");
	if (nameAttr)
	{
		layer.name = nameAttr.as_string();
	}
	else
	{
		layer.name = "undefined";
	}

	//visibility
	auto visibleAttr = node.attribute("visible");
	if (visibleAttr && visibleAttr.as_int() == 0)
	{
		layer.visible = false;
	}

	//parse layer data
	auto dataNode = node.child("data");
	if (!dataNode)
	{
		std::cerr << "Layer " << layer.name << " missing data node." << std::endl;
		return false;
	}

	//only allow csv encoding
	auto encodingAttr = dataNode.attribute("encoding");
	if (encodingAttr)
	{
		std::string encoding = encodingAttr.as_string();
		std::string data = dataNode.text().as_string();
		if (encoding == "csv")
		{
			std::vector<int> tileGids;
			std::stringstream dataStream(data);

			//extract gids
			int i;
			while (dataStream >> i)
			{
				tileGids.push_back(i);
				if (dataStream.peek() == ',')
				{
					dataStream.ignore();
				}
			}

			//create the actual tiles
			int x = 0;
			int y = 0;
			for (std::size_t i = 0; i < tileGids.size(); i++)
			{
				if (tileGids[i] > 0)
					addTileToLayer(layer, x, y, tileGids[i]);
				x++;
				if (x == width)
				{
					x = 0;
					y++;
				}
			}
		}
		else
		{
			std::cerr << "Unsupported encoding " << encoding << "." << std::endl;
			return false;
		}
	}
	else
	{
		std::cerr << "No encoding found." << std::endl;
		return false;
	}

	layers.push_back(layer);
	return true;
}

TileQuad::Ptr TMXParser::addTileToLayer(MapLayer& layer, int x, int y, int gid)
{
	gid--;
	sf::Vertex v0;
	sf::Vertex v1;
	sf::Vertex v2;
	sf::Vertex v3;

	v0.texCoords = tileInfo[gid].texture_coordinates[0] + sf::Vector2f(0.5f, 0.5f);
	v1.texCoords = tileInfo[gid].texture_coordinates[1] + sf::Vector2f(-0.5f, 0.5f);
	v2.texCoords = tileInfo[gid].texture_coordinates[2] + sf::Vector2f(-0.5f, -0.5f);
	v3.texCoords = tileInfo[gid].texture_coordinates[3] + sf::Vector2f(0.5f, -0.5f);

	v0.position = sf::Vector2f((float)(tileWidth * x), (float)(tileHeight * y));
	v1.position = sf::Vector2f((float)(tileWidth * x) + tileInfo[gid].size.x, (float)(tileHeight * y));
	v2.position = sf::Vector2f((float)(tileWidth * x) + tileInfo[gid].size.x, (float)(tileHeight * y) + tileInfo[gid].size.y);
	v3.position = sf::Vector2f((float)(tileWidth * x), (float)(tileHeight * y) + tileInfo[gid].size.y);

	int tileset_id = tileInfo[gid].tileset_id;
	if (layer.sets.find(tileset_id) == layer.sets.end())
	{
		//create new layerset for texture
		layer.sets[tileset_id] = std::make_shared<LayerSet>(tilesetTextures[tileset_id]);
	}

	return layer.sets[tileset_id]->addTile(v0, v1, v2, v3);
}

void TMXParser::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (auto& layer : layers)
	{
		target.draw(layer, states);
	}
}
