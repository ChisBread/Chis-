#pragma once
#include "board/iterator.hpp"
#include "board/types.h"
#include "resource/patterns.h"
namespace chis {
//五子棋位棋盘-基本操作
//64位代表一行可以放32个棋子，当offset为5时可以放22个
using bitline_t = uint64_t;
struct col_node {
    int y = -1;//行上的偏移
    //正常mapping,不带offset的x,y和对应位置的四线棋型
    std::pair<std::tuple<int,int>, PattternsTuple> move;
};
struct row_node {
    int x = -1;//方向上的行
    list_type<col_node> cols;//0~size
};
struct mapping_node {
    int k;//0~4 横竖撇捺
    list_type<row_node> rows;
};
struct pattern_node {
    PAT_TYPE type;
    mapping_node mappings[4];//4个方向
};
template <size_t size = 15, size_t offset = 5>
class GomokuMoves {
public:
public:

};
}  // namespace chis