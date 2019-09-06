#pragma once
#include "board/gomoku.hpp"
namespace chis {
enum RELATION_VAL : int {
    INVALID = 0,
    PV = 1,
    ALPHA = 2,
    BETA = 2,
    END = 3,  //也是一种beta节点
};
//置换表信息
struct ttInfo {
    // zobrist hash key
    uint64_t key = 0;
    //节点向下搜索的深度
    int depth = 0;
    //节点和（存储时的）窗口的关系
    RELATION_VAL rel = RELATION_VAL::INVALID;
    //节点的真实值
    int value = 0;
    //节点的最好着法
    std::tuple<int, int> bestMove = {-1, -1};
};
struct dttInfo {
    ttInfo d[2];//double ttinfo 0:deepest 1:newest
};
//统计信息
struct statInfo {
    size_t leaf_cnt = 0;              //叶节点计数
    size_t node_cnt = 0;              //总结点计数
    size_t ending_cnt = 0;            //终结局面计数
    size_t tt_record_cnt = 0;         //置换表写入
    size_t tt_record_ending_cnt = 0;  //置换表写入终结局面
    size_t tt_hit_cnt = 0;            //置换表命中（不一定截断）
    size_t tt_ending_pass_cnt = 0;    //置换表截断 终结局面
    size_t tt_pv_pass_cnt = 0;        //置换表截断 pv节点(真实值)
    size_t tt_alpha_pass_cnt = 0;     //置换表截断 alpha节点
    size_t tt_beta_pass_cnt = 0;      //置换表截断 beta节点
    size_t bestmove_try_cnt = 0;      // bestmove尝试
    size_t bestmove_pass_cnt = 0;     // bestmove截断
    size_t pvs_try_cnt = 0;           //主要变例搜索计数
    size_t pvs_pass_cnt = 0;          //主要变例剪枝
    size_t extend_try_cnt = 0;        //延伸节点计数
    size_t extend_ending_cnt = 0;     //延伸杀局计数
};
//带分数的着法串
using MovWithVal = vector_type<std::pair<std::tuple<int, int>, int>>;
//着法串
using MovsTy = vector_type<std::tuple<int, int>>;
class Solution {
   public:
    //搜索推荐着法序列(带估值)
    virtual MovWithVal Search(const size_t MAX_DEPTH = 6) = 0;
    //搜索推荐着法序列(带估值) 增加超时选项
    MovWithVal Search(const int timeout_turn, const size_t MAX_DEPTH) {
        auto rets_future = std::async(std::launch::async, [&]() { return Search(MAX_DEPTH); });
        std::future_status status;
        do {
            status = rets_future.wait_for(std::chrono::milliseconds(timeout_turn > 50 ? timeout_turn - 50 : timeout_turn));
            if (status == std::future_status::deferred) {
                StopSearch();
            } else if (status == std::future_status::timeout) {
                StopSearch();
            }
        } while (status != std::future_status::ready);
        if (IsStop()) {
            StartSearch();
        }
        return rets_future.get();
    }
    //停止搜索 原理:AlphaBeta函数指针切换到AlphaBetaEnd
    //使用: 异步+超时控制
    virtual void StopSearch() = 0;
    //开始搜索 把函数指针切回来
    virtual void StartSearch() = 0;
    //判断函数指针是否为正常搜索函数
    virtual bool IsStop() = 0;

   public:
    //得到落子方颜色
    virtual BOARD_VAL Turn() const = 0;
    //落子
    virtual void Do(int i, int j) = 0;
    virtual void Do(std::tuple<int, int> mov) {
        auto [i, j] = mov;
        Do(i, j);
    }
    //连续落子
    void Do(MovsTy movs) {
        for (auto mov : movs) {
            auto [i, j] = mov;
            Do(i, j);
        }
    };
    //撤销落子
    virtual void Undo() = 0;
    //重设棋盘(包括落子、内存等)
    virtual void Reset(size_t MEM_BYTE = 128000000) = 0;

