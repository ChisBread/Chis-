#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include "search/solution.hpp"
using namespace std;
void show() {
    chis::Solution *slu = chis::MakeSolution(15);
    slu->Do(7, 7);slu->Do(1, 1);
    slu->Do(7, 8);slu->Do(13, 13);
    //slu->Do(7, 6);slu->Do(7, 9);
    //slu->Do(7, 5);
    // slu->Do(1, 13);
    //slu->Evaluation();
    if(1){
        auto ret = slu->Search();
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
             << "总节点数" << slu->stat.node_cnt << endl;
        static const std::string patternName[] = {
            "死棋", "眠一",  "活一",  "眠二", "活二A", "活二B", "活二C",
            "眠三", "活三A", "活三B", "眠四", "活四A", "活四B", "成五",
        };
        for (int i = 0; i < 14; ++i) {
            if (!slu->PatternInfo().pattern_cnt_blk[i] &&
                !slu->PatternInfo().pattern_cnt_wht[i]) {
                continue;
            }
            cout << "GLOBAL PATTERN " << patternName[i];
            cout << " 黑:" << slu->PatternInfo().pattern_cnt_blk[i];
            cout << "白:" << slu->PatternInfo().pattern_cnt_wht[i] << std::endl;
        }
    }
}
int main() {
    show();
    return 0;
}