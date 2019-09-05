#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include "search/solution.hpp"
using namespace std;
void show() {
    chis::Solution *slu = chis::MakeSolution(15);
    slu->Do(7, 7);
    slu->Do(6, 6);
    slu->Do(6, 8);
    slu->Do(8, 6);
    slu->Do(8, 8);
    // slu->Do(7,6);
    auto [v, status] = slu->Ending();
    cout << "ENDING " << v << endl;
    if (1) {
        auto ret = slu->Search(5);
        for (size_t i = 0; i < 3 && i < ret.size(); ++i) {
            auto [x, y] = ret[i].first;
            cout << x << "\t" << y << "\t" << ret[i].second << endl;
        }
        cout << "叶子节点" << slu->stat.leaf_cnt << endl
             << "胜利节点" << slu->stat.ending_cnt << endl
             << "置换表写入" << slu->stat.tt_record_cnt << endl
             << "置换表命中" << slu->stat.tt_hit_cnt << endl
             << "节点置换(end)" << slu->stat.tt_ending_pass_cnt << endl
             << "节点置换(pv)" << slu->stat.tt_pv_pass_cnt << endl
             << "节点置换(alpha)" << slu->stat.tt_alpha_pass_cnt << endl
             << "节点置换(beta)" << slu->stat.tt_beta_pass_cnt << endl
             << "最佳着法尝试" << slu->stat.bestmove_try_cnt << endl
             << "最佳着法剪枝" << slu->stat.bestmove_pass_cnt << endl
             << "主要变例搜索尝试" << slu->stat.pvs_try_cnt << endl
             << "主要变例搜索剪枝" << slu->stat.pvs_pass_cnt << endl
             << "延伸节点" << slu->stat.extend_try_cnt << endl
             << "延伸杀棋节点" << slu->stat.extend_ending_cnt << endl
             << "总节点数" << slu->stat.node_cnt << endl;
        static const std::string patternName[] = {
            "死棋", "眠一",  "活一",  "眠二", "活二A", "活二B", "活二C",
            "眠三", "活三A", "活三B", "眠四", "活四A", "活四B", "成五",
        };
        for (int i = 0; i < 14; ++i) {
            if (!slu->PatternInfo().pattern_cnt_blk[i] && !slu->PatternInfo().pattern_cnt_wht[i]) {
                continue;
            }
            cout << "GLOBAL PATTERN " << patternName[i];
            cout << " 黑:" << int(slu->PatternInfo().pattern_cnt_blk[i]);
            cout << "白:" << int(slu->PatternInfo().pattern_cnt_wht[i]) << std::endl;
        }
        slu->Show(15, {ret.front().first});
    }
}
void CLI() {
    chis::Solution *slu = chis::MakeSolution(15);
    cout << "输入1:代表玩家先走; 输入2:代表AI先走" << endl;
    int first;
    cin >> first;
    vector<tuple<int, int>> highlight;
    int score = 0;
    auto fresh = [&]() {
        system("clear");
        cout << "叶子节点" << slu->stat.leaf_cnt << endl
             << "胜利节点" << slu->stat.ending_cnt << endl
             << "置换表写入" << slu->stat.tt_record_cnt << endl
             << "置换表命中" << slu->stat.tt_hit_cnt << endl
             << "节点置换(end)" << slu->stat.tt_ending_pass_cnt << endl
             << "节点置换(pv)" << slu->stat.tt_pv_pass_cnt << endl
             << "节点置换(alpha)" << slu->stat.tt_alpha_pass_cnt << endl
             << "节点置换(beta)" << slu->stat.tt_beta_pass_cnt << endl
             << "最佳着法尝试" << slu->stat.bestmove_try_cnt << endl
             << "最佳着法剪枝" << slu->stat.bestmove_pass_cnt << endl
             << "主要变例搜索尝试" << slu->stat.pvs_try_cnt << endl
             << "主要变例搜索剪枝" << slu->stat.pvs_pass_cnt << endl
             << "延伸节点" << slu->stat.extend_try_cnt << endl
             << "延伸杀棋节点" << slu->stat.extend_ending_cnt << endl
             << "总节点数" << slu->stat.node_cnt << endl;
        static const std::string patternName[] = {
            "死棋", "眠一",  "活一",  "眠二", "活二A", "活二B", "活二C",
            "眠三", "活三A", "活三B", "眠四", "活四A", "活四B", "成五",
        };
        for (int i = 0; i < 14; ++i) {
            if (!slu->PatternInfo().pattern_cnt_blk[i] && !slu->PatternInfo().pattern_cnt_wht[i]) {
                continue;
            }
            cout << "GLOBAL PATTERN " << patternName[i];
            cout << " 黑:" << int(slu->PatternInfo().pattern_cnt_blk[i]);
            cout << "白:" << int(slu->PatternInfo().pattern_cnt_wht[i]) << std::endl;
        }
        slu->Show(15, highlight);
        cout << "Score:" << score << endl;
        highlight.clear();
    };
    while (true) {
        fresh();
        if(ABS(score) == chis::WON) {
            cin.get();
        }
        if (auto [val, status] = slu->Ending(); status == chis::GAME_STATUS::ENDING) {
            if ((val == chis::WON) == (slu->Turn() == chis::BOARD_VAL::BLK)) {
                cout << "对局结束 黑胜!" << endl;
            } else {
                cout << "对局结束 白胜!" << endl;
            }
            break;
        }
        //电脑对战 或者
        if(first > 2 || (first == 1) == (slu->Turn() == chis::BOARD_VAL::BLK) ){
            auto ret = slu->Search(6);
            for (size_t i = 0; i < 3 && i < ret.size(); ++i) {
                highlight.push_back(ret[i].first);
                auto [x, y] = ret[i].first;
                cout << x << "\t" << y << "\t" << ret[i].second << endl;
            }
            slu->Do(highlight.front());  //落子
            score = ret.front().second;
        } else  {
            int x, y;
            cin >> x >> y;
            if(x == -1 || y == -1) {
                slu->Undo();
                slu->Undo();
            } else {
                slu->Do(x, y);
            }
            score = -1;
        }
    }
}
int main() {
    CLI();
    return 0;
}