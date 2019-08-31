#pragma once
#include "board/types.h"
namespace chis {
inline std::tuple<int, int> HENG_NEXT(const int i, const int j, const int n) {
    return {i, j + n};
}
inline std::tuple<int, int> SHU_NEXT(const int i, const int j, const int n) {
    return {i + n, j};
}
inline std::tuple<int, int> PIE_NEXT(const int i, const int j, const int n) {
    //↗
    return {i - n, j + n};
}
inline std::tuple<int, int> NA_NEXT(const int i, const int j, const int n) {
    //↘
    return {i + n, j + n};
}
}  // namespace chis