   public:  //信息
    //棋型信息
    virtual GomokuPatternInfo PatternInfo() const = 0;
    //已落子信息
    virtual MovsTy GetDoChain() const = 0;
    //单点值
    virtual BOARD_VAL Get(int i, int j) const = 0;
    //在标准输出中输出棋盘
    void Show(int size, MovsTy highlight) const {
        set_type<int> highlightSet;
        for (auto [x, y] : highlight) {
            highlightSet.insert(x * size + y);
        }
        cout << "   ";
        for (int i = 0; i < size; ++i) {
            cout << (i < 10 ? "0" : "") << i << " ";
        }
        cout << endl;
        for (int i = 0; i < size; ++i) {
            cout << (i < 10 ? "0" : "") << i << " ";
            for (int j = 0; j < size; ++j) {
                auto [x, y] = GetDoChain().empty() ? std::tuple{-1, -1} : GetDoChain().back();
                BOARD_VAL v = Get(i, j);
                std::string s = i == x && j == y ? "^" : " ";
                std::string highlightStr = highlightSet.count(i * size + j) ? "!" : " ";
                switch (v) {
                    case BOARD_VAL::BLK:
                        cout << s << "O" << s;
                        break;
                    case BOARD_VAL::WHT:
                        cout << s << "X" << s;
                        break;
                    case BOARD_VAL::EMP:
                        cout << "[" << highlightStr << "]";
                        break;
                    default:
                        break;
                }
            }
            cout << endl;
        }
    }
    //得到生成的着法
    virtual MovsTy Moves(bool must = false) = 0;
    //估值 (NegaMax)
    virtual int32_t Evaluation() = 0;
    //判断局面是否终结，终结的话打个分 (NegaMax)
    virtual std::tuple<int, GAME_STATUS> Ending() = 0;
    //获得棋型
    virtual std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> GetPattern(int i, int j) = 0;
    //获得棋型类型
    virtual std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GetPatternType(int i, int j) = 0;
    virtual ~Solution(){};
    //统计信息
    statInfo stat;
};
template <typename Board>
class solution : public Solution {
   public:
    solution(size_t MEM_BYTE = 128000000) : TT_SIZE(MEM_BYTE / sizeof(dttInfo)), TT(MEM_BYTE / sizeof(dttInfo)) {}

   public:  //以下方法都是线程不安全的，一个线程建议就搞一个实例
    //搜索算法: Min-Max
    //剪枝+启发: AlphaBeta+置换表(单置换表 BestMove)+迭代加深+PVS
    virtual MovWithVal Search(const size_t MAX_DEPTH = 6) {
        StartSearch();
        MovWithVal moves;
        for (auto mov : board.Moves()) {
            moves.push_back({mov, -WON});
        }
        for (int depth = 0; depth <= MAX_DEPTH; ++depth) {
            int alpha = -WON, beta = WON;
            for (auto &mov : moves) {
                if (IsStop()) {
                    return moves;
                }
                auto [i, j] = mov.first;
                board.Do(i, j);  //落子
                int val;
                val = -AlphaBeta(-beta, -alpha, depth);
                board.Undo();
                mov.second = val;
                if (val >= beta) {
                    break;
                }
                if (val > alpha) {
                    alpha = val;
                }
            }
            std::stable_sort(moves.begin(), moves.end(), [](auto a, auto b) { return a.second > b.second; });
            while (moves.size() > 1 && moves.back().second == -WON) {
                moves.pop_back();
            }
            auto &bestMove = moves.front();
            {
                auto [i, j] = bestMove.first;
                cout << "DEBUG "
                     << "最佳着法:" << i << " " << j << " " << bestMove.second << " " << depth << endl;
            }
            if (bestMove.second == WON || bestMove.second == -WON || moves.size() == 1) {
                break;
            }
        }
        return moves;
    }
    int AlphaBetaEnd(int alpha, int beta, int depth, int force) {
        return alpha;  //跳到叶节点
    }
    int AlphaBeta(int alpha, int beta, int depth, int force = 0) {
        bool isZeroWin = ABS(beta - alpha) == 1;
        ++stat.node_cnt;
        if (force) {
            ++stat.extend_try_cnt;
        }
        //命中缓存
        int maxVal = INT32_MIN;
        std::tuple<int, int> bestMove = {-1, -1};
        dttInfo dtt = TT[board.Hash() % TT_SIZE];
        for (int i = 0; i < 2; ++i) {
            ttInfo tt = dtt.d[i];
            if (tt.key != board.Hash()) {
                continue;
            }
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
                } else if (tt.rel == RELATION_VAL::ALPHA && tt.value <= alpha) {  //有效窗口值
                    //如果是辣鸡节点，直接返回alpha
                    ++stat.tt_alpha_pass_cnt;
                    return alpha;
                } else if (tt.rel == RELATION_VAL::BETA && tt.value >= beta) {  //有效窗口值
                    //如果是厉害节点，直接返回beta
                    ++stat.tt_beta_pass_cnt;
                    return beta;
                }
                if (auto [i, j] = tt.bestMove; i != -1 && j != -1) {
                    bestMove = tt.bestMove;
                }
            }
        }
        
