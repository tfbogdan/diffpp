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
    edit (void) {}
    const point &get_start(void) const { return start; }
    const point &get_end  (void) const { return end; }
    unsigned get_matches  (void) const { return std::abs( get_command() == DELETE ? start.y - end.y : start.x - end.x ); }
    command_t get_command (void) const { return std::abs( end.x - start.x ) > std::abs( end.y - start.y ) ? DELETE : INSERT; }  
    int get_direction     (void) const { return end.x > start.x || end.y > start.y ? 1 : -1; }
    int get_differences   (void) const { return get_matches() +  1; }
  private:
    point start;  
    point end;
  };
  
  namespace algorithms {
    
    namespace detail { 

      typedef std::map< int, int > kdmap_t;
      typedef std::vector< kdmap_t > kdmap_container_t;


      
      struct forward_direction_policy {
          static const int seek_direction = 1;
          static inline int compute_direction( const int kline, 
                                const int distance, 
                                const int smaller_k_distance, 
                                const int greater_k_distance,
                                const int delta_ab ) {
            return ( kline == (distance * -1) || ( kline != distance && smaller_k_distance < greater_k_distance ) ) ? -1 : 1; 
          }
          static bool at_end ( const int xpos, const int ypos, const int xmax, const int ymax ) { 
            return xpos >= xmax && ypos >= ymax;
          }
          static bool in_range ( const int coord, const int max_coord ) {
            return coord < max_coord;
          }
          static int delta_ab ( const int sizeA, const int sizeB ) { 
            return 0;
          }
      };

      struct backward_direction_policy {
          static const int seek_direction = -1;
          static inline int compute_direction( const int kline, 
                                const int distance, 
                                const int smaller_k_distance, 
                                const int greater_k_distance,
                                const int delta_ab ) {
            return ( kline == distance + delta_ab || ( kline != ((distance * -1) + delta_ab) && smaller_k_distance < greater_k_distance ) ) ? 1 : -1;
          }
          static bool at_end ( const int xpos, const int ypos, const int xmax, const int ymax ) { 
            return xpos <= 0 && ypos <= 0;
          }
          static bool in_range ( const int coord, const int max_coord ) {
            return coord > 0;
          }
          static int delta_ab( const int sizeA, const int sizeB ) { 
            return sizeA - sizeB;
          }
      };
          


      /*hack!*/
      struct iterator_accessor { // experimental iteration_policy ( element access policy )
        template < typename iter >
        static void advance ( iter &it, int step ) { 
          std::advance(it, step);
        }
        template < typename iter > 
        auto dereference( iter it ) -> typename iter::reference { // temporary. will only work with c++11
          return *it;
        }
      };
      /*end of hack*/

      template < typename direction_policy, typename iteration_policy > 
      struct edge_walker : public direction_policy, public iteration_policy { 
        
        typedef direction_policy        direction_t;
        typedef iteration_policy            element_access_t;

        typedef edge_walker< direction_t, element_access_t > type_t;

        explicit edge_walker ( const direction_t &dir = direction_t(), const element_access_t &elacc = element_access_t() ) 
          : direction_t(dir), 
          element_access_t(elacc) {}


        template < typename iterA, typename iterB, typename predicate >
        int operator () ( iterA A0, 
                          const int countA, 
                          iterB B0, 
                          const int countB, 
                          const int kline, 
                          const int distance, 
                          const int dist_lower_k,
                          const int dist_upper_k,
                          const int delta_ab,
                          predicate eq  ) { // some stuff might throw. has to be moved away from ctor
                          // this hasn't been tested. not even compiled 
          // determine movement direction, in terms of k-lines
          direction = direction_t::compute_direction( 
                kline, 
                distance, 
                dist_lower_k,
                dist_upper_k,
                delta_ab );

          prev_kline = kline + direction * -1;
          xend = ( prev_kline > kline ? dist_upper_k: dist_lower_k ) ;
          xend += ( direction == direction_t::seek_direction ) * direction_t::seek_direction;
          yend = xend - kline;

          iteration_policy::advance(A0, xend - ( direction_t::seek_direction == -1 ) );
          iteration_policy::advance(B0, yend - ( direction_t::seek_direction == -1 ) );

          while ( direction_t::in_range( xend, countA ) && 
                  direction_t::in_range( yend, countB ) && 
                  eq ( 
                            element_access_t::dereference( A0 ), 
                            element_access_t::dereference( B0 ) 
                     ) ) { 
            xend += direction_t::seek_direction;
            yend += direction_t::seek_direction;
            element_access_t::advance(A0, direction_t::seek_direction);
            element_access_t::advance(B0, direction_t::seek_direction);
          }
          return xend;
        }

        int get_xend( void ) const { return xend; }
        int get_yend( void ) const { return yend; }
        int get_direction( void ) const { return direction; }
        int get_prevk( void ) const { return prev_kline; }
      private:
        int xend, yend;
        int direction;
        int prev_kline;
      };

      /*hack! shall be seriously reconsidered*/
      struct kdmap_stdmap { 
        static int distance_for_k_for_map ( const std::map<int, int> &m, const int kline ) {
          if ( !m.count(kline) ) return 0;
          return m.find(kline)->second;
        }
        int distance_for_k ( const int kline ) const { 
          return distance_for_k_for_map( map, kline );
        }
        void distance_for_k( const int kline, const int distance ) { 
          map[kline] = distance;
        }
        void distance_snapshot( const unsigned distance ) { 
        }
        const std::map<int, int> &getmap(void) const { return map; }
      protected:
        std::map< int, int > map;
      };
      struct fwd_kdmap_stdmap : public kdmap_stdmap { 
        void reset ( const int sizeA, const int sizeB ) {
          map.clear();
          map[1] = 0; 
        };
      };
      struct bwd_kdmap_stdmap : public kdmap_stdmap { 
        void reset ( const int sizeA, const int sizeB ) { 
          map.clear();
          map[sizeA - sizeB - 1] = sizeA;
        }
      };
      template < typename direction_policy > 
      struct monitored_kdmap_stdmap : public direction_policy { 
        void distance_snapshot ( const unsigned distance ) { 
          if ( distance < maps.size() ) {
            maps[distance] = direction_policy::getmap();
            return;
          } else if ( distance == maps.size() ) { 
            maps.push_back(direction_policy::getmap() );
            return;  
          } else maps.resize(distance+1);
          maps[distance] = direction_policy::getmap();
        }

        int xend_for_dk ( const unsigned distance, const int kline ) const { 
          if ( distance < maps.size() ) { 
            return direction_policy::distance_for_k_for_map(maps[distance], kline);
          }
          return 0;
        }

        unsigned size(void) const { return maps.size(); } 
      protected:
        std::vector< std::map< int, int > > maps;
      }; 

      typedef monitored_kdmap_stdmap< fwd_kdmap_stdmap > fwd_monitored_kdmap_stdmap;
      typedef monitored_kdmap_stdmap< bwd_kdmap_stdmap > bwd_monitored_kdmap_stdmap;
      /*end of hack*/
      

      template < typename direction_policy, typename element_access_policy, typename kdmap_policy >
      struct greedy_graph_walker : public kdmap_policy { 
        typedef direction_policy                                                    direction_t;
        typedef element_access_policy                                               element_access_t;
        typedef kdmap_policy                                                        kdmap_t;
        typedef greedy_graph_walker< direction_t, element_access_t, kdmap_t >  type_t;
        typedef edge_walker< direction_t, element_access_t >                    seeker_t;


        explicit greedy_graph_walker ( const kdmap_t &kdmap = kdmap_t() ) 
          :kdmap_t(kdmap) {}

        template < typename iterA, typename iterB, typename predicate > inline 
        int operator() (  iterA A0,
                          iterB B0, 
                          const int sizeA,
                          const int sizeB,
                          predicate eq ) { 
          kdmap_t::reset(sizeA, sizeB);
          seeker_t seeker;
          const int delta_ab = direction_policy::delta_ab(sizeA,sizeB);
          int distance(-1);
          do {
            ++distance;
            const int distance_inverse( distance * -1 );
            int kline = distance_inverse + delta_ab;
            do {
              const int dist_for_k = seeker(A0, sizeA, B0, sizeB, kline, distance, kdmap_t::distance_for_k( kline - 1), kdmap_t::distance_for_k( kline + 1), delta_ab, eq);
              kdmap_t::distance_for_k(kline, dist_for_k);
              kline += 2;
            } while ( kline <= distance + delta_ab && !direction_t::at_end(seeker.get_xend(), seeker.get_yend(), sizeA, sizeB) );
            kdmap_policy::distance_snapshot(distance);
          } while ( distance<= sizeA + sizeB && !direction_t::at_end(seeker.get_xend(), seeker.get_yend(), sizeA, sizeB) );
          return distance;
        }
      }; 

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



      template < typename direction_policy, typename command_interpreter >
      struct shortest_path_stepper : public command_interpreter { 
        typedef direction_policy            direction_t;
        typedef command_interpreter         interpreter_t;
        typedef shortest_path_stepper< direction_t, interpreter_t > type_t;

        explicit shortest_path_stepper ( const interpreter_t &interpreter = interpreter_t() ) 
          :interpreter_t( interpreter ) {}

        void operator() (   const int sizeA,
                            const int sizeB, 
                            const int distance, 
                            const int kline, 
                            const int dist_kline,        // kdmap[kline]
                            const int dist_ppkline,      // kdmap[kline+1]
                            const int dist_mmkline,      // kdmap[kline-1]
                            const int delta_ab ) { 
          const int distance_inverse(distance* -1);
          
          _xend = dist_kline;
          _yend = _xend - kline;

          _direction = direction_t::compute_direction(kline, distance, dist_mmkline, dist_ppkline, delta_ab);
          
          _prev_kline = kline + _direction * -1;

          _xstart = ( _prev_kline < kline ? dist_mmkline : dist_ppkline );
          _ystart = _xstart - _prev_kline;

        }

        int xstart(void) const { return _xstart;}
        int ystart(void) const { return _ystart;}

        int xend(void) const { return _xend;}
        int yend(void) const { return _yend;}
        
        int direction(void) const { return _direction;}

        unsigned matches(void) const { 
          return std::abs( // todo: abstract away std::abs
                      std::abs(_xend - _xstart) > std::abs(_yend - _ystart) ? 
                      _ystart - _yend : 
                      _xstart - _xend 
                      );
        }
        bool erase(void) const { return (std::abs(_xend-_xstart) > std::abs(_yend-_ystart));}
        bool insert(void) const { return !erase();}

      private:
        int _prev_kline;
        int _direction;
        int _xstart, _ystart, _xend, _yend;
      };


      template < typename direction_policy > 
      struct shortest_path_walker { 
        typedef direction_policy direction_t;
        template < typename dcontours_kdmaps, typename shortest_path_stepper > 
        void operator () (  const int sizeA, 
                            const int sizeB,
                            const dcontours_kdmaps &kdmaps,
                            shortest_path_stepper &generator ) { 
          int xcurrent = sizeA;
          int ycurrent = sizeB;
          int distance = kdmaps.size()-1;
          if ( xcurrent>0 || ycurrent>0 ) {// not generalized 
            do {
                const int kline = xcurrent - ycurrent;
                generator(sizeA, sizeB, distance, kline, kdmaps.xend_for_dk(distance, kline), kdmaps.xend_for_dk(distance,kline+1),kdmaps.xend_for_dk(distance, kline-1), direction_t::delta_ab(sizeA, sizeB) );
                xcurrent = generator.xend();
                ycurrent = generator.yend();
              --distance;
            } while(xcurrent>0 || ycurrent>0 );
          }
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
    /// @param[out] sln container which will store the solution
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
  } //::diff::algorithms

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief Computes the difference between a range A and a range B.
  /// The difference between two ranges A and B represents the number of 
  /// changes we need to make to the range A in order to transform it into B's equivalent
  /// @param [in] A0, B0          ranges to be compared
  /// @param [in] sizeA, sizeB    size of each range
  /// @param [eq] eq              predicate used to compare the elements
  template < typename fwdItA, typename fwdItB, typename predicate > inline
  int difference ( fwdItA A0, fwdItB B0, const int sizeA, const int sizeB, predicate eq = predicate() ) {
    typedef algorithms::detail::greedy_graph_walker< 
        algorithms::detail::forward_direction_policy,
        algorithms::detail::iterator_accessor, 
        algorithms::detail::fwd_kdmap_stdmap > computer_type;
    computer_type computer;
    return computer(A0, B0, sizeA, sizeB, eq);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief Computes the difference between a range A and a range B.
  /// The difference between two ranges A and B represents the number of 
  /// changes we need to make to the range A in order to transform it into B's equivalent
  /// @param [in] A0, B0          ranges to be compared
  /// @param [in] sizeA, sizeB    size of each range
  /// @param [eq] eq              predicate used to compare the elements
  template < typename fwdItA, typename fwdItB, typename predicate > inline
  int difference_bwd ( fwdItA A0, fwdItB B0, const int sizeA, const int sizeB, predicate eq = predicate() ) {
    typedef algorithms::detail::greedy_graph_walker< 
        algorithms::detail::backward_direction_policy,
        algorithms::detail::iterator_accessor, 
        algorithms::detail::bwd_kdmap_stdmap > computer_type;
    computer_type computer;
    return computer(A0, B0, sizeA, sizeB, eq);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @brief Computes the difference between a range A and a range B.
  /// The difference between two ranges A and B represents the number of 
  /// changes we need to make to the range A in order to transform it into B's equivalent
  /// @param [in] A0, B0, An, Bm  ranges [A0, An) and [B0, Bm) to be compared
  /// @param [eq] eq              predicate used to compare the elements
  //template < typename fwdItA, typename fwdItB, typename predicate > inline 
  //int difference ( fwdItA A0, fwdItA An, fwdItB B0, fwdItB Bm, predicate eq = predicate()) {
  //  const int sizeA = std::distance(A0, An);
  //  const int sizeB = std::distance(B0, Bm);
  //  return diffpp::difference<fwdItA, fwdItB, predicate>(   static_cast<fwdItA>(A0), 
  //                                                          static_cast<fwdItB>(B0), 
  //                                                          static_cast<const int>(sizeA),
  //                                                          static_cast<const int>(sizeB),
  //                                                          static_cast<predicate>(eq) );
  //}

} // ::diffpp

#endif // ALGORITHM_HPP_INCLUDED
