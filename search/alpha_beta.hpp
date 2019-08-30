#pragma once
#include "board/gomoku.hpp"
namespace chis {
//template <typename Board, size_t MAX_DEPTH>
using Board = GomokuBoard<15,5>;
const size_t MAX_DEPTH = 6;
class AlphaBeta {
   public:
   public:
    int alpha_beta(int alpha, int beta, int depth) {
        if(auto[val, ok] = board.Ending();ok) {
            return val;
        }
    }
    Board board;
};
}  // namespace chis
