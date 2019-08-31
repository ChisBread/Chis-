#pragma once
#include "utils/types.h"
#include "utils/zobrist_hash.hpp"
namespace chis {
//棋盘值类型
enum BOARD_VAL : uint8_t {  //一些值
    EMP = 0b00,             //空
    WHT = 0b01,             //白
    BLK = 0b10,             //黑
    INV = 0b11,             //无效点
};

//棋盘赋值中间类型 b[][]
template <typename _board_t>
class assign_tmp {
   public:
    _board_t &operator=(const uint8_t &v) { return b.Do(i, j, BOARD_VAL(v)); }

   public:
    _board_t &b;
    size_t i, j;
};
//棋盘赋值中间类型 b[]
template <typename _board_t>
class array_tmp {
   public:
    assign_tmp<_board_t> operator[](size_t j) {
        return assign_tmp<_board_t>{b, i, j};
    }

   public:
    _board_t &b;
    size_t i;
};

}  // namespace chis
