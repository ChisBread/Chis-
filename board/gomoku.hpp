#pragma once
#include "board/mappings.hpp"
#include "board/types.h"
#include "gomoku_bitboard.hpp"
namespace chis {

//只考虑黑棋棋形，白棋转成黑棋再判断
using pinfo_t = uint8_t;
static const pinfo_t WON = 0x80;

//五子棋棋盘-状态/Hash/...
template <bsize_t size = 15, bsize_t offset = 5, typename BoardTy = GomokuBitBoard<size, offset>>
class GomokuBoard {
   public:
    using GomokuBoardType = GomokuBoard<size, offset>;
    struct do_info {
        bsize_t i;
        bsize_t j;
        bsize_t v;
    };
    class hash_func {
       public:
        uint64_t operator()(const GomokuBoardType &b) const {
            return b.hashval;
        }
    };
    class equal_func {
       public:
        uint64_t operator()(const GomokuBoardType &b,
                            const GomokuBoardType &c) const {
            return b.hashval == c.hashval;
        }
    };
    struct point_pattern {
        uint8_t heng;
        uint8_t shu;
        uint8_t pie;
        uint8_t na;
    };
   public:
    //重载[]
    array_tmp<GomokuBoardType> operator[](size_t i) {
        return array_tmp<GomokuBoardType>{*this, i};
    }
    //落子
    GomokuBoardType &Do(bsize_t i, bsize_t j, const bcell_t &v) {
        board.set(i, j, v);
        bsize_t val = v.val.to_ulong();
        hashval ^= zobrist[i][j][val];
        doChain.push_back(do_info{i, j, val});
        return *this;
    }
    //撤销落子
    GomokuBoardType &Uodo() {
        board.set(doChain.back().i, doChain.back().j, EMP);  //置空
        hashval ^=
            zobrist[doChain.back().i][doChain.back().j][doChain.back().v];
        doChain.pop_back();
        return *this;
    }
    bcell_t Get(bsize_t i, bsize_t j) const { return board.get(i, j); }
    //评估函数
    int32_t Evaluation() { return 0; }
    //得到点附近的棋型
    point_pattern GetPattern(bsize_t i, bsize_t j) { return {}; }

   public:
    GomokuBoard() {
        hashval = rand_uint64();
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                zobrist[i][j][0] = rand_uint64();
                zobrist[i][j][1] = rand_uint64();
                zobrist[i][j][2] = rand_uint64();
                zobrist[i][j][3] = rand_uint64();
            }
        }
    }

   public:
    // status
    vector_type<do_info> doChain;
    BoardTy board;  //棋盘
    // hashing
    uint64_t hashval;
    uint64_t zobrist[size][size][4];
};
template <typename BTy, typename VTy>
using GomokuBoardMap = std::unordered_map<BTy, VTy, typename BTy::hash_func,
                                          typename BTy::equal_func>;
}  // namespace chis