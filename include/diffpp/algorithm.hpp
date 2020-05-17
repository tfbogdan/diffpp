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
#pragma once

#include <vector>
#include <cassert>

namespace diffpp {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Computes the difference between a range A and a range B.
/// The difference between two ranges A and B represents the number of
/// changes we need to make to the range A in order to transform it into B's equivalent
/// @param [in] A0, B0          ranges to be compared
/// @param [in] sizeA, sizeB    size of each range
/// @param [eq] eq              predicate used to compare the elements
template < typename iterA, typename iterB> inline
int difference ( iterA firstA, iterA lastA, iterB firstB, iterB lastB) {
    const int N = std::distance(firstA, lastA);
    const int M = std::distance(firstB, lastB);

    // handle the two special cases where either range is empty
    if (N == 0) {
        return M;
    }

    if (M == 0) {
        return N;
    }

    const int worstCase = N + M + 1;
    std::vector<int> kdmapV(worstCase, 0);

    auto kdmap = [&kdmapV, M](int k) {
      if (k+M >= 0 && k+M < kdmapV.size()) {
          return kdmapV[k + M];
      } else {
          assert(false);
      }
    };

    auto kdmap_store = [&kdmapV, M](int k, int d) {
        if (k+M >= 0 && k+M < kdmapV.size()) {
            kdmapV[k + M] = d;
        } else {
            assert(false);
        }
    };

    std::vector<std::vector<int>> ks;
    ks.resize(M+N+2);
    ks[0].resize(2, 0); // The special case at the start

    auto ixForKnD = [](int k, int d){
        return ((-d) - k) / -2;
    };

    for (int distance = 0; distance <= N + M; ++distance) {
        auto& kmap = ks[distance + 1];
        auto& pkmap = ks[distance];
        kmap.resize(distance + 1);

        for ( int kx = 0; kx <= distance; ++kx) {
            int kline = -distance + kx * 2;

            bool down = kline == -distance || (kline != distance && pkmap[ixForKnD(kline - 1, distance - 1)] <  pkmap[ixForKnD(kline + 1, distance - 1)]);
            int kPrev = down ? kline + 1 : kline - 1;

            // start point
            int xStart = pkmap[ixForKnD(kPrev, distance - 1)];
//            int yStart = xStart - kPrev;

            // mid point
            int xMid = down ? xStart : xStart + 1;
            int yMid = xMid - kline;

            // end point
            int xEnd = xMid;
            int yEnd = yMid;

            // follow diagonal
            int snake = 0;
            while ( xEnd < N && yEnd < M && *std::next(firstA, xEnd) == *std::next(firstB, yEnd)) {
                xEnd++; yEnd++; snake++;
            }

            kmap[kx] = xEnd;
            if ( xEnd >= N && yEnd >= M ) {
                return distance;
            }
        }
    }
    return N + M;
}

} // ::diffpp
