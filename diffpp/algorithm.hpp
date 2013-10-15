/**
*  																						
*  The MIT License (MIT)
*  
*  Copyright (c) 2013 bogdan tudoran
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
#ifndef ALGORITHM_HPP_INCLUDED
#define ALGORITHM_HPP_INCLUDED

#include <list>
#include <map>
#include <iterator>
#include <functional>
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

		edit ( const point &Start, const point &End ) : start(Start), end(End) {}
		const point &get_start(void) const { return start; }
		const point &get_end  (void) const { return end; }
		unsigned get_matches  (void) const { return std::abs( get_command() == DELETE ? start.y - end.y : start.x - end.x ); }
		command_t get_command (void) const { return std::abs( end.x - start.x ) > std::abs( end.y - start.y ) ? DELETE : INSERT; }	
		int get_direction			(void) const { return end.x > start.x || end.y > start.y ? 1 : -1; }
		int get_differences		(void) const { return get_matches() +  1; }
	private:
		point start;	
		point end;
	};
	
	namespace algorithms {
		
		namespace detail { 

			typedef std::map< int, int > kdmap_t;
			typedef std::vector< kdmap_t > kdmap_container_t;

			struct search_forward { 
				template < typename fwdItA, typename fwdItB, typename predicate > inline
				static bool exec ( const fwdItA A0, const fwdItB B0, const int sizeA, const int sizeB, kdmap_t &kdmap, const int distance, predicate eq, const int delta_ab, const int inv_dist, const int kline ) {
					bool down = ( inv_dist == kline || ( kline != distance && kdmap[kline-1] < kdmap[kline+1] ) );
					int prev_kline = kline + ( down ? 1 : -1 );

					point start; start.x = kdmap[prev_kline];
					start.y = start.x - prev_kline;

					point end; end.x = start.x + ( down ? 0 : 1 );	
					end.y = end.x - kline;

					fwdItA itA( A0 ); fwdItB itB( B0 );
					std::advance( itA, end.x );
					std::advance( itB, end.y );

					while ( end.x < sizeA && end.y < sizeB && eq(*itA, *itB) ) { 
						++end.x;
						++end.y;
						++itA; ++itB;
					}

					kdmap[kline] = end.x;

					if ( end.x >= sizeA && end.y >= sizeB ) return true; // solution found
					return false;
				}
			};

			struct search_backward { 
				template < typename fwdItA, typename fwdItB, typename predicate  > inline
				static bool exec ( const fwdItA A0, const fwdItB B0, const int sizeA, const int sizeB, kdmap_t &kdmap, int distance, predicate eq, const int delta_ab, const int inv_dist, const int kline ) {
					bool up = ( kline == distance + delta_ab || ( kline != (inv_dist + delta_ab) && kdmap[kline-1] < kdmap[kline+1] ) );
					const int prev_kline = kline + ( up ? -1 : 1 );

					point start;start.x = kdmap[ prev_kline ];
					start.y = start.x - prev_kline;

					point end; end.x = up ? start.x : start.x - 1;
					end.y = end.x - kline;

					fwdItA itA(A0);
					fwdItB itB(B0);
					std::advance(itA, end.x-1);
					std::advance(itB, end.y-1);
					
					while ( end.x > 0 && end.y > 0 && eq(*itA, *itB) ) {
						--end.x;
						--end.y;
						--itA;--itB;
					}

					kdmap[kline] = end.x;

					if ( end.x <= 0 && end.y <= 0 ) return true;
					return false;
				}
			};

			struct solver_greedyfwd {
				template < typename container_t > 
				static void solve ( kdmap_container_t &kdmaps, container_t &sln, const int sizeA, const int sizeB ) {
					point current( sizeA, sizeB );
					for ( int distance = ( kdmaps.size()-1); current.x > 0 || current.y > 0; --distance ) {
						kdmap_t &kdmap( kdmaps[distance] );
						const int inv_dist(distance * -1);
						const int kline = current.x - current.y;
						
						point end; end.x = kdmap[kline];
						end.y = end.x - kline; 

						bool down = ( inv_dist == kline || ( kline != distance && kdmap[kline-1] < kdmap[kline+1] ) );

						int prev_kline = kline + ( down ? 1 : -1 );

						point start; start.x = kdmap[prev_kline];
						start.y = start.x - prev_kline;

						sln.insert( sln.begin(), 1, edit(start, end) );
						
						current = start;
					}
				}
			};

			struct solver_greedybwd {
				template < typename container_t > 
				static void solve ( kdmap_container_t &kdmaps, container_t &sln, const int sizeA, const int sizeB ) {
					point current(0,0);
					const int delta_ab = sizeA-sizeB;
					
          for ( int distance = kdmaps.size() -1; current.x < sizeA || current.y < sizeB; --distance ) {
						kdmap_t &kdmap( kdmaps[distance] );
            const int kline = current.x - current.y;
						const int inv_dist = distance * -1;

						point end; end.x = kdmap[kline];
						end.y = end.x - kline;

						bool up = ( kline == distance + delta_ab || ( kline != (inv_dist + delta_ab) && kdmap[kline-1] < kdmap[kline+1] ) );
						const int prev_kline = kline - ( up ? 1 : -1 );

						point start; start.x = kdmap[prev_kline];
						start.y = start.x - prev_kline;
						
						sln.insert( sln.end(), 1, edit(start, end) );

						current = start;

					}
				}
			};

			template < typename search_dir, typename solve_dir, typename fwdItA, typename fwdItB, typename container_t, typename predicate > inline
			void diff_greedy ( fwdItA A0, fwdItB B0, container_t &solution_out, kdmap_t &kdmap, const int sizeA, const int sizeB, predicate eq, bool dir ) { 
				kdmap_container_t path_maps;
				bool solution_found = false;
				const int worst_case = sizeA + sizeB;
				const int delta_ab = dir ? 0 : sizeA - sizeB;

				for ( int distance(0); distance <= worst_case; ++distance ) {
					const int inv_dist = distance * (-1);
					for ( int kline = inv_dist + delta_ab; kline <= distance + delta_ab; kline += 2 ) {
						solution_found = search_dir::exec(A0, B0, sizeA, sizeB, kdmap, distance, eq, delta_ab, inv_dist, kline );
						if ( solution_found ) break;
					}
					path_maps.push_back(kdmap);
					if ( solution_found ) break;
				}

				if ( solution_found )
					solve_dir::solve( path_maps, solution_out, sizeA, sizeB );
				
			}

			template < typename fwdItA, typename fwdItB > inline 
			edit find_middle_edit ( fwdItA A0, fwdItB B0, const int sizeA, const int sizeB, kdmap_t &kdmapfwd, kdmap_t &kdmapbwd ) {
				const int worst_case = sizeA + sizeB;
				const int delta = sizeA - sizeB;

				for ( int distance(0); distance <= (worst_case+1)/2; ++distance ) {
					const int inv_dist = distance * (-1); 
					
					for ( int kline(inv_dist); kline<=distance; kline+=2 ) {
						if ( delta%2==0 && (kline>=delta-(distance-1) && kline<=delta+(distance-1)) ) {
						}
					}

					for ( int kline(inv_dist); kline<=distance; kline+=2 ) {
					}
					
				}
				return edit();
			}
			template < typename fwdItA, typename fwdItB, typename container_t, typename predicate > inline 
			void diff_linear ( const fwdItA A0, const int sizeA, const fwdItB B0, const int sizeB,container_t &sln, predicate eq, kdmap_t &kdmapfwd, kdmap_t &kdmapbwd ) {
				if ( sizeB==0 && sizeA>0 ) {;} // edge case
				if ( sizeA==0 && sizeB>0 ) {;} // edge case
				if ( sizeA==0 || sizeB==0 ) return; // something is really going bad
				
				edit middle_edit = find_middle_edit(A0, B0, sizeA, sizeB, kdmapfwd, kdmapbwd );
				const int edits = middle_edit.get_differences();

				if ( edits > 1 ) {
					const int xs = middle_edit.get_start().x;
					const int ys = middle_edit.get_start().y;
					const int xe = middle_edit.get_end().x;
					const int ye = middle_edit.get_end().y;

					fwdItA itA(A0);
					fwdItB itB(B0);
					std::advance(itA, xs);
					std::advance(itB, ys);

					diff_linear(itA, xs, itB, ys, sln, eq, kdmapfwd, kdmapbwd);

					itA=A0;
					itB=B0;
					std::advance(itA, xe);
					std::advance(itB, ye);
					diff_linear(itA, xe, itB, ye, sln, eq, kdmapfwd, kdmapbwd);
				} else if ( edits == 1 ) {

				} else if ( edits == 0 ) { 
				
				}

			}
		}
		
		/// computes the difference between range A [A0,An) and range B [B0, Bm), expressed as the shortest edit script for turning range a to range b
		/// @param[in] A0, An, B0, Bm ranges to be diffed
		///	@param[out] sln container which will store the solution
		/// @param[in] eq predicate function used to compare the elements. defaults to std::equal_to
		template < typename fwdItA, typename fwdItB, typename container_t, typename predicate=std::equal_to<const typename fwdItA::reference > > inline
		void diff_greedyfwd ( fwdItA A0, fwdItA An, fwdItB B0, fwdItB Bm, container_t &sln, predicate eq = predicate() ) {
			const int sizeA = std::distance( A0, An );
			const int sizeB = std::distance( B0, Bm );
			detail::kdmap_t kdmap;
			kdmap[1] = 0;
			detail::diff_greedy<detail::search_forward,
								detail::solver_greedyfwd> ( A0, B0, sln, kdmap, sizeA, sizeB, eq, true );		
		}

		template < typename fwdItA, typename fwdItB, typename container_t, typename predicate=std::equal_to<const typename fwdItA::reference > > inline
		void diff_greedybwd ( fwdItA A0, fwdItA An, fwdItB B0, fwdItB Bm, container_t &sln, predicate eq = predicate() ) {
			const int sizeA = std::distance( A0, An );
			const int sizeB = std::distance( B0, Bm );
			detail::kdmap_t kdmap;
			kdmap[ sizeA - sizeB - 1 ] = sizeA;
			detail::diff_greedy<detail::search_backward,
								detail::solver_greedybwd> ( A0, B0, sln, kdmap, sizeA, sizeB, eq, false );		
		}

		template < typename fwdItA, typename fwdItB, typename container_t, typename predicate=std::equal_to<const typename fwdItA::reference > > inline
		void diff_linear ( fwdItA A0, fwdItA An, fwdItB B0, fwdItB Bm, container_t &sln, predicate eq = predicate() ) {
// this function is a stube
			const int sizeA = std::distance( A0, An );
			const int sizeB = std::distance( B0, Bm );

			detail::kdmap_t kdmapfwd;kdmapfwd[1]=0;
			detail::kdmap_t kdmapbwd;kdmapbwd[ sizeA - sizeB - 1 ] = sizeA;
			
			detail::diff_linear(A0,sizeA,B0,sizeB,sln,eq,kdmapfwd,kdmapbwd);	
		}

		template < typename fwdItA, typename fwdItB, typename container_t, typename predicate=std::equal_to<const typename fwdItA::reference > > inline
		void diff ( fwdItA A0, fwdItA An, fwdItB B0, fwdItB Bm, container_t &sln, predicate eq = predicate() ) {
			diff_greedyfwd(A0,An,B0,Bm,sln,eq);	
		}
	}	//::diff::algorithms

} // ::diffpp

#endif // ALGORITHM_HPP_INCLUDED
