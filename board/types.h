#pragma once
#include "utils/types.h"
namespace chis
{
//长度类型
using bsize_t = uint64_t;
class bcell_t;

//棋盘赋值中间类型 b[][]
template<typename _board_t>
class assign_tmp {
public:
    _board_t& operator=(const bcell_t &v) {
        return b.Do(i,j,v);
    }
public:
    _board_t &b;
    size_t i, j;
};
//棋盘赋值中间类型 b[]
template<typename _board_t>
class array_tmp {
public:
    assign_tmp<_board_t> operator[](size_t j) {
        return assign_tmp<_board_t>{b, i, j};
    }
public:
    _board_t &b;
    size_t i;
};
//棋盘值类型
class bcell_t {
public:
    bcell_t(const bitset_type<2> &val):val(val){}
    bcell_t(const bsize_t &val):val(val){}
    template<typename _board_t>
    bcell_t(const assign_tmp<_board_t> &a):val(a.b.Get(a.i, a.j).val){}
public:
    template<typename _board_t>
    bcell_t& operator=(assign_tmp<_board_t> &a) {
        return *this = a.b.Get(a.i, a.j);
    }
    bcell_t& operator=(const bcell_t &a) {
        return *this = a;
    }
    bool operator[](size_t i) const {
        return val[i];
    }
public:
    bitset_type<2> val;
};
//一些值
enum BOARD_VAL {
    EMP = 0, WHT = 1, BLK= 2, INV = 3
};
const static bcell_t EMP_CELL = bitset_type<2>(uint64_t(EMP));//00
const static bcell_t WHT_CELL = bitset_type<2>(uint64_t(WHT));//01
const static bcell_t BLK_CELL = bitset_type<2>(uint64_t(BLK));//10
const static bcell_t INV_CELL = bitset_type<2>(uint64_t(INV));//11

} // namespace chis
