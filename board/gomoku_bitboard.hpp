#pragma once
#include "board/iterator.hpp"
#include "board/types.h"
namespace chis {
//五子棋位棋盘-基本操作
//64位代表一行可以放32个棋子，当offset为5时可以放22个
using bitline_t = uint64_t;
template <size_t size = 15, size_t offset = 5>
class GomokuBitBoard {
   public:  // required method
    //相对坐标
    void Set(int i, int j, const BOARD_VAL val) {
        SetReal(i+offset, j+offset, val);
    }
    void Reset(int i, int j) {
        ResetReal(i+offset, j+offset);
    }
    BOARD_VAL Get(int i, int j) const { 
        return GetReal(i+offset, j+offset);
    }
    //绝对坐标
    void SetReal(int i, int j, const BOARD_VAL val) { 
        for(int k = 0; k < 4; ++k) {
            auto[x, y] = Mappings[k](i, j, size+offset*2);
            board[k][x] &= ~(bitline_t(0x3U) << y*2);//mask
            board[k][x] |= val << y*2;
        }
    }
    void ResetReal(int i, int j) { 
        for(int k = 0; k < 4; ++k) {
            auto[x, y] = Mappings[k](i, j, size+offset*2);
            board[k][x] &= ~(bitline_t(0x3U) << y*2);//mask
        }
    }
    BOARD_VAL GetReal(int i, int j) const { 
        auto[x, y] = Mappings[0](i, j, size+offset*2);
        return BOARD_VAL((board[0][x] >> y*2) & 0x3U);//lmov and mask
    }
    uint32_t GetPattern(int i, int j, int k) const {
        auto[x, y] = Mappings[k](i+offset, j+offset, size+offset*2);
        return (board[k][x] >> (y-5)*2) & 0x3FFFFF;//(由于有offset的存在，y至少是5)
    }
    //i,j is not real!
    PattternsTuple GetPattern(int i, int j) const {
        uint32_t ret[4] = {};
        for(int k = 0; k < 4; ++k) {
            auto[x, y] = Mappings[k](i+offset, j+offset, size+offset*2);
            ret[k] = (board[k][x] >> (y-5)*2) & 0x3FFFFF;//(由于有offset的存在，y至少是5)
        }
        return {ret[0], ret[1], ret[2], ret[3]};
    }
    //找到左右两边第一个非空点
    array_type<array_type<uint32_t,4>,2> GetEmptyPointPattern(int i, int j) const {
        uint32_t pats[4] = {};
        std::tie(pats[0], pats[1], pats[2], pats[3]) = GetPattern(i, j);
        array_type<array_type<uint32_t,4>,2> ret = {};
        for(int k = 0; k < 4; ++k) {
            for(int neg = -1; neg < 2; neg += 2) {
                for(int l = 1; l < 6; ++l) {
                    BOARD_VAL side = BOARD_VAL((pats[k] >> (5+l*neg)*2)&0x3U);//两个方向
                    if(side != BOARD_VAL::EMP) {
                        auto[xn, yn] = Nexts[k](i,j,l*neg);//正常节点偏移
                        auto[x, y] = Mappings[k](xn+offset, yn+offset, size+offset*2);//转成bit坐标
                        ret[neg==1?0:1][k] = (board[k][x] >> (y-5)*2) & 0x3FFFFF;//mask 得到棋型
                        break;
                    }
                }
            }
        }
        return ret;
    }
   public:
    GomokuBitBoard() {
        static_assert(offset>=5);
        static_assert(size+offset*2<32);
        for(int i = 0; i < size+offset*2; ++i) {
            for(int j = 0; j < size+offset*2; ++j) {
                //越界
                if(i < offset || i >= size+offset || 
                   j < offset || j >=size+offset ) {
                    SetReal(i, j, BOARD_VAL::INV);
                } else {
                    ResetReal(i, j);
                }
            }
        }
    }
   public:
    //横竖撇捺四个方向的棋盘 每个值代表这个方向上的行
    bitline_t board[4][(size+offset*2)*2] = {};
};
}  // namespace chis