#pragma once
#include "board/types.h"
namespace chis {

//二维数组到一维数组的四种映射方法
//@i 二维数组横坐标
//@j 二维数组纵坐标
//@size   二维数组有效边长
//@offset 二维数组保护区宽度
// HENG →
// SHU ↓
// PIE ↗
// NA ↘
//*_REAL 输入考虑offset的绝对坐标 / 否则输入有效的相对坐标

inline constexpr size_t HENG(const int i, const int j, const size_t size,
                             const size_t offset = 0) {
    return (i + offset) * (size + offset * 2) + (j + offset);
}
inline constexpr size_t HENG_REAL(const int i, const int j, const size_t size,
                                  const size_t offset = 0) {
    return HENG(i, j, size + offset * 2);
}
inline constexpr size_t HENG_MAXLEN(const size_t size,
                                    const size_t offset = 0) {
    return HENG(size + offset * 2 - 1, size + offset * 2 - 1, size + offset * 2,
                0) +
           1;
}
inline constexpr size_t SHU(const int i, const int j, const size_t size,
                            const size_t offset = 0) {
    return (j + offset) * (size + offset * 2) + (i + offset);
}
inline constexpr size_t SHU_REAL(const int i, const int j, const size_t size,
                                 const size_t offset = 0) {
    return SHU(i, j, size + offset * 2);
}
inline constexpr size_t SHU_MAXLEN(const size_t size, const size_t offset = 0) {
    return SHU(size + offset * 2 - 1, size + offset * 2 - 1, size + offset * 2,
               0) +
           1;
}
inline constexpr size_t PIE(const int i, const int j, const size_t size,
                            const size_t offset = 0) {
    return ((i + offset) + (j + offset)) * (size + offset * 2) + (j + offset);
}
inline constexpr size_t PIE_REAL(const int i, const int j, const size_t size,
                                 const size_t offset = 0) {
    return PIE(i, j, size + offset * 2);
}
inline constexpr size_t PIE_MAXLEN(const size_t size, const size_t offset = 0) {
    return PIE(size + offset * 2 - 1, size + offset * 2 - 1, size + offset * 2,
               0) +
           1;
}
inline constexpr size_t NA(const int i, const int j, const size_t size,
                           const size_t offset = 0) {
    return ((size + offset * 2) - 1 + (i + offset) - (j + offset)) *
               (size + offset * 2) +
           (j + offset);
}
inline constexpr size_t NA_REAL(const int i, const int j, const size_t size,
                                const size_t offset = 0) {
    return NA(i, j, size + offset * 2);
}
inline constexpr size_t NA_MAXLEN(const size_t size, const size_t offset = 0) {
    return NA(size + offset * 2 - 1, -offset, size + offset * 2, 0) + 1;
}

}  // namespace chis