#pragma once
#include "board/gomoku.hpp"
namespace chis {
enum RELATION_VAL : int {
    INVALID = 0,
    PV = 1,
    ALPHA = 2,
    BETA = 2,
    END = 3,
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
    size_t tt_record_ending_cnt = 0;
    size_t tt_hit_cnt = 0;
    size_t tt_ending_pass_cnt = 0;
    size_t tt_pv_pass_cnt = 0;
    size_t tt_alpha_pass_cnt = 0;
    size_t tt_beta_pass_cnt = 0;
    size_t bestmove_try_cnt = 0;
    size_t bestmove_pass_cnt = 0;
    size_t pvs_try_cnt = 0;
    size_t pvs_pass_cnt = 0;
    size_t node_cnt = 0;
};
using MovWithVal = vector_type<std::pair<std::tuple<int, int>, int>>;
using MovsTy = vector_type<std::tuple<int, int>>;
class Solution {
   public:
    virtual MovWithVal Search(const size_t MAX_DEPTH = 5,
                              const MovWithVal &recommend = {}) = 0;
    virtual void Do(int i, int j) = 0;
    void Do(MovsTy movs) {
        for (auto mov : movs) {
            auto [i, j] = mov;
            Do(i, j);
        }
    };
    virtual MovsTy GetDoChain() const = 0;
    virtual void Undo() = 0;
    virtual BOARD_VAL Get(int i, int j) const = 0;
    virtual void StopSearch() = 0;
    virtual void StartSearch() = 0;
    virtual bool IsStop() = 0;
    virtual void Reset(size_t MEM_BYTE = 128000000) = 0;
    virtual pattern_info PatternInfo() = 0;
    virtual std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GetPatternType(
        int i, int j) = 0;
    virtual std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> GetPattern(
        int i, int j) = 0;
    virtual int32_t Evaluation() = 0;
    virtual ~Solution(){};
    statInfo stat;
};
template <typename Board>
class solution : public Solution {
   public:
    solution(size_t MEM_BYTE = 128000000)
        : TT_SIZE(MEM_BYTE / sizeof(ttInfo)), TT(MEM_BYTE / sizeof(ttInfo)) {}

   public:  //以下方法都是线程不安全的，一个线程建议就搞一个实例
    virtual MovWithVal Search(const size_t MAX_DEPTH = 5,
                              const MovWithVal &recommend = {}) {
        StartSearch();
        MovWithVal moves;
        for (auto mov : board.Moves()) {
            moves.push_back({mov, -WON});
        }
        for (int depth = 0; depth <= MAX_DEPTH; ++depth) {
            for (auto &mov : moves) {
                auto [i, j] = mov.first;
                board.Do(i, j);  //落子
                int val;
                //如果已经找到PV, 则后续节点使用PVS
                val = -AlphaBeta(-WON, WON, depth);
                board.Undo();
                mov.second = val;
                if (val == WON) {
                    break;
                }
            }
            std::stable_sort(moves.begin(), moves.end(), [](auto a, auto b) {
                return a.second > b.second;
            });
            while (moves.size() > 1 && moves.back().second == -WON) {
                moves.pop_back();
            }
            auto &bestMove = moves.front();
            {
                auto [i, j] = bestMove.first;
                cout << "DEBUG "
                     << "最佳着法:" << i << " " << j << " " << bestMove.second
                     << " " << depth << endl;
            }
            if (bestMove.second == WON || bestMove.second == -WON ||
                moves.size() == 1) {
                break;
            }
        }
        return moves;
    }

   public:  //下面这些是线程安全的, 用来做超时控制，可以做到瞬间返回(截断)
    virtual void Do(int i, int j) { board.Do(i, j); }
    virtual void Undo() { board.Undo(); }
    virtual void StopSearch() {
        std::lock_guard<std::mutex> lg(ABPtrMtx);
        AlphaBetaPtr = &solution<Board>::AlphaBetaEnd;
    }
    virtual void StartSearch() {
        std::lock_guard<std::mutex> lg(ABPtrMtx);
        AlphaBetaPtr = &solution<Board>::AlphaBeta;
    }
    virtual bool IsStop() {
        return AlphaBetaPtr == &solution<Board>::AlphaBetaEnd;
    }
    virtual void Reset(size_t MEM_BYTE = 128000000) {
        board.Reset();
        TT_SIZE = MEM_BYTE / sizeof(ttInfo);
		{ 
			vector_type<ttInfo>().swap(TT);
		}    
        TT = vector_type<ttInfo>(TT_SIZE);
    }
    virtual pattern_info PatternInfo() { return board.PatternInfo(); }
    virtual MovsTy GetDoChain() const {
        MovsTy ret;
        for (auto d : board.doChain) {
            ret.push_back({d.i, d.j});
        }
        return ret;
    }
    virtual BOARD_VAL Get(int i, int j) const { return board.Get(i, j); }
    virtual std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GetPatternType(
        int i, int j) {
        return board.GetPatternType(i, j);
    }
    virtual std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> GetPattern(
        int i, int j) {
        return board.GetPattern(i, j);
    }
    virtual int32_t Evaluation() { return board.Evaluation(); }

