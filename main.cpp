#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include "search/Solution.hpp"
using namespace std;
void show() {
    chis::Solution<chis::GomokuBoard<15, 5>> slu;
    chis::GomokuBoard<15, 5> board;
    board.Do(7, 7);
    board.Do(1, 1);
    board.Do(7, 8);
    board.Do(13, 13);
    board.Do(7, 6);
    board.Do(13, 1);
    // board.Do(7, 5);
    // board.Do(1, 13);
    {
        auto ret = slu.Search(board);
        auto [i, j] = ret.first;
        cout << i << "\t" << j << "\t" << ret.second << endl;
        cout << "叶子节点" << slu.stat.leaf_cnt << endl
             << "置换表写入" << slu.stat.tt_record_cnt << endl
             << "节点置换(主要变例)" << slu.stat.tt_pv_pass_cnt << endl
             << "节点置换(alpha)" << slu.stat.tt_alpha_pass_cnt << endl
             << "节点置换(beta)" << slu.stat.tt_beta_pass_cnt << endl
             << "最佳着法剪枝" << slu.stat.bestmove_pass_cnt << endl
             << "主要变例搜索剪枝" << slu.stat.pvs_pass_cnt << endl
             << "主要变例搜索剪枝(根节点)" << slu.stat.pvs_pass_root_cnt
             << endl;
    }
}
int main() {
    show();
    return 0;
}