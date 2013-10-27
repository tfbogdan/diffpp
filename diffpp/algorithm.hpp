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
  
  namespace algorithms {
    namespace detail { 

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
          

      template < typename iterA, typename iterB, typename direction_policy, typename predicate > inline
      int common_subsequence_length( iterA A0, iterB B0, const int ALen, const int BLen, const int Ai, const int Bi, direction_policy dir, predicate eq ) {
        const unsigned idxA = Ai - ( direction_policy::seek_direction == -1 );
        const unsigned idxB = Bi - ( direction_policy::seek_direction == -1 );
        if ( idxA < ALen && idxB < BLen ) {
          std::advance(A0, idxA);
          std::advance(B0, idxB);
          int len(0);
          if ( eq(*A0, *B0) ) {
            do {
              len += direction_policy::seek_direction; 
              if (  direction_policy::in_range(Ai+len, ALen ) && 
                    direction_policy::in_range(Bi+len, BLen ) ) {
                std::advance(A0, direction_policy::seek_direction);
                std::advance(B0, direction_policy::seek_direction);
              } else break;
            } while(eq(*A0, *B0));
          }
          return len;
        }
        return 0;
      }

      template < typename direction_policy > 
      struct edge_walker { 
        typedef direction_policy        direction_t;

        typedef edge_walker< direction_t > type_t;

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
                          predicate eq  ) { 

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

          const int matches = common_subsequence_length(A0, B0, countA, countB, xend, yend, direction_t(), eq);
          xend += matches;
          yend += matches;
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
      

      template < typename direction_policy, typename kdmap_policy >
      struct greedy_graph_walker : public kdmap_policy { 
        typedef direction_policy                                                    direction_t;
        typedef kdmap_policy                                                        kdmap_t;
        typedef greedy_graph_walker< direction_t, kdmap_t >                         type_t;
        typedef edge_walker< direction_t >                                          seeker_t;


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
    }
  }

    
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
        algorithms::detail::bwd_kdmap_stdmap > computer_type;
    computer_type computer;
    return computer(A0, B0, sizeA, sizeB, eq);
  }
} // ::diffpp

#endif // ALGORITHM_HPP_INCLUDED
