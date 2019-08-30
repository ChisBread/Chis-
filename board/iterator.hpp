#pragma once
#include "board/types.h"
namespace chis {
inline std::tuple<bsize_t, bsize_t> HENG_NEXT(const bsize_t i, const bsize_t j,
                                              const int n) {
    return {i, j + n};
}
inline std::tuple<bsize_t, bsize_t> SHU_NEXT(const bsize_t i, const bsize_t j,
                                             const int n) {
    return {i + n, j};
}
inline std::tuple<bsize_t, bsize_t> PIE_NEXT(const bsize_t i, const bsize_t j,
                                             const int n) {
    //↗
    return {i - n, j + n};
}
inline std::tuple<bsize_t, bsize_t> NA_NEXT(const bsize_t i, const bsize_t j,
                                            const int n) {
    //↘
    return {i + n, j + n};
}
}  // namespace chis