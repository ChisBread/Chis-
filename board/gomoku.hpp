#pragma once
#include "board/gomoku_bitboard.hpp"
#include "board/types.h"
#include "resource/patterns.h"
namespace chis {
static const int WON = 99999999;
static const int NBRATE = 2;
struct pattern_info {
    int pattern_cnt_blk[16] = {};
    int pattern_cnt_wht[16] = {};
};

//五子棋棋盘-状态/Hash/...
template <size_t size = 15, size_t offset = 5,
          typename BoardTy = GomokuBitBoard<size, offset>>
class GomokuBoard {
   public:
    using GomokuBoardType = GomokuBoard<size, offset>;

    struct do_info {
        int i;
        int j;
        BOARD_VAL v;
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
        doChain.push_back(do_info{i, j, v});
        //减去左右两边第一个棋子的同向棋型
        {
            auto pats = board.GetEmptyPointPattern(i, j);
            for(int k = 0; k < 4; ++k) {
                for(int l = 0; l < 2; ++l) {
                    //减去黑子棋型
                    if(((pats[i][k] >> 10)&0x3) == BOARD_VAL::BLK) {
                        BOARD_VAL blk = pattern_type[pats[i][k]] & 0xf;
                        --pinfo.pattern_cnt_blk[blk];
                    //减去白字棋型
                    } else if(((pats[i][k] >> 10)&0x3) == BOARD_VAL::WHT) {
                        BOARD_VAL wht = pattern_type[pats[i][k]] >> 4;
                        --pinfo.pattern_cnt_wht[wht];
                    }
                }
            }
        }
        board.Set(i, j, v);    //棋盘变化
        zobrist.Set(i, j, v);  // hash变化
        uint32_t pats[4] = board.GetPattern(i,j);
        //四向棋型
        for(int k = 0; k < 4; ++k) {
            //减去黑子棋型
            if(v == BOARD_VAL::BLK) {
                BOARD_VAL blk = pattern_type[pats[k]] & 0xf;
                ++pinfo.pattern_cnt_blk[blk];
            //减去白字棋型
            } else if(v == BOARD_VAL::WHT) {
                BOARD_VAL wht = pattern_type[pats[k]] >> 4;
                ++pinfo.pattern_cnt_wht[wht];
            }
        }
        {
            auto pats = board.GetEmptyPointPattern(i, j);
            for(int k = 0; k < 4; ++k) {
                for(int l = 0; l < 2; ++l) {
                    //减去黑子棋型
                    if(((pats[i][k] >> 10)&0x3) == BOARD_VAL::BLK) {
                        BOARD_VAL blk = pattern_type[pats[i][k]] & 0xf;
                        ++pinfo.pattern_cnt_blk[blk];
                    //减去白字棋型
                    } else if(((pats[i][k] >> 10)&0x3) == BOARD_VAL::WHT) {
                        BOARD_VAL wht = pattern_type[pats[i][k]] >> 4;
                        ++pinfo.pattern_cnt_wht[wht];
                    }
                }
            }
        }
        return *this;
    }
    //撤销落子
    GomokuBoardType &Undo() {
        board.Reset(doChain.back().i, doChain.back().j);  //棋盘置空
        zobrist.Set(doChain.back().i, doChain.back().j,
                    doChain.back().v);  // hash变化
        doChain.pop_back();
        return *this;
    }
    void Reset() {
        while (!doChain.empty()) {
            Undo();
        }
    }

   public:
    // Hash
    uint64_t Hash() const { return zobrist.Hash(); }
    // 着法生成
    vector_type<std::tuple<int, int>> Moves(bool must = false) {
        if (doChain.empty()) {
            return {{size / 2, size / 2}};
        }
        vector_type<std::pair<std::tuple<int, int>, int>> sorted;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (board.Get(i, j) == BOARD_VAL::EMP) {
                    int nb = 0;
                    for (int n = -NBRATE; n <= NBRATE; ++n) {
                        if (n == 0) continue;
                        auto addnb = [&](int x, int y) {
                            if (board.GetReal(x, y) != BOARD_VAL::EMP &&
                                board.GetReal(x, y) != BOARD_VAL::INV) {
                                nb += 1;
                            }
                        };
                        auto [hx, hy] = HENG_NEXT(i + offset, j + offset, n);
                        addnb(hx, hy);
                        auto [sx, sy] = SHU_NEXT(i + offset, j + offset, n);
                        addnb(sx, sy);
                        auto [px, py] = PIE_NEXT(i + offset, j + offset, n);
                        addnb(px, py);
                        auto [nx, ny] = NA_NEXT(i + offset, j + offset, n);
                        addnb(nx, ny);
                    }
                    if (nb) {
                        sorted.push_back({{i, j}, nb});
                    }
                }
            }
        }

