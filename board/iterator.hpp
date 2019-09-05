#pragma once
#include "board/types.h"
namespace chis {
inline std::tuple<int, int> HENG_NEXT(const int i, const int j, const int n) { return {i, j + n}; }
inline std::tuple<int, int> SHU_NEXT(const int i, const int j, const int n) { return {i + n, j}; }
inline std::tuple<int, int> PIE_NEXT(const int i, const int j, const int n) {
    //↗
    return {i - n, j + n};
}
inline std::tuple<int, int> NA_NEXT(const int i, const int j, const int n) {
    //↘
    return {i + n, j + n};
}
using NextFunc = std::tuple<int, int> (*)(const int i, const int j, const int n);
static NextFunc Nexts[] = {HENG_NEXT, SHU_NEXT, PIE_NEXT, NA_NEXT};

//将横向坐标映射到特定方向的数组上, tuple(0)代表行，tuple(1)代表列
inline std::tuple<int, int> HENG_MAPING(const int i, const int j, const int size) { return {i, j}; }
inline std::tuple<int, int> SHU_MAPING(const int i, const int j, const int size) { return {j, i}; }
inline std::tuple<int, int> PIE_MAPING(const int i, const int j, const int size) {
    /*
            [0]  [1]  [2]
        [0] 0,0, 0,1, 0,2
        [1] 1,0, 1,1, 1,2
        [2] 2,0, 2,1, 2,2
        TO
            [0]  [1]  [2]
        [0] 0,0
        [1] 1,0, 0,1
        [2] 2,0, 1,1, 0,2
        [3]      2,1, 1,2
        [4]           2,2
    */
    return {i + j, j};
}
inline std::tuple<int, int> NA_MAPING(const int i, const int j, const int size) {
    /*
            [0]  [1]  [2]
        [0] 0,0, 0,1, 0,2
        [1] 1,0, 1,1, 1,2
        [2] 2,0, 2,1, 2,2
        TO
            [0]  [1]  [2]
        [0] 2,0
        [1] 1,0, 2,1
        [2] 0,0, 1,1, 2,2
        [3]      0,1, 1,2
        [4]           0,2
    */
    return {size - (i - j) - 1, j};
}
using MappingFunc = std::tuple<int, int> (*)(const int i, const int j, const int size);
static MappingFunc Mappings[] = {HENG_MAPING, SHU_MAPING, PIE_MAPING, NA_MAPING};

}  // namespace chis