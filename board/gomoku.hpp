#pragma once
#include "board/gomoku_arrayboard.hpp"
#include "board/types.h"
#include "resource/patterns.h"
namespace chis {
static const int WON = 999999999;
//五子棋棋盘-状态/Hash/...
template <size_t size = 15, size_t offset = 5,
          typename BoardTy = GomokuArrayBoard<size, offset>>
class GomokuBoard {
   public:
    using GomokuBoardType = GomokuBoard<size, offset>;
    struct pattern_info {
        size_t pattern_cnt_blk[14] = {};
        size_t pattern_cnt_wht[14] = {};
    };
    struct do_info {
        int i;
        int j;
        BOARD_VAL v;
        pattern_info pinfo;
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
    //
    BOARD_VAL Turn() const {
        return doChain.size() % 2 ? BOARD_VAL::WHT : BOARD_VAL::BLK;
    }
    //落子
    GomokuBoardType &Do(int i, int j) { return Do(i, j, Turn()); }
    GomokuBoardType &Do(int i, int j, const BOARD_VAL v) {
        doChain.push_back(do_info{i, j, v, pinfo});
        {  //减去当前状态
            uint8_t pats[4];
            std::tie(pats[0], pats[1], pats[2], pats[3]) = GetPatternType(i, j);
            for (auto pat : pats) {
                --pinfo.pattern_cnt_blk[pat & 0xf];
                --pinfo.pattern_cnt_wht[pat >> 4];
            }
        }
        board.Set(i, j, v);    //棋盘变化
        zobrist.Set(i, j, v);  // hash变化
        {                      //累加新状态
            uint8_t pats[4];
            std::tie(pats[0], pats[1], pats[2], pats[3]) = GetPatternType(i, j);
            for (auto pat : pats) {
                ++pinfo.pattern_cnt_blk[pat & 0xf];
                ++pinfo.pattern_cnt_wht[pat >> 4];
            }
        }
        return *this;
    }
    //撤销落子
    GomokuBoardType &Undo() {
        pinfo = doChain.back().pinfo;                        //棋型回滚
        board.Set(doChain.back().i, doChain.back().j, EMP);  //棋盘置空
        zobrist.Set(doChain.back().i, doChain.back().j,
                    doChain.back().v);  // hash变化
        doChain.pop_back();
        return *this;
    }

   public:
    // Hash
    uint64_t Hash() const { return zobrist.Hash(); }
    // 着法生成
    vector_type<std::tuple<int, int>> Moves() const {
        if (doChain.empty()) {
            return {{size / 2, size / 2}};
        }
        vector_type<std::tuple<int, int>> ret;
        bool dedup[size + offset * 2][size + offset * 2] = {};
        for (int di = doChain.size() - 1; di >= 0; --di) {
            auto doitem = doChain[di];
            for (int i = doitem.i - 3; i < doitem.i + 3; ++i) {
                for (int j = doitem.j - 3; j < doitem.j + 3; ++j) {
                    if (board.Get(i, j) == BOARD_VAL::EMP &&
                        !dedup[i + offset][j + offset]) {
                        ret.push_back({i, j});
                        dedup[i + offset][j + offset] = true;
                    }
                }
            }
        }
        return ret;
    }
    // 评估函数
    int32_t Evaluation() const {
        static const int evaluation[14] = {0,  1,   10,  12,  30,   35,   40,
                                           45, 100, 120, 230, 1000, 1000, WON};
        int32_t val = 0;
        if (pinfo.pattern_cnt_blk[13]) {
            val = WON;
        } else if (pinfo.pattern_cnt_wht[13]) {
            val = -WON;
        } else {
            for (size_t i = 1; i < 14; ++i) {
                val +=
                    (pinfo.pattern_cnt_blk[i] * evaluation[i]);  //棋型间2倍差
                val -= (pinfo.pattern_cnt_wht[i] * evaluation[i]);
            }
        }
        return Turn() == BOARD_VAL::WHT ? -val : val;
    }
    int32_t PointEvaluation(int i, int j) const {
        static const int evaluation[14] = {0,  1,   10,  12,  30,   35,   40,
                                           45, 100, 120, 130, 1000, 1000, WON};
        int32_t val = 0;
        uint8_t pats[4];
        std::tie(pats[0], pats[1], pats[2], pats[3]) = GetPatternType(i, j);
        for (auto pat : pats) {
            //对方重要的点，也是自己重要的点
            val += evaluation[pat & 0xf];
            val += evaluation[pat >> 4];
        }
        return val;
    }
    std::tuple<int32_t, bool> Ending() const {
        if (doChain.size() == size * size) {
            return {0, true};
        }
        if (pinfo.pattern_cnt_blk[13] || pinfo.pattern_cnt_wht[13]) {
            return {Evaluation(), true};
        }
        return {0, false};
    }
    //取值
    BOARD_VAL Get(int i, int j) const { return board.Get(i, j); }
    //得到点附近的棋型
    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GetPatternType(int i,
                                                                  int j) const {
        auto [h, s, p, n] = board.GetPattern(i, j);
        return {pattern_type[h], pattern_type[s], pattern_type[p],
                pattern_type[n]};
    }
    std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> GetPattern(int i,
                                                                  int j) const {
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
    pattern_info pinfo;
};
template <typename BTy, typename VTy>
using GomokuBoardMap = std::unordered_map<BTy, VTy, typename BTy::hash_func,
                                          typename BTy::equal_func>;
}  // namespace chis