        vector_type<std::tuple<int, int>> ret;
        auto enemy_pat = Turn() == BOARD_VAL::BLK ? pinfo.pattern_cnt_wht
                                                  : pinfo.pattern_cnt_blk;

        auto me_pat = Turn() == BOARD_VAL::BLK ? pinfo.pattern_cnt_blk
                                               : pinfo.pattern_cnt_wht;

        // TODO 增量点棋型
        if (!must && ((me_pat[PAT_TYPE::S4] || me_pat[PAT_TYPE::L4A] ||
                       me_pat[PAT_TYPE::L4B] || me_pat[PAT_TYPE::L3A] ||
                       me_pat[PAT_TYPE::L3B] || me_pat[PAT_TYPE::S3] ||
                       me_pat[PAT_TYPE::L2A] + me_pat[PAT_TYPE::L2B] +
                               me_pat[PAT_TYPE::L2C] >
                           1) ||
                      //有4、有3、有两个活2
                      (enemy_pat[PAT_TYPE::S4] || enemy_pat[PAT_TYPE::L4A] ||
                       enemy_pat[PAT_TYPE::L4B] || enemy_pat[PAT_TYPE::L3A] ||
                       enemy_pat[PAT_TYPE::L3B] || enemy_pat[PAT_TYPE::S3] ||
                       enemy_pat[PAT_TYPE::L2A] + enemy_pat[PAT_TYPE::L2B] +
                               enemy_pat[PAT_TYPE::L2C] >
                           1))) {
            for (auto &item : sorted) {
                auto [x, y] = item.first;
                uint8_t pats[4] = {};
                //敌方
                std::tie(pats[0], pats[1], pats[2], pats[3]) = GetPatternType(
                    x, y,
                    Turn() == BOARD_VAL::BLK ? BOARD_VAL::WHT : BOARD_VAL::BLK);
                int enemy_info[16] = {};
                PAT_TYPE enemy_pty = PAT_TYPE::NON;
                for (int i = 0; i < 4; ++i) {
                    if (Turn() == BOARD_VAL::BLK) {  //白色反派
                        pats[i] = pats[i] >> 4;
                    } else {
                        pats[i] = pats[i] & 0x0F;
                    }
                    ++enemy_info[pats[i]];
                    if (enemy_pty < pats[i]) {
                        enemy_pty = PAT_TYPE(pats[i]);
                    }
                }
                //己方
                std::tie(pats[0], pats[1], pats[2], pats[3]) = GetPatternType(
                    x, y,
                    Turn() == BOARD_VAL::BLK ? BOARD_VAL::BLK : BOARD_VAL::WHT);
                int me_info[16] = {};
                PAT_TYPE me_pty = PAT_TYPE::NON;
                for (int i = 0; i < 4; ++i) {
                    if (Turn() == BOARD_VAL::BLK) {
                        pats[i] = pats[i] & 0x0F;
                    } else {
                        pats[i] = pats[i] >> 4;
                    }
                    ++me_info[pats[i]];
                    if (me_pty < pats[i]) {
                        me_pty = PAT_TYPE(pats[i]);
                    }
                }
                //优先级判断
                //己方>敌方
                //成5>活4>=44=43>33
                if (me_info[PAT_TYPE::FIVE]) {
                    return {{x, y}};
                } else if (enemy_info[PAT_TYPE::FIVE]) {
                    item.second = WON + 9;
                    //己方 活4或者44, 43
                } else if (me_info[PAT_TYPE::L4A] || me_info[PAT_TYPE::L4B] ||
                           me_info[PAT_TYPE::S4] > 1 ||
                           (me_info[PAT_TYPE::S4] &&
                            (me_info[PAT_TYPE::L3A] ||
                             me_info[PAT_TYPE::L3B]))) {
                    item.second = WON + 8;
                    //对方 活4或者44, 43
                } else if (enemy_info[PAT_TYPE::L4A] ||
                           enemy_info[PAT_TYPE::L4B] ||
                           enemy_info[PAT_TYPE::S4] > 1 ||
                           (enemy_info[PAT_TYPE::S4] &&
                            (enemy_info[PAT_TYPE::L3A] +
                             enemy_info[PAT_TYPE::L3B]))) {
                    item.second = WON + 7;
                    //己方 33
                } else if (me_info[PAT_TYPE::L3A] + me_info[PAT_TYPE::L3B] >
                           1) {
                    item.second = WON + 6;
                    //对方 33
                } else if (enemy_info[PAT_TYPE::L3A] +
                               enemy_info[PAT_TYPE::L3B] >
                           1) {
                    item.second = WON + 5;
                    //己方 冲4
                } else if (me_info[PAT_TYPE::S4]) {
                    item.second = WON + 4;
                }
                // val >= WON+8 | 不考虑冲4，以val截断
                // val == WON+7 | 除了val,考虑冲4点 WON+4
            }
        }
        std::sort(sorted.begin(), sorted.end(),
                  [](auto a, auto b) { return a.second > b.second; });
        //存在成5
        if (sorted.front().second >= WON + 8) {
            return {sorted.front().first};
            //
        } else if (sorted.front().second == WON + 7) {
            for (auto &s : sorted) {
                if (s.second < WON + 4) {
                    break;
                }
                if (s.second == WON + 7 || s.second == WON + 4) {
                    ret.push_back({s.first});
                }
            }
        } else {
            for (auto &s : sorted) {
                ret.push_back({s.first});
            }
        }
        return ret;
    }
    // 评估函数 NegaEva
    int32_t Evaluation() {
        static const int evaluation[14] = {0,  1,   10,  12,  30,   35,   40,
                                           45, 100, 120, 230, 1000, 1000, WON};
        int val = 0;
        for (int i = 1; i < 14; ++i) {
            val += (pinfo.pattern_cnt_blk[i] * evaluation[i]);
            val -= (pinfo.pattern_cnt_wht[i] * evaluation[i]);
        }
        return Turn() == BOARD_VAL::WHT ? -val : val;
    }
    std::tuple<int32_t, bool> Ending() {
        if (doChain.size() == size * size) {
            return {0, true};
        }
        int32_t val = 0, val2 = 0;

        // check 先手胜利 A为下一步先手 返回A的胜负
        auto check = [](const int(&A)[16], const int(&B)[16]) {
            // B已经赢了
            if (B[PAT_TYPE::FIVE]) {
                return -WON;
            }
            // A已经赢了
            else if (A[PAT_TYPE::FIVE]) {
                return WON;
            }
            // A先手，有4直接赢
            else if (A[PAT_TYPE::L4A] || A[PAT_TYPE::L4B] || A[PAT_TYPE::S4]) {
                return WON;
            }
            // 到这里，A已经没5没4了
            // B活四
            if (B[PAT_TYPE::L4A] || B[PAT_TYPE::L4B]) {
                return -WON;
            }
            // B没有成5点
            if (!B[PAT_TYPE::S4]) {
                // A有成活四点就必胜了
                if (A[PAT_TYPE::L3A] || A[PAT_TYPE::L3B]) {
                    return WON;
                }
            }
            // A没有成4点（只能被动防守），则考虑B有杀
            if (!(A[PAT_TYPE::L3A] || A[PAT_TYPE::L3B] || A[PAT_TYPE::S3])) {
                // B必胜的情况
                // B下一把有两个及以上成5点，大概率堵不住（待统计）
                if (B[PAT_TYPE::S4] > 1) {
                    return -WON;
                }
                // B下一把有一个成5点（只能防守），同时又有成活4点
                else if (B[PAT_TYPE::S4] &&
                         (B[PAT_TYPE::L3A] || B[PAT_TYPE::L3B])) {
                    return -WON;
                }
                // B下一把有两个及以上成活四点 (双活三)
                else if (B[PAT_TYPE::L3A] + B[PAT_TYPE::L3B] > 1) {
                    return -WON;
                }
            }

            return 0;
        };

        if (Turn() == BOARD_VAL::BLK) {  //黑先手
            val = check(pinfo.pattern_cnt_blk, pinfo.pattern_cnt_wht);

        } else {
            val = -check(pinfo.pattern_cnt_wht, pinfo.pattern_cnt_blk);
        }
        return {Turn() == BOARD_VAL::WHT ? -val : val, val != 0};
    }
    int32_t PointEvaluation(int i, int j) {
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
    //取值
    BOARD_VAL Get(int i, int j) const { return board.Get(i, j); }
    //得到点附近的棋型
    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GetPatternType(int i,
                                                                  int j) const {
        auto [h, s, p, n] = board.GetPattern(i, j);
        return {pattern_type[h], pattern_type[s], pattern_type[p],
                pattern_type[n]};
    }
    //得到点附近的棋型（空点填充)
    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GetPatternType(
        int i, int j, uint32_t v) const {
        auto [h, s, p, n] = board.GetPattern(i, j);
        return {pattern_type[h | (v << 10)], pattern_type[s | (v << 10)],
                pattern_type[p | (v << 10)], pattern_type[n | (v << 10)]};
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
    uint64_t pinfoSig;
};
template <typename BTy, typename VTy>
using GomokuBoardMap = std::unordered_map<BTy, VTy, typename BTy::hash_func,
                                          typename BTy::equal_func>;
}  // namespace chis