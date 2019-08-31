#pragma once
#include "board/gomoku.hpp"
namespace chis {
enum RELATION_VAL : int {
    INVALID = 0,
    PV = 1,
    ALPHA = 2,
    BETA = 2,  //置换表内的值比
};
//置换表信息
struct ttInfo {
    uint64_t key = 0;
    int depth = 0;
    RELATION_VAL rel = RELATION_VAL::INVALID;
    int value = 0;
    std::tuple<int, int> bestMove = {-1, -1};
};
//统计信息
struct statInfo {
    size_t leaf_cnt = 0;
    size_t ending_cnt = 0;
    size_t tt_record_cnt = 0;
    size_t tt_hit_cnt = 0;
    size_t tt_pv_pass_cnt = 0;
    size_t tt_alpha_pass_cnt = 0;
    size_t tt_beta_pass_cnt = 0;
    size_t bestmove_try_cnt = 0;
    size_t bestmove_pass_cnt = 0;
    size_t pvs_try_cnt = 0;
    size_t pvs_pass_cnt = 0;
    size_t node_cnt = 0;
};

template <typename Board>
// using Board = GomokuBoard<15, 5>;
// const size_t MAX_DEPTH = 6;
// const size_t MEM_BYTE = 128000000;
// const size_t TT_SIZE = MEM_BYTE / sizeof(ttInfo);
class Solution {
   public:
    Solution(size_t MEM_BYTE = 128000000)
        : TT_SIZE(MEM_BYTE / sizeof(ttInfo)), TT(MEM_BYTE / sizeof(ttInfo)) {}
        using MovWithVal = vector_type<std::pair<std::tuple<int, int>, int>>;
   public:  //以下方法都是线程不安全的，一个线程建议就搞一个实例
    MovWithVal Search(Board &board, const size_t MAX_DEPTH = 5,const MovWithVal &recommend = {}) {
        StartSearch();
        MovWithVal moves;
        for (auto mov : board.Moves()) {
            moves.push_back({mov, -WON});
        }
        for (size_t depth = 0; depth <= MAX_DEPTH; ++depth) {
            for (auto &mov : moves) {
                auto [i, j] = mov.first;
                board.Do(i, j);  //落子
                int val;
                //如果已经找到PV, 则后续节点使用PVS
                val = -AlphaBeta(board, -WON, WON, depth);
                board.Undo();
                mov.second = val;
                //cout << val << endl;
                if (val == WON) {
                    break;
                }
            }
            std::stable_sort(moves.begin(), moves.end(), [](auto a, auto b) {
                return a.second > b.second;
            });
            while(moves.size() > 1 && moves.back().second == -WON) {
                moves.pop_back();
            }
            auto &bestMove = moves.front();
            {
                auto [i, j] = bestMove.first;
                cout << "最佳着法:" << i << " " << j << " "
                     << bestMove.second << " " << depth << endl;
            }
            if (bestMove.second == WON || bestMove.second == -WON || moves.size() == 1) {
                break;
            }
        }
        return moves;
    }
    //带Alpha-Beta剪枝的Min-Max, 使用NegaMax简化形式
    //增加了置换表优化
    //增加了主要变例搜索优化
    int AlphaBeta(Board &board, int alpha, int beta, int depth) {
        bool isZeroWin = ABS(beta-alpha) == 1;
        if(!isZeroWin) {
            ++stat.node_cnt;
        }
        // cout << alpha << "\t" << beta << endl;
        //命中缓存
        int maxVal = INT32_MIN;
        std::tuple<int, int> bestMove = {-1, -1};
        if (ttInfo tt = TT[board.Hash() % TT_SIZE];
            tt.key == board.Hash() &&
            (depth <= tt.depth || (tt.value == WON || tt.value == -WON))) {
            ++stat.tt_hit_cnt;
            if (tt.rel == RELATION_VAL::PV) {
                //如果是真实值，则直接选用
                ++stat.tt_pv_pass_cnt;
                return tt.value;
            } else if (tt.rel == RELATION_VAL::ALPHA &&
                       tt.value <= alpha) {  //有效窗口值
                //如果是辣鸡节点，直接返回alpha
                ++stat.tt_alpha_pass_cnt;
                return alpha;
            } else if (tt.rel == RELATION_VAL::BETA &&
                       tt.value >= beta) {  //有效窗口值
                //如果是厉害节点，直接返回beta
                ++stat.tt_beta_pass_cnt;
                return beta;
            }
            if (auto [i, j] = tt.bestMove; i != -1 && j != -1) {
                bestMove = tt.bestMove;
            }
        }
        auto record_tt = [&](uint64_t key, int dp, RELATION_VAL rel, int value,
                             std::tuple<int, int> mov) {
            //非零窗口且深度足够 or 胜利局面
            if (( !isZeroWin && TT[key % TT_SIZE].depth <= dp) ||
                (value == WON || value == -WON)) {
                ++stat.tt_record_cnt;
                TT[key % TT_SIZE] = {key, dp, rel, value, mov};
            }
        };
        {  
            //叶子节点
            if (auto [val, end] = board.Ending(); end) {
                if (!isZeroWin) {
                    ++stat.leaf_cnt;
                    if(end) {
                        ++stat.ending_cnt;
                    }
                }
                //局面终结
                record_tt(board.Hash(), depth, RELATION_VAL::PV, val, bestMove);
                return val;
            }
            if(depth == 0) {
                if (!isZeroWin) {
                    ++stat.leaf_cnt;
                }
                int val = board.Evaluation();
                return val;
            }
        }

        //搜索一个着法，如果返回在窗口内，则更新alpha.
        //如果在窗口外，则更新置换表，返回out
        bool foundPV = false;
        auto search_move = [&](std::tuple<int, int> mov,
                               int next_depth) -> bool {
            auto [i, j] = mov;
            board.Do(i, j);  //落子
            //搜索操作，返回估值和是否在窗口内
            auto [val, out] = [&]() -> std::tuple<int, bool> {
                int val;
                //如果已经找到PV, 则后续节点使用PVS
                if (foundPV) {
                    ++stat.pvs_try_cnt;
                    val = -(this->*AlphaBetaPtr)(board, -alpha - 1, -alpha,
                                                 next_depth);
                    if (val > alpha && val < beta) {
                        val = -(this->*AlphaBetaPtr)(board, -beta, -alpha,
                                                     next_depth);
                    } else {
                        ++stat.pvs_pass_cnt;
                    }
                } else {
                    val = -(this->*AlphaBetaPtr)(board, -beta, -alpha,
                                                 next_depth);
                }
                if (val >= beta) {
                    return {beta, true};
                } else if (val > alpha) {
                    alpha = val;
                    foundPV = true;
                }
                return {val, false};
            }();
            board.Undo();  //撤销
            //记录最好下法
            if (val > maxVal) {
                maxVal = val;
                bestMove = mov;
            }
            if (out) {
                record_tt(board.Hash(), depth, RELATION_VAL::BETA, beta,
                          bestMove);
            }
            return out;
        };
        //置换表中有最佳着法,直接尝试
        if (auto [i, j] = bestMove; i != -1 && j != -1) {
            ++stat.bestmove_try_cnt;
            bool out = search_move(bestMove, depth - 1);
            if (out) {
                ++stat.bestmove_pass_cnt;
                return beta;
            }
        }
        //着法生成,正常搜索
        for (auto mov : board.Moves()) {
            // beta节点 被截断
            bool out = search_move(mov, depth - 1);
            if (out) {
                return beta;
            }
        }
        //没有out, 更新置换表
        record_tt(board.Hash(), depth,
                  maxVal == alpha ? RELATION_VAL::PV : RELATION_VAL::ALPHA,
                  alpha, bestMove);
        return alpha;
    }

   public:  //下面这些是线程安全的, 用来做超时控制，可以做到瞬间返回(截断)
    void StopSearch() {
        std::lock_guard<std::mutex> lg(ABPtrMtx);
        AlphaBetaPtr = &Solution<Board>::AlphaBetaEnd;
    }
    void StartSearch() {
        std::lock_guard<std::mutex> lg(ABPtrMtx);
        AlphaBetaPtr = &Solution<Board>::AlphaBeta;
    }
    bool IsStop() { return AlphaBetaPtr == &Solution<Board>::AlphaBetaEnd; }
    int AlphaBetaEnd(Board &board, int alpha, int beta, int depth) {
        return AlphaBeta(board, alpha, beta, 0);  //跳到叶节点
    }

   public:
    const size_t TT_SIZE;
    vector_type<ttInfo> TT;
    statInfo stat;
    std::mutex ABPtrMtx;
    int (Solution<Board>::*AlphaBetaPtr)(Board &, int, int,
                                         int) = &Solution<Board>::AlphaBeta;
};
}  // namespace chis
