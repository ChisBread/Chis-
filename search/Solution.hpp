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
    size_t tt_record_cnt = 0;
    size_t tt_pv_pass_cnt = 0;
    size_t tt_alpha_pass_cnt = 0;
    size_t tt_beta_pass_cnt = 0;
    size_t bestmove_pass_cnt = 0;
    size_t pvs_pass_cnt = 0;
    size_t pvs_pass_root_cnt = 0;
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

   public:  //以下方法都是线程不安全的，一个线程建议就搞一个实例
    std::pair<std::tuple<int, int>, int> Search(Board &board,
                                                const size_t MAX_DEPTH = 5) {
        StartSearch();
        std::vector<std::pair<std::tuple<int, int>, int>> moves;
        for (auto mov : board.Moves()) {
            moves.push_back({mov, -WON});
        }
        std::pair<std::tuple<int, int>, int> bestMove = {{-1, -1}, 0};
        for (size_t depth = 0; depth <= MAX_DEPTH; ++depth) {
            int alpha = -WON, beta = WON;  //重置窗口
            bool foundPV = false;
            for (auto &mov : moves) {
                auto [i, j] = mov.first;
                board.Do(i, j);  //落子
                // int val = -AlphaBeta(board, -beta, -alpha, depth);
                int val;
                //如果已经找到PV, 则后续节点使用PVS
                if (foundPV) {
                    val = -AlphaBeta(board, -alpha - 1, -alpha, depth);
                    if (val > alpha && val < beta) {
                        val = -AlphaBeta(board, -beta, -alpha, depth);
                    } else {
                        ++stat.pvs_pass_root_cnt;
                    }
                } else {
                    val = -AlphaBeta(board, -beta, -alpha, depth);
                }
                board.Undo();
                mov.second = val;
                if (val > alpha) {
                    alpha = val;
                    foundPV = true;
                    //返回胜利节点
                    if (val == WON) {
                        break;
                    }
                }
            }
            std::stable_sort(moves.begin(), moves.end(), [](auto a, auto b) {
                return a.second > b.second;
            });
            bestMove = moves.front();
            {
                auto [i, j] = moves.front().first;
                cout << "最佳着法:" << i << " " << j << " "
                     << moves.front().second << " " << depth << endl;
            }
            if (bestMove.second == WON || bestMove.second == -WON) {
                break;
            }
        }
        return bestMove;
    }
    //带Alpha-Beta剪枝的Min-Max, 使用NegaMax简化形式
    //增加了置换表优化
    //增加了主要变例搜索优化
    int AlphaBeta(Board &board, int alpha, int beta, int depth) {
        // cout << alpha << "\t" << beta << endl;
        //命中缓存
        int maxVal = INT32_MIN;
        std::tuple<int, int> bestMove = {-1, -1};
        if (ttInfo tt = TT[board.Hash() % TT_SIZE];
            tt.key == board.Hash() &&
            (depth <= tt.depth || (tt.value == WON || tt.value == -WON))) {
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
            if (((beta - alpha != 1) && TT[key % TT_SIZE].depth <= dp) ||
                (value == WON || value == -WON)) {
                ++stat.tt_record_cnt;
                TT[key % TT_SIZE] = {key, dp, rel, value, mov};
            }
        };
        {  //正常结束
            //叶子节点
            if (depth == 0) {
                if (beta - alpha != 1) {
                    ++stat.leaf_cnt;
                }
                auto val = board.Evaluation();
                record_tt(board.Hash(), depth, RELATION_VAL::PV, val, bestMove);
                return val;
            }
            if (auto [val, ok] = board.Ending(); ok) {
                if (beta - alpha != 1) {
                    ++stat.leaf_cnt;
                }
                //局面终结
                record_tt(board.Hash(), depth, RELATION_VAL::PV, val, bestMove);
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
