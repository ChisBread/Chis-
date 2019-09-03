#pragma once
#include "board/iterator.hpp"
#include "board/types.h"
#include "resource/patterns.h"
namespace chis {
//五子棋位棋盘-基本操作
//64位代表一行可以放32个棋子，当offset为5时可以放22个
using bitline_t = uint64_t;
struct row_node;
struct mapping_node;
struct root_node;
//属性 
struct col_node {
    int y = 0;//行上的偏移
    //正常mapping,不带offset的x,y和对应位置的四线棋型
    std::tuple<int,int> move;
    col_node *prior = nullptr, *next = nullptr;
    row_node *parent = nullptr;//
};
struct row_node {
    int x = 0;//方向上的行
    row_node *prior = nullptr, *next = nullptr;
    col_node *col_root = nullptr;
    mapping_node *parent = nullptr;
};
struct mapping_node {
    int k;//0~3 横竖撇捺
    mapping_node *prior = nullptr, *next = nullptr;
    row_node *row_root = nullptr;
    pattern_node *parent = nullptr;
};
struct pattern_node {
    PAT_TYPE pty;
    int count = 0;
    pattern_node *prior = nullptr, *next = nullptr;
    mapping_node *mapping_root = nullptr;//4个方向
    root_node *parent = nullptr;
};
struct root_node {
    BOARD_VAL val;//黑白棋型
    pattern_node *patterns_root = nullptr;//14个棋型
};

template <size_t size = 15, size_t offset = 5>
class GomokuMoves {
public:
    GomokuMoves() {
        for(int v = -1; v < 2; v+=2) {
            root_node &root = roots[v];
            root.val = v;
            for(int p = 0; p < 14; ++p) {
                pattern_node &pattern = pattern_pool[v][p];
                pattern.pty = p;//值
                pattern.parent = &root;//父节点
                pattern_prior = &pattern;
                for(int k = 0; k < 4; ++k) {
                    mapping_node &mapping = mapping_pool[v][p][k];
                    mapping.k = k;//值
                    mapping.parent = &pattern;//父节点
                    mapping_prior = &mapping;
                    for(int i = 0; i < size; ++i) {
                        for(int j = 0; j < size; ++j) {
                            auto[x,y] = Mappings[k](i+offfset, j+offset);
                            row_node &row = row_pool[v][p][k][x];
                            //初始化
                            row.x = x;//值
                            row.parent = &mapping;//父节点
                            if(row_prior != &row) {
                                row_prior = &row;
                            }
                            col_node &col = col_pool[v][p][k][x][y];
                            col.y = y;//值
                            col.move = {i, j};//着法
                            col.parent = &row;//父节点
                            col_prior = &col;
                        }
                    }
                }
            }
        }
    }
public:
    //落子方、棋型、方向、点
    void Insert(BOARD_VAL val, PAT_TYPE pty, int k, std::tuple<int,int> move) {
        root_node &root = roots[v];
        pattern_node &pattern = pattern_pool[v][pty];
        mapping_node &mapping = mapping_pool[v][pty][k];
        auto[i,j] = move;
        auto[x,y] = Mappings[k](i+offfset, j+offset);
        row_node &row = row_pool[v][pty][k][x];
        col_node &col = col_pool[v][p][k][x][y];
        col.prior = col.next = nullptr;
        //插入链表
    }
    //删除点 
    void Delete(BOARD_VAL val, PAT_TYPE pty, int k,std::tuple<int,int> move) {

    }
public:
    //3:颜色最大值为0b10 冗余一个
    //14:每个颜色14种棋型
    //4:每种棋型有四个方向
    //(size+offset*2)*2: 每个方向上最多(size+offset*2)*2行
    //size+offset*2: 每行最多size+offset*2列
    //反向索引 值,列映射->行映射->方向->棋型->颜色
    col_node col_pool[3][14][4][(size+offset*2)*2][size+offset*2];
    row_node row_pool[3][14][4][(size+offset*2)*2];
    mapping_node mapping_pool[3][14][4];
    pattern_node pattern_pool[3][14];
    //正向索引 颜色->棋型->方向->行映射->列映射,值
    root_node roots[3];//x in (1,2) y in(1,13)
};
}  // namespace chis