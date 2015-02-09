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

		enum TileType {
			TILE_BLANK,
			TILE_WALL,
			TILE_START,
			TILE_GOAL
		};
		
		struct Node {
			uint32_t x;
			uint32_t y;
			inline bool operator==( const Node &rhs ) {
				if ( x == rhs.x && y == rhs.y ) {
					return true;
				}
				return false;
			}
		};

		static const size_t dimensions = 16u;

		static struct GameState {
			uint8_t tiles[dimensions][dimensions];
			std::list<Node> openList;
			std::list<Node> closedList;
		} state = {};

		static Renderer::View *sceneView = nullptr;

		static void RenderScene( void ) {
			// the view is already bound
		}

		static void GenerateMaze( void ) {
			// surround the map in a box
			for ( size_t x = 0u; x < dimensions; x++ ) {
				state.tiles[x][0] = TILE_WALL;
				state.tiles[x][dimensions - 1] = TILE_WALL;
			}
			for ( size_t y = 0u; y < dimensions; y++ ) {
				state.tiles[0][y] = TILE_WALL;
				state.tiles[dimensions - 1][y] = TILE_WALL;
			}
			
			// randomly place the start position with 2 cell padding
			int32_t startX = 0u;
			int32_t startY = 0u;
			while ( true ) {
				startX = 1 + (rand() % (dimensions - 1));
				startY = 1 + (rand() % (dimensions - 1));
				if ( startX < 3 || startX >= dimensions - 3 ) {
					continue;
				}
				if ( startY < 3 || startY >= dimensions - 3 ) {
					continue;
				}
				if ( state.tiles[startX][startY] == TILE_BLANK ) {
					break;
				}
			}
			state.tiles[startX][startY] = TILE_START;
			uint8_t *startNW	= &state.tiles[startX - 1][startY - 1];
			uint8_t *startN		= &state.tiles[startX    ][startY - 1];
			uint8_t *startNE	= &state.tiles[startX + 1][startY - 1];
			uint8_t *startE		= &state.tiles[startX + 1][startY    ];
			uint8_t *startSE	= &state.tiles[startX + 1][startY + 1];
			uint8_t *startS		= &state.tiles[startX    ][startY + 1];
			uint8_t *startSW	= &state.tiles[startX - 1][startY + 1];
			uint8_t *startW		= &state.tiles[startX - 1][startY    ];

			// randomly surround the start tile
			int32_t side = rand() % 4;
			if ( side == 0 ) {
				// north
				*startNW	= TILE_WALL;
				*startN		= TILE_BLANK;
				*startNE	= TILE_WALL;
				*startE		= TILE_WALL;
				*startSE	= TILE_WALL;
				*startS		= TILE_WALL;
				*startSW	= TILE_WALL;
				*startW		= TILE_WALL;
			}
			else if ( side == 1 ) {
				// east
				*startNW	= TILE_WALL;
				*startN		= TILE_WALL;
				*startNE	= TILE_WALL;
				*startE		= TILE_BLANK;
				*startSE	= TILE_WALL;
				*startS		= TILE_WALL;
				*startSW	= TILE_WALL;
				*startW		= TILE_WALL;
			}
			else if ( side == 2 ) {
				// south
				*startNW	= TILE_WALL;
				*startN		= TILE_WALL;
				*startNE	= TILE_WALL;
				*startE		= TILE_WALL;
				*startSE	= TILE_WALL;
				*startS		= TILE_BLANK;
				*startSW	= TILE_WALL;
				*startW		= TILE_WALL;
			}
			else if ( side == 3 ) {
				// west
				*startNW	= TILE_WALL;
				*startN		= TILE_WALL;
				*startNE	= TILE_WALL;
				*startE		= TILE_WALL;
				*startSE	= TILE_WALL;
				*startS		= TILE_WALL;
				*startSW	= TILE_WALL;
				*startW		= TILE_BLANK;
			}

			// randomly place the end position with 2 cell padding, at-least 3 cells away from the start
			int32_t goalX = 0u;
			int32_t goalY = 0u;
			while ( true ) {
				goalX = 1 + (rand() % (dimensions - 1));
				goalY = 1 + (rand() % (dimensions - 1));
				if ( goalX < 3 || goalX >= dimensions - 3 ) {
					continue;
				}
				if ( goalY < 3 || goalY >= dimensions - 3 ) {
					continue;
				}
				if ( std::abs( goalX - startX ) + std::abs( goalY - startY ) < 6 ) {
					continue;
				}
				if ( state.tiles[goalX][goalY] == TILE_BLANK ) {
					break;
				}
				
			}
			state.tiles[goalX][goalY] = TILE_GOAL;
			uint8_t *goalNW	= &state.tiles[goalX - 1][goalY - 1];
			uint8_t *goalN	= &state.tiles[goalX    ][goalY - 1];
			uint8_t *goalNE	= &state.tiles[goalX + 1][goalY - 1];
			uint8_t *goalE	= &state.tiles[goalX + 1][goalY    ];
			uint8_t *goalSE	= &state.tiles[goalX + 1][goalY + 1];
			uint8_t *goalS	= &state.tiles[goalX    ][goalY + 1];
			uint8_t *goalSW	= &state.tiles[goalX - 1][goalY + 1];
			uint8_t *goalW	= &state.tiles[goalX - 1][goalY    ];

			// randomly surround the goal tiles
			side = rand() % 4;
			if ( side == 0 ) {
				// north
				*goalNW	= TILE_WALL;
				*goalN	= TILE_BLANK;
				*goalNE	= TILE_WALL;
				*goalE	= TILE_WALL;
				*goalSE	= TILE_WALL;
				*goalS	= TILE_WALL;
				*goalSW	= TILE_WALL;
				*goalW	= TILE_WALL;
			}
			else if ( side == 1 ) {
				// east
				*goalNW	= TILE_WALL;
				*goalN	= TILE_WALL;
				*goalNE	= TILE_WALL;
				*goalE	= TILE_BLANK;
				*goalSE	= TILE_WALL;
				*goalS	= TILE_WALL;
				*goalSW	= TILE_WALL;
				*goalW	= TILE_WALL;
			}
			else if ( side == 2 ) {
				// south
				*goalNW	= TILE_WALL;
				*goalN	= TILE_WALL;
				*goalNE	= TILE_WALL;
				*goalE	= TILE_WALL;
				*goalSE	= TILE_WALL;
				*goalS	= TILE_BLANK;
				*goalSW	= TILE_WALL;
				*goalW	= TILE_WALL;
			}
			else if ( side == 3 ) {
				// west
				*goalNW	= TILE_WALL;
				*goalN	= TILE_WALL;
				*goalNE	= TILE_WALL;
				*goalE	= TILE_WALL;
				*goalSE	= TILE_WALL;
				*goalS	= TILE_WALL;
				*goalSW	= TILE_WALL;
				*goalW	= TILE_BLANK;
			}

			// partially surround the start/end positions
			for ( size_t iteration = 0u; iteration < 5; iteration++ ) {
				
			}

			// continuously branch off randomly from the surrounding walls using a weighted flood-fill
			//	5 iterations?
		}
		
		static void PrintMaze( void ) {
			for ( size_t x = 0u; x < dimensions; x++ ) {
				for ( size_t y = 0u; y < dimensions; y++ ) {
					switch( state.tiles[x][y] ) {

					case TILE_BLANK: {
						console.Print( "  " );
					} break;

					case TILE_WALL: {
						console.Print( "# " );
					} break;

					case TILE_START: {
						console.Print( "S " );
					} break;

					case TILE_GOAL: {
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
				PathfindingStep( PathfindingAlgorithm::AStar );
				lastTime = currentTime;
			}
		}

		void DrawFrame( void ) {
			sceneView->Bind();

			const uint32_t screenWidth = Cvar::Get( "vid_width" )->GetInt();
			const uint32_t screenHeight = Cvar::Get( "vid_height" )->GetInt();

			const real32_t aspectCoefficient =
				static_cast<real32_t>( screenHeight ) / static_cast<real32_t>( screenWidth );

			const real32_t tileWidth = (screenWidth / dimensions) * aspectCoefficient;
			const real32_t tileHeight = (screenHeight / dimensions);

			for ( size_t x = 0u; x < dimensions; x++ ) {
				for ( size_t y = 0u; y < dimensions; y++ ) {
					const vector4 *colour = nullptr;
					switch( state.tiles[x][y] ) {

					case TILE_BLANK: {
						// ...
					} break;

					case TILE_WALL: {
						colour = &colourTable[ColourIndex( COLOUR_GREY )];
					} break;

					case TILE_START: {
						colour = &colourTable[ColourIndex( COLOUR_GREEN )];
					} break;

					case TILE_GOAL: {
						colour = &colourTable[ColourIndex( COLOUR_RED )];
					} break;

					}

					Renderer::DrawQuad(
						x * tileWidth,
						y * tileHeight,
						tileWidth,
						tileHeight,
						0.0f, 0.0f, 1.0f, 1.0f, colour, nullptr );
				}
			}
		}

	} // namespace ClientGame

} // namespace XS
