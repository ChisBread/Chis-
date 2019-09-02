#pragma once
#include "board/iterator.hpp"
#include "board/types.h"
namespace chis {
//五子棋位棋盘-基本操作
template <size_t size = 15, size_t offset = 5>
class GomokuArrayBoard {
   public:  // required method
    //相对坐标
    void Set(int i, int j, const BOARD_VAL val) {
        board[i + offset][j + offset] = val;
    }
    BOARD_VAL Get(int i, int j) const { return board[i + offset][j + offset]; }
    //绝对坐标
    void SetReal(int i, int j, const BOARD_VAL val) { board[i][j] = val; }
    BOARD_VAL GetReal(int i, int j) const { return board[i][j]; }
    std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> GetPattern(int i,
                                                                  int j) const {
        uint32_t hp = 0, sp = 0, pp = 0, np = 0;
        for (int n = -5; n < 6; ++n) {
            auto [hx, hy] = HENG_NEXT(i + offset, j + offset, n);
            hp = (hp << 2);
            hp |= board[hx][hy];
            auto [sx, sy] = SHU_NEXT(i + offset, j + offset, n);
            sp = (sp << 2);
            sp |= board[sx][sy];
            auto [px, py] = PIE_NEXT(i + offset, j + offset, n);
            pp = (pp << 2);
            pp |= board[px][py];
            auto [nx, ny] = NA_NEXT(i + offset, j + offset, n);
            np = (np << 2);
            np |= board[nx][ny];
        }
        return {hp, sp, pp, np};
    }

   public:
    GomokuArrayBoard() {
        for (int i = 0; i < size + offset * 2; ++i) {
            for (int j = 0; j < offset; ++j) {
                SetReal(i, j, INV);
                SetReal(i, size + offset * 2 - 1 - j, INV);
                SetReal(j, i, INV);
                SetReal(size + offset * 2 - 1 - j, i, INV);
            }
        }
    }

   public:
    BOARD_VAL board[size + offset * 2][size + offset * 2] = {};
};
}  // namespace chis