#include "GameMap.h"
#include <fstream>
#include <random>
#include <functional>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "ConfigFileInterface.h"

GameMap::GameMap() : 
		x_extent(0), y_extent(0), 
		x_offset(0), y_offset(0), 
		tile_width(0), tile_height(0), 
		display_width(0), display_height(0)
	{
		// create color multi threaded logger
		auto logger = spdlog::get("EngineLogger");
		logger->trace("GameMap() created");
	}

	GameMap::GameMap(const std::string& filename) : GameMap()
	{
		this->LoadMap(filename);
	}

	GameMap::~GameMap()
	{
		// create color multi threaded logger
		auto logger = spdlog::get("EngineLogger");
		logger->trace("GameMap destroyed");

		for (auto it = std::begin(tile_surf); it != std::end(tile_surf); ++it)
		{
			SDL_FreeSurface(*it);
			*it = nullptr;
		}
		for (auto it = std::begin(deco_surf); it != std::end(deco_surf); ++it)
		{
			SDL_FreeSurface(*it);
			*it = nullptr;
		}
		for (auto it = std::begin(over_surf); it != std::end(over_surf); ++it)
		{
			SDL_FreeSurface(*it);
			*it = nullptr;
		}
	}

	void GameMap::LoadMap(const std::string& filename)
	{
		// create color multi threaded logger
		auto logger = spdlog::get("EngineLogger");
		logger->debug("GameMap loaded from: {0}", filename);

		// Root object is a "map"
		ConfigFile::ConfigFileInterface cfi(filename);
		auto maproot = cfi.GetConfigObject("map");

		if (maproot == nullptr)
		{
			logger->error("No map object in file {0}", filename);
			throw std::string("No map in file");
		}

		this->x_extent = atoi(maproot->GetAttribute("width").c_str());
		this->y_extent = atoi(maproot->GetAttribute("height").c_str());
		logger->debug("Map with x_extent {0} and y_extent {1}", this->x_extent, this->y_extent);

		this->tile_width = atoi(maproot->GetAttribute("tilewidth").c_str());
		this->tile_height = atoi(maproot->GetAttribute("tileheight").c_str());
		logger->debug("Tiles of width {0} and height {1}", this->tile_width, this->tile_height);

		for (auto e : maproot->GetChildren())
		{
			logger->info("Map child with name \"{0}\"", e.GetName());
		}
		
		return;
	}

	
	void GameMap::LoadTileImages(std::vector<std::string> image_files)
		// Recieve a map from TileIndex to filename
	{
		// create color multi threaded logger
		auto logger = spdlog::get("EngineLogger");
		logger->trace("GameMap::LoadTileImages(std::map<TileIndex, std::string> image_files)");

		// Store as a map from TileIndex to SDL_Surface*
		logger->debug("Convert vector from filenames to surface pointers");
		for (auto it = std::begin(image_files); it != std::end(image_files); ++it)
		{
			logger->debug("Filename: {0}", *it);
			SDL_Surface* p = IMG_Load(it->c_str());
			this->tile_surf.push_back(p);

			logger->debug("Pointer: {0}", static_cast<void*>(p));
		}
	}

	void GameMap::LoadTileImages(std::string filename)
	{
		// create color multi threaded logger
		auto logger = spdlog::get("EngineLogger");
		logger->debug("TileImages loaded from: {0}", filename);

		// TODO FIXME EEEEEEEEEE
		throw std::string("Unimplemented");

		return;
	}

	void GameMap::LoadTestMap(unsigned int nx, unsigned int ny)
	{
		// create color multi threaded logger
		auto logger = spdlog::get("EngineLogger");
		logger->debug("Creating test map of size: {0},{1}", nx, ny);
		
		this->x_extent = nx;
		this->y_extent = ny;
		this->display_width  = nx;
		this->display_height = ny;
		this->tile_height = 32;
		this->tile_width  = 32;

		this->tile_indices = IndexArray(nx, ny);
		this->deco_indices = IndexArray(nx, ny);
		this->over_indices = IndexArray(nx, ny);
		
		if (this->tile_surf.size() == 0)
		{
			throw std::exception("No tiles loaded, cannot construct test map");
		}

		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution(0, this->tile_surf.size()-1);
		logger->debug("Number of tile surfaces: {0}", this->tile_surf.size());
		auto dice = std::bind(distribution, generator);

		for (unsigned int y = 0; y < ny; ++y)
		{
			for (unsigned int x = 0; x < nx; ++x)
			{
				this->tile_indices.Set(x, y, dice());
			}
			logger->trace("At location {0},{1}, index = {2}", 0, y, tile_indices.At(0, y));
		}
		
	}

	void GameMap::DrawTiles(SDL_Surface * surf)
	{
		this->Draw(surf, this->tile_indices, this->tile_surf);
	}

	void GameMap::DrawOverlay(SDL_Surface * surf)
	{
		this->Draw(surf, this->over_indices, this->over_surf);
	}

	void GameMap::DrawDecorators(SDL_Surface * surf)
	{
		this->Draw(surf, this->deco_indices, this->deco_surf);
	}

	void GameMap::Draw(SDL_Surface* surf, GameMap::IndexArray indices, std::vector<SDL_Surface*> surfaces)
	{
		SDL_Rect source;
		source.x = 0; source.y = 0; source.w = this->tile_width; source.h = this->tile_height;

		SDL_Rect dest;
		dest.w = this->tile_width; dest.h = this->tile_height;

		int tileindex = 0;
		// Iterate over display width and height
		for (unsigned int kx = 0; kx < this->display_width; ++kx)
		{
			for (unsigned int ky = 0; ky < this->display_height; ++ky)
			{
				// Screen location is absolute, no offset
				dest.x = dest.w * kx;
				dest.y = dest.h * ky;

				//std::cerr << "Destination coords: " << dest.x << ',' << dest.y << std::endl;

				// The tile index is relative (uses offset)
				tileindex = indices.At(kx + this->x_offset, ky + this->y_offset);
				SDL_BlitSurface(surfaces[tileindex], &source, surf, &dest);

				//std::cerr << "Any errors?  " << SDL_GetError() << std::endl;
			}
		}
	}

	void GameMap::SetOffset(int off_x, int off_y)
	{
		// Make sure offset won't move us past the edges of the map
		auto max_x_offset = this->x_extent - this->display_width;
		auto min_x_offset = 0;

		// If so, clip to the edges
		if (off_x > max_x_offset)
		{
			this->x_offset = max_x_offset;
		}
		else if (off_x < min_x_offset)
		{
			this->x_offset = min_x_offset;
		}
		else
		{
			this->x_offset = off_x;
		}

		// Same as above for y direction
		auto max_y_offset = this->y_extent - this->display_height;
		auto min_y_offset = 0;

		if (off_y > max_y_offset)
		{
			this->y_offset = max_y_offset;
		}
		else if (off_y < min_y_offset)
		{
			this->y_offset = min_y_offset;
		}
		else
		{
			this->y_offset = off_y;
		}

	}

	void GameMap::DeltaOffset(int dx, int dy)
	{
		// Use code in SetOffset
		auto new_x_offset = this->x_offset + dx;
		auto new_y_offset = this->y_offset + dy;

		this->SetOffset(new_x_offset, new_y_offset);
	}

	GameMap::IndexArray::IndexArray() : stride(0), vec()
	{}

	GameMap::IndexArray::IndexArray(const int width, const int height): stride(width), vec(width*height, 0)
	{
		// create color multi threaded logger
		auto logger = spdlog::get("EngineLogger");
		logger->debug("IndexArray created with size: {0},{1}", width, height);

	}

	GameMap::IndexArray::~IndexArray()
	{
		// create color multi threaded logger
		auto logger = spdlog::get("EngineLogger");
		logger->trace("IndexArray destroyed");

	}

	void GameMap::IndexArray::Set(const int x, const int y, GameMap::TileIndex index)
	{
		this->vec[x + y * stride] = index;
	}

	GameMap::TileIndex GameMap::IndexArray::At(const int x, const int y) const
	{
		return this->vec[x + y * stride];
	}

	const GameMap::TileIndex* GameMap::IndexArray::PointAt(const int x, const int y) const
	{
		auto dp = this->vec.data() + (x + y * stride);
		return dp;
	}