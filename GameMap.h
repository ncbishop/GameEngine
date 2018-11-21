#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include <string>
#include <vector>
#include <map>
#include <tuple>

	class GameMap
	{
	private:
		class IndexArray;

	public:
		using TileIndex = int;

		GameMap();
		GameMap(const std::string& filename);
		~GameMap();

		// Functions for setting up and changing maps
		//  can be slow
		void LoadMap(const std::string& filename);
		void LoadTileImages(std::vector<std::string> image_files);
		void LoadTileImages(std::string filename);
		void LoadTestMap(unsigned int nx, unsigned int ny);

		// Functions for drawing during operation
		void DrawTiles(SDL_Surface* surf);
		void DrawOverlay(SDL_Surface* surf);
		void DrawDecorators(SDL_Surface* surf);

		// Functions for navigating around maps
		void SetOffset(int off_x, int off_y);
		void DeltaOffset(int dx, int dy);
		std::tuple<int, int> GetOffset() const;

		// Function for manipulating display area
		void SetView(int x_display, int y_display);

	private:
		void Draw(SDL_Surface* surf, IndexArray indices, std::vector<SDL_Surface*> surfaces);

		// Number of tiles in complete map
		unsigned int x_extent, y_extent;

		// Width to display (in units of tiles)
		unsigned int display_width, display_height;

		// How wide are the tiles?
		unsigned int tile_width, tile_height;

		// Where is the view in relation to the extent?
		int x_offset, y_offset;

		class IndexArray
		{
		public:
			IndexArray();
			IndexArray(const int width, const int height);
			~IndexArray();

			void Set(const int x, const int y, TileIndex index);
			TileIndex At(const int x, const int y) const;
			const TileIndex* PointAt(const int x, const int y) const;
						
		private:
			std::vector<TileIndex> vec;
			int stride;
		};

		IndexArray tile_indices;
		IndexArray deco_indices;
		IndexArray over_indices;

		std::vector<SDL_Surface*> tile_surf;
		std::vector<SDL_Surface*> deco_surf;
		std::vector<SDL_Surface*> over_surf;

		
	};
