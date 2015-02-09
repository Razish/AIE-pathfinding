#include <list>

#include "XSClient/XSClient.h"
#include "XSCommon/XSCommon.h"
#include "XSCommon/XSConsole.h"
#include "XSCommon/XSCvar.h"
#include "XSCommon/XSFile.h"
#include "XSCommon/XSString.h"
#include "XSCommon/XSColours.h"
#include "XSRenderer/XSInternalFormat.h"
#include "XSRenderer/XSMaterial.h"
#include "XSRenderer/XSShaderProgram.h"
#include "XSRenderer/XSTexture.h"
#include "XSRenderer/XSVertexAttributes.h"
#include "XSRenderer/XSView.h"

namespace XS {

	namespace ClientGame {

		enum class PathfindingAlgorithm {
			AStar
		};

		enum class TileType {
			Blank,
			Wall,
			Start,
			Goal
		};
		
		enum Direction {
			NorthWest,
			North,
			NorthEast,
			East,
			SouthEast,
			South,
			SouthWest,
			West,
			NUM_DIRECTIONS
		};
		
		struct Tile {
			uint32_t	x, y;
			// a cost of -1 means it is an unmoveable direction (i.e. off the map)
			// the lower the cost, the better the move
			int32_t		weights[NUM_DIRECTIONS];
			TileType	type;

#if 0
			inline bool operator==( const Node &rhs ) {
				if ( x == rhs.x && y == rhs.y ) {
					return true;
				}
				return false;
			}
#endif
		};

		static constexpr size_t dimensions[2] = { 32u, 18u };

		static struct GameState {
			Tile tiles[dimensions[0]][dimensions[1]];

			// tiles are added to the open list if it's to be explored
			// when a tile is determined to be unsuitable, it's moved to the closed list and never checked again
			std::list<Tile *> openList, closedList;
		} state = {};
		
		static Tile *GetTile( const uint32_t x, const uint32_t y ) {
			return &state.tiles[x][y];
		}

		static TileType GetTileType( const uint32_t x, const uint32_t y ) {
			return state.tiles[x][y].type;
		}

		static Renderer::View *sceneView = nullptr;

		static void RenderScene( void ) {
			// the view is already bound
		}

		static void GenerateMaze( void ) {
			// initialise the tiles
			for ( size_t x = 0u; x < dimensions[0]; x++ ) {
				for ( size_t y = 0u; y < dimensions[1]; y++ ) {
					state.tiles[x][y].x = x;
					state.tiles[x][y].y = y;
					state.tiles[x][y].type = TileType::Blank;
				//	state.tiles[x][y].weights[North] = 0;
				//	state.tiles[x][y].weights[East] = 0;
				//	state.tiles[x][y].weights[South] = 0;
				//	state.tiles[x][y].weights[West] = 0;
				}
			}

			// surround the map in a box
			for ( size_t x = 0u; x < dimensions[0]; x++ ) {
				state.tiles[x][0].type = TileType::Wall;
			//	state.tiles[x][0].weights[North] = -1; // unmoveable

				state.tiles[x][dimensions[1] - 1].type = TileType::Wall;
			//	state.tiles[x][dimensions[1] - 1].weights[South] = -1; // unmoveable
			}
			for ( size_t y = 0u; y < dimensions[1]; y++ ) {
				state.tiles[0][y].type = TileType::Wall;
			//	state.tiles[0][y].weights[East] = -1;

				state.tiles[dimensions[0] - 1][y].type = TileType::Wall;
			//	state.tiles[dimensions[0] - 1][y].weights[West] = -1;
			}
			
			// randomly place the start position with 2 cell padding
			int32_t startX = 0u;
			int32_t startY = 0u;
			while ( true ) {
				startX = 1 + (rand() % (dimensions[0] - 1));
				startY = 1 + (rand() % (dimensions[1] - 1));
				if ( startX < 3 || startX >= dimensions[0] - 3 ) {
					continue;
				}
				if ( startY < 3 || startY >= dimensions[1] - 3 ) {
					continue;
				}
				if ( GetTileType( startX, startY ) == TileType::Blank ) {
					break;
				}
			}
			state.tiles[startX][startY].type = TileType::Start;
			Tile *neighbours[NUM_DIRECTIONS] = {};
			neighbours[NorthWest]	= GetTile( startX - 1, startY - 1 );
			neighbours[North]		= GetTile( startX    , startY - 1 );
			neighbours[NorthEast]	= GetTile( startX + 1, startY - 1 );
			neighbours[East]		= GetTile( startX + 1, startY     );
			neighbours[SouthEast]	= GetTile( startX + 1, startY + 1 );
			neighbours[South]		= GetTile( startX    , startY + 1 );
			neighbours[SouthWest]	= GetTile( startX - 1, startY + 1 );
			neighbours[West]		= GetTile( startX - 1, startY     );

			// randomly surround the start tile
			int32_t side = rand() % 4;
			neighbours[NorthWest]->type	= TileType::Wall;
			neighbours[North    ]->type	= (side == 0) ? TileType::Blank : TileType::Wall;
			neighbours[NorthEast]->type	= TileType::Wall;
			neighbours[East     ]->type	= (side == 1) ? TileType::Blank : TileType::Wall;
			neighbours[SouthEast]->type	= TileType::Wall;
			neighbours[South    ]->type	= (side == 2) ? TileType::Blank : TileType::Wall;
			neighbours[SouthWest]->type	= TileType::Wall;
			neighbours[West     ]->type	= (side == 3) ? TileType::Blank : TileType::Wall;



			// randomly place the end position with 2 cell padding, at-least 3 cells away from the start
			int32_t goalX = 0u;
			int32_t goalY = 0u;
			while ( true ) {
				goalX = 1 + (rand() % (dimensions[0] - 1));
				goalY = 1 + (rand() % (dimensions[1] - 1));
				if ( goalX < 3 || goalX >= dimensions[0] - 3 ) {
					continue;
				}
				if ( goalY < 3 || goalY >= dimensions[1] - 3 ) {
					continue;
				}
				if ( std::abs( goalX - startX ) + std::abs( goalY - startY ) < 6 ) {
					continue;
				}
				if ( GetTileType( goalX, goalY ) == TileType::Blank ) {
					break;
				}
				
			}
			state.tiles[goalX][goalY].type = TileType::Goal;
			neighbours[NorthWest]	= GetTile( goalX - 1, goalY - 1 );
			neighbours[North]		= GetTile( goalX    , goalY - 1 );
			neighbours[NorthEast]	= GetTile( goalX + 1, goalY - 1 );
			neighbours[East]		= GetTile( goalX + 1, goalY     );
			neighbours[SouthEast]	= GetTile( goalX + 1, goalY + 1 );
			neighbours[South]		= GetTile( goalX    , goalY + 1 );
			neighbours[SouthWest]	= GetTile( goalX - 1, goalY + 1 );
			neighbours[West]		= GetTile( goalX - 1, goalY     );

			// randomly surround the goal tiles
			side = rand() % 4;
			neighbours[NorthWest]->type	= TileType::Wall;
			neighbours[North    ]->type	= (side == 0) ? TileType::Blank : TileType::Wall;
			neighbours[NorthEast]->type	= TileType::Wall;
			neighbours[East     ]->type	= (side == 1) ? TileType::Blank : TileType::Wall;
			neighbours[SouthEast]->type	= TileType::Wall;
			neighbours[South    ]->type	= (side == 2) ? TileType::Blank : TileType::Wall;
			neighbours[SouthWest]->type	= TileType::Wall;
			neighbours[West     ]->type	= (side == 3) ? TileType::Blank : TileType::Wall;

			// partially surround the start/end positions
			for ( size_t iteration = 0u; iteration < 5; iteration++ ) {
				
			}

			// continuously branch off randomly from the surrounding walls using a weighted flood-fill
			//	5 iterations?
		}
		