   public:
    //带Alpha-Beta剪枝的Min-Max, 使用NegaMax简化形式
    //增加了置换表优化
    //增加了主要变例搜索优化
    int AlphaBeta(int alpha, int beta, int depth) {
        bool isZeroWin = ABS(beta - alpha) == 1;
        ++stat.node_cnt;
        //命中缓存
        int maxVal = INT32_MIN;
        std::tuple<int, int> bestMove = {-1, -1};
        if (ttInfo tt = TT[board.Hash() % TT_SIZE]; tt.key == board.Hash()) {
            ++stat.tt_hit_cnt;  //命中key
            //如果是结束，则直接选用
            if (tt.rel == RELATION_VAL::END) {
                ++stat.tt_ending_pass_cnt;
                return tt.value;
            }
            if (depth <= tt.depth) {
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
        }
        auto record_tt = [&](uint64_t key, int dp, RELATION_VAL rel, int value,
                             std::tuple<int, int> mov) {
            //深度优先替换
            if (TT[key % TT_SIZE].depth <= dp) {
                if (value == WON || value == -WON) {
                    ++stat.tt_record_ending_cnt;
                    TT[key % TT_SIZE] = {key, dp, RELATION_VAL::END, value,
                                         mov};
                } else if (!isZeroWin) {  //非零窗口
                    ++stat.tt_record_cnt;
                    TT[key % TT_SIZE] = {key, dp, rel, value, mov};
                }
            }
            // TODO 时效优先替换
        };
        {
            //叶子节点
            if (auto [val, end] = board.Ending(); end) {
                ++stat.leaf_cnt;
                ++stat.ending_cnt;
                //局面终结
                record_tt(board.Hash(), depth, RELATION_VAL::PV, val, bestMove);
                return val;
            }
            if (depth == 0) {
                ++stat.leaf_cnt;
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
                    val =
                        -(this->*AlphaBetaPtr)(-alpha - 1, -alpha, next_depth);
                    if (val > alpha && val < beta) {
                        val = -(this->*AlphaBetaPtr)(-beta, -alpha, next_depth);
                    } else {
                        ++stat.pvs_pass_cnt;
                    }
                } else {
                    val = -(this->*AlphaBetaPtr)(-beta, -alpha, next_depth);
                }
                if (val >= beta) {
                    return {val, true};
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
                record_tt(board.Hash(), depth, RELATION_VAL::BETA, maxVal,
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
                return maxVal;
            }
        }
        //着法生成,正常搜索
        for (auto mov : board.Moves()) {
            // beta节点 被截断
            bool out = search_move(mov, depth - 1);
            if (out) {
                return maxVal;
            }
        }
        //没有out, 更新置换表
        record_tt(board.Hash(), depth,
                  maxVal == alpha ? RELATION_VAL::PV : RELATION_VAL::ALPHA,
                  maxVal, bestMove);
        return maxVal;
    }
    int AlphaBetaEnd(int alpha, int beta, int depth) {
        return alpha;  //跳到叶节点
    }

   public:
    Board board;
    size_t TT_SIZE;
    vector_type<ttInfo> TT;
    std::mutex ABPtrMtx;
    int (solution<Board>::*AlphaBetaPtr)(int, int,
                                         int) = &solution<Board>::AlphaBeta;
};
Solution *MakeSolution(size_t size, size_t MEM_BYTE = 128000000) {
    switch (size) {
        case 10:
            return new solution<GomokuBoard<10>>(MEM_BYTE);
        case 15:
            return new solution<GomokuBoard<15>>(MEM_BYTE);
        case 20:
            return new solution<GomokuBoard<20>>(MEM_BYTE);
        default:
            throw(std::out_of_range("board size must in values(10,15,20)"));
    }
}
}  // namespace chis