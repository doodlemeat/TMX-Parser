#pragma once
#include <memory>
#include <array>

class TileQuad
{
public:
	typedef std::shared_ptr<TileQuad> Ptr;

	TileQuad(int i0, int i1, int i2, int i3);
	~TileQuad();

private:
	std::array<int, 4> indicies;
};

