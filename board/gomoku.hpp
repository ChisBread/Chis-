#pragma once
#include "board/gomoku_arrayboard.hpp"
#include "board/types.h"
#include "resource/patterns.h"
namespace chis {

//五子棋棋盘-状态/Hash/...
template <bsize_t size = 15, bsize_t offset = 5,
          typename BoardTy = GomokuArrayBoard<size, offset>>
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
        uint64_t operator()(const GomokuBoardType &b) const { return b.hash(); }
    };
    class equal_func {
       public:
        uint64_t operator()(const GomokuBoardType &b,
                            const GomokuBoardType &c) const {
            return b.hash() == c.hash();
        }
    };

   public:
    //重载[]
    array_tmp<GomokuBoardType> operator[](size_t i) {
        return array_tmp<GomokuBoardType>{*this, i};
    }
    //落子
    GomokuBoardType &Do(bsize_t i, bsize_t j, const BOARD_VAL v) {
        board.Set(i, j, v);
        zobrist.Set(i, j, v);
        doChain.push_back(do_info{i, j, v});
        return *this;
    }
    //撤销落子
    GomokuBoardType &Uodo() {
        board.Set(doChain.back().i, doChain.back().j, EMP);  //置空
        zobrist.Set(doChain.back().i, doChain.back().j, doChain.back().v);
        doChain.pop_back();
        return *this;
    }

   public:
    // Hash
    uint64_t Hash() const { return zobrist.Hash(); }
    //评估函数
    int32_t Evaluation() const { return 0; }
    //取值
    BOARD_VAL Get(bsize_t i, bsize_t j) const { return board.Get(i, j); }
    //得到点附近的棋型
    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GetPatternType(
        bsize_t i, bsize_t j) const {
        auto [h, s, p, n] = board.GetPattern(i, j);
        return {pattern_type[h], pattern_type[s], pattern_type[p],
                pattern_type[n]};
    }
    std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> GetPattern(
        bsize_t i, bsize_t j) const {
        return board.GetPattern(i, j);
    }

   public:
    GomokuBoard() {}

   public:
    // status
    vector_type<do_info> doChain;
    BoardTy board;  //棋盘
    // hashing
    ZobristHash<size, 4> zobrist;
    // pattern
    GomokuPatterns pattern_type;
};
template <typename BTy, typename VTy>
using GomokuBoardMap = std::unordered_map<BTy, VTy, typename BTy::hash_func,
                                          typename BTy::equal_func>;
}  // namespace chis