		static void PrintMaze( void ) {
			for ( size_t x = 0u; x < dimensions[0]; x++ ) {
				for ( size_t y = 0u; y < dimensions[1]; y++ ) {
					switch( GetTileType( x, y ) ) {

					case TileType::Blank: {
						console.Print( "  " );
					} break;

					case TileType::Wall: {
						console.Print( "# " );
					} break;

					case TileType::Start: {
						console.Print( "S " );
					} break;

					case TileType::Goal: {
						console.Print( "G " );
					} break;

					}
				}
				console.Print( "\n" );
			}
		}
		
		static void PathfindingStep( PathfindingAlgorithm algo = PathfindingAlgorithm::AStar ) {
			console.Print( "Running pathfinding step\n" );
		}

		void Init( void ) {
			const uint32_t width = Cvar::Get( "vid_width" )->GetInt();
			const uint32_t height = Cvar::Get( "vid_height" )->GetInt();
			sceneView = new Renderer::View( width, height, RenderScene );

			GenerateMaze();
			PrintMaze();
		}

		void RunFrame( void ) {
			static double lastTime = 0.0;
			const double currentTime = Client::GetElapsedTime();
			if ( lastTime < currentTime - 750 ) {
				// step every 750ms
				PathfindingStep( PathfindingAlgorithm::AStar );
				lastTime = currentTime;
			}
		}

		void DrawFrame( void ) {
			sceneView->Bind();

			const uint32_t screenWidth = Cvar::Get( "vid_width" )->GetInt();
			const uint32_t screenHeight = Cvar::Get( "vid_height" )->GetInt();

			const real32_t tileWidth = screenWidth / dimensions[0];
			const real32_t tileHeight = screenHeight / dimensions[1];

			for ( size_t x = 0u; x < dimensions[0]; x++ ) {
				for ( size_t y = 0u; y < dimensions[1]; y++ ) {
					const vector4 *colour = nullptr;
					switch( GetTileType( x, y ) ) {

					case TileType::Blank: {
						// ...
					} break;

					case TileType::Wall: {
						colour = &colourTable[ColourIndex( COLOUR_GREY )];
					} break;

					case TileType::Start: {
						colour = &colourTable[ColourIndex( COLOUR_GREEN )];
					} break;

					case TileType::Goal: {
						colour = &colourTable[ColourIndex( COLOUR_RED )];
					} break;

					}

					Renderer::DrawQuad(
						(x * tileWidth) + 1.0f,
						(y * tileHeight) + 1.0f,
						tileWidth - 2.0f,
						tileHeight - 2.0f,
						0.0f, 0.0f, 1.0f, 1.0f, colour, nullptr );
				}
			}
		}

	} // namespace ClientGame

} // namespace XS