        auto record_tt = [&](uint64_t key, int dp, RELATION_VAL rel, int value, std::tuple<int, int> mov) {
            //深度优先替换
            if (value == WON || value == -WON) {
                ++stat.tt_record_ending_cnt;
                TT[key % TT_SIZE].d[1] = {key, dp, RELATION_VAL::END, value, mov};
                if (TT[key % TT_SIZE].d[0].depth <= dp) {
                    ++stat.tt_record_ending_cnt;
                    TT[key % TT_SIZE].d[0] = {key, dp, RELATION_VAL::END, value, mov};
                }
            } else if (!isZeroWin) {  //非零窗口
                ++stat.tt_record_cnt;
                TT[key % TT_SIZE].d[1] = {key, dp, rel, value, mov};
                if (TT[key % TT_SIZE].d[0].depth <= dp) {
                    ++stat.tt_record_cnt;
                    TT[key % TT_SIZE].d[0] = {key, dp, rel, value, mov};
                }
            }
            // TODO 时效优先替换
        };
        {//叶子节点
            auto [val, status] = board.Ending();
            if (status == GAME_STATUS::ENDING) {
                if (force) {
                    ++stat.extend_ending_cnt;
                }
                ++stat.leaf_cnt;
                ++stat.ending_cnt;
                //局面终结
                record_tt(board.Hash(), depth, RELATION_VAL::PV, val, bestMove);
                return val;
            }
            if (depth <= 0) {  //终结或者延伸
                switch (status) {
                    case GAME_STATUS::DEFEND:  //防守延伸
                        force = 1;
                        break;
                     case GAME_STATUS::ATTACK:  //进攻延伸
                        force = 1;
                        break;
                    default:
                        force = 0;  //停止延伸
                        break;
                }
                if (force == 0 || depth < -2) {  //强制搜索最多一层
                    ++stat.leaf_cnt;
                    int val = board.Evaluation()+depth*50;
                    return val;
                }
            }
        }

        //搜索一个着法，如果返回在窗口内，则更新alpha.
        //如果在窗口外，则更新置换表，返回out
        bool foundPV = false;
        auto search_move = [&](std::tuple<int, int> mov, int next_depth) -> bool {
            auto [i, j] = mov;
            board.Do(i, j);  //落子
            //搜索操作，返回估值和是否在窗口内
            auto [val, out] = [&]() -> std::tuple<int, bool> {
                int val;
                //如果已经找到PV, 则后续节点使用PVS
                if (foundPV) {
                    ++stat.pvs_try_cnt;
                    val = -(this->*AlphaBetaPtr)(-alpha - 1, -alpha, next_depth, force);
                    if (val > alpha && val < beta) {
                        val = -(this->*AlphaBetaPtr)(-beta, -alpha, next_depth, force);
                    } else {
                        ++stat.pvs_pass_cnt;
                    }
                } else {
                    val = -(this->*AlphaBetaPtr)(-beta, -alpha, next_depth, force);
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
                record_tt(board.Hash(), depth, RELATION_VAL::BETA, maxVal, bestMove);
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
        record_tt(board.Hash(), depth, maxVal == alpha ? RELATION_VAL::PV : RELATION_VAL::ALPHA, maxVal, bestMove);
        return maxVal;
    }
    ///////////////////////////////////////////////////////
    virtual void StopSearch() {
        std::lock_guard<std::mutex> lg(ABPtrMtx);
        AlphaBetaPtr = &solution<Board>::AlphaBetaEnd;
    }
    virtual void StartSearch() {
        std::lock_guard<std::mutex> lg(ABPtrMtx);
        AlphaBetaPtr = &solution<Board>::AlphaBeta;
    }
    virtual bool IsStop() { return AlphaBetaPtr == &solution<Board>::AlphaBetaEnd; }

   public:  //下面这些是线程安全的, 用来做超时控制，可以做到瞬间返回(截断)
    virtual BOARD_VAL Turn() const { return board.Turn(); }
    virtual void Do(int i, int j) { board.Do(i, j); }
    virtual void Undo() { board.Undo(); }
    virtual std::tuple<int, GAME_STATUS> Ending() { return board.Ending(); }
    virtual void Reset(size_t MEM_BYTE = 128000000) {
        board.Reset();
        TT_SIZE = MEM_BYTE / sizeof(dttInfo);
        { vector_type<dttInfo>().swap(TT); }
        TT = vector_type<dttInfo>(TT_SIZE);
    }

   public:
    virtual GomokuPatternInfo PatternInfo() const { return board.PatternInfo(); }
    virtual MovsTy GetDoChain() const {
        MovsTy ret;
        for (auto d : board.doChain) {
            ret.push_back({d.i, d.j});
        }
        return ret;
    }
    virtual BOARD_VAL Get(int i, int j) const { return board.Get(i, j); }
    virtual MovsTy Moves(bool must = false) { return board.Moves(must); }
    virtual int32_t Evaluation() { return board.Evaluation(); }
    virtual std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> GetPattern(int i, int j) { return board.GetPattern(i, j); }
    virtual std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GetPatternType(int i, int j) { return board.GetPatternType(i, j); }

   public:
    /////////棋盘+知识///////
    Board board;
    /////////置换表//////////
    size_t TT_SIZE;
    vector_type<dttInfo> TT;
    ///////锁与搜索指针///////
    std::mutex ABPtrMtx;
    int (solution<Board>::*AlphaBetaPtr)(int, int, int, int) = &solution<Board>::AlphaBeta;
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