#include <list>
#include <map>
#include <queue>
#include <algorithm>

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

		static Renderer::View *sceneView = nullptr;
		static constexpr size_t dimensions[2] = { 32u, 18u };

		// each tile acts as a node in the graph to search
		// a tile has a specific type (e.g. start, goal, wall)
		// the only identifying property of a tile is the position, which is inferred by the tilemap

		// references for A* pathfinding:
		//	https://en.wikipedia.org/wiki/A*_search_algorithm#Pseudocode
		//	http://www.policyalmanac.org/games/aStarTutorial.htm
		//	http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html
		//	http://code.activestate.com/recipes/577457-a-star-shortest-path-algorithm/

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
			int32_t		x, y;

			// a cost of -1 means it is an unmoveable direction (i.e. off the map)
			// the lower the cost, the better the move
			int16_t		cost[NUM_DIRECTIONS];
			Tile		*neighbours[NUM_DIRECTIONS];
			TileType	type;

			// the only unique element for a tile is the coordinate, so use this for comparison
			inline bool operator==( const Tile &rhs ) const {
				if ( x == rhs.x && y == rhs.y ) {
					return true;
				}
				return false;
			}
		};
		typedef std::vector<Tile *> nodeList;

		struct Path {
			enum class Algorithm {
				AStar,
				DStar, //TODO
				Dijkstra, //TODO
				BFS, //TODO
			};

			// tiles are added to the open list if it's to be explored
			// when a tile is determined to be unsuitable, it's moved to the closed list and never checked again
			nodeList				openList, closedList;
			std::map<Tile*, Tile*>	cameFrom;
			nodeList				path;
			Algorithm				algorithm;

			// calculate the H cost of moving from t1 to t2
			int16_t HeuristicCost(
				const Tile *t1,
				const Tile *t2
			);

			// we've moved, so recalculate the path
			bool Reconstruct(
				const Tile *start,
				const Tile *goal
			);

			// construct a path from current tile to goal tile
			nodeList Find(
				Tile *current,
				Tile *goal
			);
		};

		static struct GameState {
			Tile							 tiles[dimensions[0]][dimensions[1]];
			Path							 path;
			nodeList						 route;
			Tile							*start, *goal;
			std::map<const Tile*, int16_t>	 fScore;
			std::map<const Tile*, int16_t>	 gScore;
		} state = {};

		static Tile *GetTile( uint32_t x, uint32_t y ) {
			if ( x < dimensions[0] && y < dimensions[1] ) {
				return &state.tiles[x][y];
			}
			return nullptr;
		}

		static TileType GetTileType( uint32_t x, uint32_t y ) {
			return state.tiles[x][y].type;
		}

		int16_t Path::HeuristicCost( const Tile *t1, const Tile *t2 ) {
			switch( algorithm ) {

				case Algorithm::AStar: {
					Tile delta = {
						.x = std::abs( t2->x - t1->x ),
						.y = std::abs( t2->y - t1->y )
					};

					// euclidean distance
					//	* by 8 to alleviate lack of integer precision
					//return static_cast<int16_t>( sqrt( delta.x*delta.x + delta.y*delta.y ) * 8 );

					// custom
					//	abuse integer truncation to add negative weight to diagonal moves
					return static_cast<int16_t>( delta.x * 1.5f ) + static_cast<int16_t>( delta.y * 1.5f );
				} break;

				default: {
					//TODO: handle other algo's
					return 0;
				} break;

			}

			return 0;
		}

		bool Path::Reconstruct( const Tile *start, const Tile *goal ) {
			//TODO: backtrack from the goal position to the start position based on which order we traversed the
			//	nodes
			//TODO: smooth the path
			return false;
		}

		nodeList Path::Find( Tile *current, Tile *goal ) {
			switch( algorithm ) {

				case Algorithm::AStar: {
					if ( !openList.empty() ) {
						console.Print( "Open list is not empty\n" );
						Indent indentListIsOpen( 1 );

						int16_t lowestF = INT16_MAX;

						// current = node in open list with lowest f score
						// F=G+H
						for ( Tile *tile : openList ) {
							int16_t thisG = 0;
							int16_t thisH = HeuristicCost( tile, goal );
							int16_t thisF = thisG + thisH;
							if ( thisF < lowestF ) {
								console.Print(
									"%i < %i, selecting tile at %i,%i\n",
									thisF,
									lowestF,
									tile->x,
									tile->y
								);
								lowestF = thisF;
								current = tile;
							}
						}

#if 1
						// wut?
						if ( current == goal ) {
							console.Print( "Found the goal\n" );
							openList.clear();
							//Reconstruct( start, goal );
							break;
						}
#endif
						// drop it from the openList, add it to the closedList
						auto it = std::find( openList.begin(), openList.end(), current );
						if ( it != openList.end() ) {
							console.Print(
								"Removing %i,%i from openList, moving to closedList\n",
								current->x,
								current->y
							);
							openList.erase( it );
							closedList.push_back( current );
						}

						// then look at its neighbours
						console.Print( "Searching neighbours...\n" );
						for ( Tile *neighbour : current->neighbours ) {
							if ( neighbour == nullptr ) {
								continue;
							}

							// only add tiles we can navigate to.
							if ( neighbour->type == TileType::Wall ) {
								console.Print( "Ignoring wall at %i,%i\n", neighbour->x, neighbour->y );
								continue;
							}

							console.Print( "Looking at neighbour %i,%i\n", neighbour->x, neighbour->y );

							Indent indentNeighbours( 1 );

							// if this neighbour is in the closedList, skip it
							if ( std::find( closedList.begin(), closedList.end(), neighbour ) != closedList.end() ) {
								continue;
							}
							int16_t score = state.gScore[current] + HeuristicCost( current, neighbour );

							// if this neighbour is not in the openList, or if it has a lower score, mark it for
							//	 traversal
							if ( std::find( openList.begin(), openList.end(), neighbour ) == openList.end()
								|| score < state.gScore[neighbour] )
							{
								cameFrom[neighbour] = current;
								state.gScore[neighbour] = score;
								state.fScore[neighbour] = state.gScore[neighbour] + HeuristicCost( neighbour, goal );
								// if neighbour is not in the open set
								if ( std::find( openList.begin(), openList.end(), neighbour ) == openList.end() ) {
									console.Print( "Adding %i,%i to openList\n", neighbour->x, neighbour->y );
									openList.push_back( neighbour );
									openList = std::vector<Tile *>( openList.begin(), openList.end() );
								}
							}
						}
					}
				} break;

				default: {
					//TODO: handle other algo's
				} break;

			}

			return path;
		}

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

					state.tiles[x][y].neighbours[NorthWest]	= GetTile( x - 1, y - 1 );
					state.tiles[x][y].neighbours[North]		= GetTile( x    , y - 1 );
					state.tiles[x][y].neighbours[NorthEast]	= GetTile( x + 1, y - 1 );
					state.tiles[x][y].neighbours[East]			= GetTile( x + 1, y     );
					state.tiles[x][y].neighbours[SouthEast]	= GetTile( x + 1, y + 1 );
					state.tiles[x][y].neighbours[South]		= GetTile( x    , y + 1 );
					state.tiles[x][y].neighbours[SouthWest]	= GetTile( x - 1, y + 1 );
					state.tiles[x][y].neighbours[West]			= GetTile( x - 1, y     );
				}
			}

			// surround the map in a box
			for ( size_t x = 0u; x < dimensions[0]; x++ ) {
				state.tiles[x][0].type = TileType::Wall;
				state.tiles[x][dimensions[1] - 1].type = TileType::Wall;
			}
			for ( size_t y = 0u; y < dimensions[1]; y++ ) {
				state.tiles[0][y].type = TileType::Wall;
				state.tiles[dimensions[0] - 1][y].type = TileType::Wall;
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
			state.start = GetTile( startX, startY );

			// randomly surround the start tile
			int32_t side = rand() % 4;
			state.start->neighbours[NorthWest]->type	= TileType::Wall;
			state.start->neighbours[North    ]->type	= (side == 0) ? TileType::Blank : TileType::Wall;
			state.start->neighbours[NorthEast]->type	= TileType::Wall;
			state.start->neighbours[East     ]->type	= (side == 1) ? TileType::Blank : TileType::Wall;
			state.start->neighbours[SouthEast]->type	= TileType::Wall;
			state.start->neighbours[South    ]->type	= (side == 2) ? TileType::Blank : TileType::Wall;
			state.start->neighbours[SouthWest]->type	= TileType::Wall;
			state.start->neighbours[West     ]->type	= (side == 3) ? TileType::Blank : TileType::Wall;

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
			state.goal = GetTile( goalX, goalY );

			// randomly surround the goal tiles
			side = rand() % 4;
			state.goal->neighbours[NorthWest]->type	= TileType::Wall;
			state.goal->neighbours[North    ]->type	= (side == 0) ? TileType::Blank : TileType::Wall;
			state.goal->neighbours[NorthEast]->type	= TileType::Wall;
			state.goal->neighbours[East     ]->type	= (side == 1) ? TileType::Blank : TileType::Wall;
			state.goal->neighbours[SouthEast]->type	= TileType::Wall;
			state.goal->neighbours[South    ]->type	= (side == 2) ? TileType::Blank : TileType::Wall;
			state.goal->neighbours[SouthWest]->type	= TileType::Wall;
			state.goal->neighbours[West     ]->type	= (side == 3) ? TileType::Blank : TileType::Wall;

			//TODO: continuously branch off randomly from the surrounding walls using a weighted flood-fill?
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

		void Init( void ) {
			const uint32_t width = Cvar::Get( "vid_width" )->GetInt();
			const uint32_t height = Cvar::Get( "vid_height" )->GetInt();
			sceneView = new Renderer::View( width, height, RenderScene );

			GenerateMaze();
			PrintMaze();
			state.path.algorithm = Path::Algorithm::AStar;
			state.path.closedList.clear();
			// populate the open list with the start position
			state.path.openList.push_back( state.start );
			state.path.cameFrom.clear();

			state.gScore[state.start] = 0;
			state.fScore[state.start] = state.gScore[state.start]
				+ state.path.HeuristicCost( state.start, state.goal );
		}

		void RunFrame( void ) {
			static double lastTime = 0.0;
			const double currentTime = Client::GetElapsedTime();
			if ( lastTime < currentTime - 250 ) {
				// step every 750ms
				state.path.Find( state.start, state.goal );
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
					const Tile *tile = GetTile( x, y );
					const vector4 *colour = nullptr;
					switch( tile->type ) {

					case TileType::Blank: {
						if ( std::find( state.path.openList.begin(), state.path.openList.end(), tile )
							!= state.path.openList.end() )
						{
							colour = &colourTable[ColourIndex( COLOUR_YELLOW )];
						}
						if ( std::find( state.path.closedList.begin(), state.path.closedList.end(), tile )
							!= state.path.closedList.end() )
						{
							colour = &colourTable[ColourIndex( COLOUR_ORANGE )];
						}
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
