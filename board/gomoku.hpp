#pragma once
#include "board/operator.hpp"
#include "board/types.h"
namespace chis {
template<bsize_t size = 15, bsize_t offset = 5>
class GomokuBitBoard {
public:
    //相对坐标
    void set(bsize_t i, bsize_t j, const bcell_t &val) {
        heng[HENG(i,j,size,offset)*2] = val[0];
        shu[  SHU(i,j,size,offset)*2] = val[0];
        pie[  PIE(i,j,size,offset)*2] = val[0];
        na[    NA(i,j,size,offset)*2] = val[0];
        heng[HENG(i,j,size,offset)*2+1] = val[1];
        shu[  SHU(i,j,size,offset)*2+1] = val[1];
        pie[  PIE(i,j,size,offset)*2+1] = val[1];
        na[    NA(i,j,size,offset)*2+1] = val[1];
    }
    bcell_t get(bsize_t i, bsize_t j) {
        return (bsize_t(heng[HENG(i,j,size,offset)*2+1])<<1)+bsize_t(heng[HENG(i,j,size,offset)*2]);
    }
    //绝对坐标
    void set_real(bsize_t i, bsize_t j, const bcell_t &val) {
        heng[HENG_REAL(i,j,size,offset)*2] = val[0];
        shu[  SHU_REAL(i,j,size,offset)*2] = val[0];
        pie[  PIE_REAL(i,j,size,offset)*2] = val[0];
        na[    NA_REAL(i,j,size,offset)*2] = val[0];
        heng[HENG_REAL(i,j,size,offset)*2+1] = val[1];
        shu[  SHU_REAL(i,j,size,offset)*2+1] = val[1];
        pie[  PIE_REAL(i,j,size,offset)*2+1] = val[1];
        na[    NA_REAL(i,j,size,offset)*2+1] = val[1];
    }
    bcell_t get_real(bsize_t i, bsize_t j) {
        return (bsize_t(heng[HENG_REAL(i,j,size,offset)*2+1])<<1)+bsize_t(heng[HENG_REAL(i,j,size,offset)*2]);
    }
public:
    GomokuBitBoard() {
        for(size_t i = 0; i < size+offset*2; ++i) {
            for(size_t j = 0; j < offset; ++j) {
                set_real(i, j, INV);
                set_real(i, size + offset*2 - 1 - j, INV);
                set_real(j, i, INV);
                set_real(size + offset*2 - 1 - j, i, INV);
            }
        }
    }
public:
    bitset_type<HENG_MAXLEN(size,offset)*2> heng;
    bitset_type< SHU_MAXLEN(size,offset)*2> shu;
    bitset_type< PIE_MAXLEN(size,offset)*2> pie;
    bitset_type<  NA_MAXLEN(size,offset)*2> na;
};
struct do_info {
    bsize_t i;
    bsize_t j;
};
template<bsize_t size = 15, bsize_t offset = 5>
class GomokuBoard {
public:
    //重载[]
    array_tmp<GomokuBoard<size, offset>> operator[](size_t i) {
            return array_tmp<GomokuBoard<size, offset>>{*this, i};
    }
    //落子
    GomokuBoard<size, offset>& Do(bsize_t i, bsize_t j, const bcell_t &v) {
        board.set(i, j, v);
        doChain.push_back(do_info{i, j});
        return *this;
    }
    //撤销落子
    GomokuBoard<size, offset>& Uodo() {
        board.set(doChain.back().i, doChain.back().j, EMP);
        doChain.pop_back();
        return *this;
    }
    bcell_t Get(bsize_t i, bsize_t j) {
        return board.get(i,j);
    }
    //得到点附近的棋型
    // bitset_type<11>[4] GetPattern(bsize_t i, bsize_t j) {

    // }
public:
    GomokuBoard() {
    }
public:
    //status
    vector_type<do_info> doChain;
    GomokuBitBoard<size, offset> board;//四种值的位棋盘
};
}