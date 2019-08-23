#pragma once
#include "board/types.h"
namespace chis {
    
//二维数组到一维数组的四种映射方法
//@i 二维数组横坐标
//@j 二维数组纵坐标
//@size   二维数组有效边长
//@offset 二维数组保护区宽度
//HENG →
//输入相对坐标
inline constexpr bsize_t HENG(const bsize_t i, const bsize_t j, const bsize_t size, const bsize_t offset = 0) {
    return (i+offset) * (size+offset*2) + (j+offset);
}
//输入绝对坐标
inline constexpr bsize_t HENG_REAL(const bsize_t i, const bsize_t j, const bsize_t size, const bsize_t offset = 0) {
    return HENG(i, j, size+offset*2);
}
//计算最大坐标
inline constexpr bsize_t HENG_MAXLEN(const bsize_t size, const bsize_t offset = 0) {
    return HENG(size+offset*2-1, size+offset*2-1, size+offset*2, 0)+1;
}
//SHU ↓
inline constexpr bsize_t SHU(const bsize_t i, const bsize_t j, const bsize_t size, const bsize_t offset = 0)  {
    return (j+offset) * (size+offset*2) + (i+offset);
}
//输入绝对坐标
inline constexpr bsize_t SHU_REAL(const bsize_t i, const bsize_t j, const bsize_t size, const bsize_t offset = 0) {
    return SHU(i, j, size+offset*2);
}
inline constexpr bsize_t SHU_MAXLEN(const bsize_t size, const bsize_t offset = 0) {
    return SHU(size+offset*2-1, size+offset*2-1, size+offset*2, 0)+1;
}
//PIE ↗
inline constexpr bsize_t PIE(const bsize_t i, const bsize_t j, const bsize_t size, const bsize_t offset = 0)  {
    return ( (i+offset) + (j+offset) ) * (size+offset*2) + (j+offset);
}
//输入绝对坐标
inline constexpr bsize_t PIE_REAL(const bsize_t i, const bsize_t j, const bsize_t size, const bsize_t offset = 0) {
    return PIE(i, j, size+offset*2);
}
inline constexpr bsize_t PIE_MAXLEN(const bsize_t size, const bsize_t offset = 0) {
    return PIE(size+offset*2-1, size+offset*2-1, size+offset*2, 0)+1;
}
//NA ↘
inline constexpr bsize_t NA(const bsize_t i, const bsize_t j, const bsize_t size, const bsize_t offset = 0)  {
    return ((size+offset*2)-1 + (i+offset) - (j+offset)) * (size+offset*2) + (j+offset);
}
//输入绝对坐标
inline constexpr bsize_t NA_REAL(const bsize_t i, const bsize_t j, const bsize_t size, const bsize_t offset = 0) {
    return NA(i, j, size+offset*2);
}
inline constexpr bsize_t NA_MAXLEN(const bsize_t size, const bsize_t offset = 0) {
    return NA(size+offset*2-1, -offset, size+offset*2, 0)+1;
}

}// namespace chis