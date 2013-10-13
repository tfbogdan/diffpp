
/**
*  																						
*  The MIT License (MIT)
*  
*  Copyright (c) 2013 tfbogdan
*  
*  Permission is hereby granted, free of charge, to any person obtaining a copy of
*  this software and associated documentation files (the "Software"), to deal in
*  the Software without restriction, including without limitation the rights to
*  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
*  the Software, and to permit persons to whom the Software is furnished to do so,
*  subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
*  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
*  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
*  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
*  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*  
*/ 
#ifndef SCRIPT_HPP_INCLUDED
#define SCRIPT_HPP_INCLUDED

#include <list>
#include <map>
#include <iterator>
#include <cstdlib>
#include <cassert> 

namespace diffpp {
	
	struct point {
		point( void ) : x(0), y(0) {}
		point( int X, int Y ) : x(X), y(Y) {}
		int x,y;
	};

	struct edit {
		typedef enum { INSERT, DELETE } command_t;

		edit ( const point &Start, const point &Mid, const point &End ) : start(Start) { 
			assert( Start.x <= End.x && Start.y <= End.y );
			command = Start.x < Mid.x ? DELETE : INSERT;
			matches = End.x - Mid.x;
		}
		const point &get_start( void ) const { return start; }
		unsigned get_matches( void ) const { return matches; }
		command_t get_command( void ) const { return command; }	
	private:
		command_t command;
		unsigned matches;
		point start;	
	};
	
	namespace algorithms {
		
		namespace detail { 

			typedef std::map< int, unsigned > k_path_map;
			typedef std::vector< k_path_map > path_maps_t;

			struct longestpath_fordistance_fwd { 
				template < typename fwdItA, typename fwdItB > inline
				static bool exec ( const fwdItA A0, const fwdItB B0, const int sizeA, const int sizeB, k_path_map &kmap, const int distance ) {
					const int inv_distance = distance * -1; 
					for ( int kline = inv_distance; kline <= distance; kline += 2 ) {
						bool down = ( inv_distance == kline || ( kline != distance && kmap[kline-1] < kmap[kline+1] ) );
						
						int prev_kline = kline + ( down ? 1 : -1 );

						point start; start.x = kmap[prev_kline];
						start.y = start.x - prev_kline;

						point end; end.x = start.x + ( down ? 0 : 1 );	
						end.y = end.x - kline;

						fwdItA itA( A0 ); fwdItB itB( B0 );
						std::advance( itA, end.x );
						std::advance( itB, end.y );

						while ( end.x < sizeA && end.y < sizeB && *itA == *itB ) { 
							++end.x;
							++end.y;
							++itA; ++itB;
						}

						kmap[kline] = end.x;

						if ( end.x >= sizeA && end.y >= sizeB ) return true; // solution found
					}
					return false;
				}
			};

			struct longestpath_fordistance_bwd { 
				template < typename fwdItA, typename fwdItB > inline
				static bool exec ( fwdItA A0, fwdItB B0, const int sizeA, const int sizeB, k_path_map &kmap, int distance ) {
					return false;
				}
			};

			struct solver_greedyfwd {
				template < typename container_t > 
				static void solve ( path_maps_t &kdmaps, container_t &sln, const int sizeA, const int sizeB ) {
					point current( sizeA, sizeB );
					for ( int distance = ( kdmaps.size()-1); current.x > 0 || current.y > 0; --distance ) {
						k_path_map &kmap( kdmaps[distance] );
						const int inv_dist(distance * -1);
						const int kline = current.x - current.y;
						
						point end; end.x = kmap[kline];
						end.y = end.x - kline; 

						bool down = ( inv_dist == kline || ( kline != distance && kmap[kline-1] < kmap[kline+1] ) );

						int prev_kline = kline + ( down ? 1 : -1 );

						point start; start.x = kmap[prev_kline];
						start.y = start.x - prev_kline;

						point mid; mid.x = start.x + ( down ? 0 : 1 );
						mid.y = mid.x - kline;
					
						sln.push_back( edit(start, mid, end) );	
						
						current = start;
					}
				}
			};

			struct solver_greedybwd {
				template < typename container_t > 
				static void solve ( const path_maps_t &kdmaps, container_t &sln, const int sizeA, const int sizeB ) {
				
				}
			};

			template < typename calc_longestpath, typename calc_solutionfct, typename fwdItA, typename fwdItB, typename container_t  > inline
			void diff_greedy ( fwdItA A0, fwdItA An, fwdItB B0, fwdItB Bm, container_t &solution_out ) { 
				const int sizeA = std::distance(A0,An); 
				const int sizeB = std::distance(B0,Bm);
				
				k_path_map kmap;
				kmap[1] = 0;

				path_maps_t path_maps;
				bool solution_found = false;
				const int worst_case = sizeA + sizeB;

				for ( int distance(0); distance <= worst_case; ++distance ) {
					solution_found = calc_longestpath::exec(A0, B0, sizeA, sizeB, kmap, distance);
					path_maps.push_back(kmap);
					if ( solution_found ) break;
				}

				if ( solution_found )
					calc_solutionfct::solve( path_maps, solution_out, sizeA, sizeB );
				
			}
		}

		template < typename fwdItA, typename fwdItB, typename container_t > inline
		void diff_greedyfwd ( fwdItA A0, fwdItA An, fwdItB B0, fwdItB Bm, container_t &sln ) {
			detail::diff_greedy<detail::longestpath_fordistance_fwd,
								detail::solver_greedyfwd> ( A0, An, B0, Bm, sln );		
		}

		template < typename fwdItA, typename fwdItB, typename container_t > inline
		void diff_greedybwd ( fwdItA A0, fwdItA An, fwdItB B0, fwdItB Bm, container_t &sln ) {
			detail::diff_greedy<detail::longestpath_fordistance_bwd,
								detail::solver_greedybwd> ( A0, An, B0, Bm, sln );		
		}

	}	//::diff::algorithms

} // ::diffpp

#endif //SCRIPT_HPP_INCLUDED
