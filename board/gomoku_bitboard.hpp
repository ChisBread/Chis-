#pragma once
#include "board/mappings.hpp"
#include "board/types.h"
namespace chis {
//五子棋位棋盘-基本操作
template <bsize_t size = 15, bsize_t offset = 5>
class GomokuBitBoard {
   public:
    //相对坐标
    void set(bsize_t i, bsize_t j, const bcell_t &val) {
        heng[HENG(i, j, size, offset) * 2] = val[0];
        shu[SHU(i, j, size, offset) * 2] = val[0];
        pie[PIE(i, j, size, offset) * 2] = val[0];
        na[NA(i, j, size, offset) * 2] = val[0];
        heng[HENG(i, j, size, offset) * 2 + 1] = val[1];
        shu[SHU(i, j, size, offset) * 2 + 1] = val[1];
        pie[PIE(i, j, size, offset) * 2 + 1] = val[1];
        na[NA(i, j, size, offset) * 2 + 1] = val[1];
    }
    bcell_t get(bsize_t i, bsize_t j) const {
        return BOARD_VAL(
            (bsize_t(heng[HENG(i, j, size, offset) * 2 + 1]) << 1) +
            bsize_t(heng[HENG(i, j, size, offset) * 2]));
    }
    //绝对坐标
    void set_real(bsize_t i, bsize_t j, const bcell_t &val) {
        heng[HENG_REAL(i, j, size, offset) * 2] = val[0];
        shu[SHU_REAL(i, j, size, offset) * 2] = val[0];
        pie[PIE_REAL(i, j, size, offset) * 2] = val[0];
        na[NA_REAL(i, j, size, offset) * 2] = val[0];
        heng[HENG_REAL(i, j, size, offset) * 2 + 1] = val[1];
        shu[SHU_REAL(i, j, size, offset) * 2 + 1] = val[1];
        pie[PIE_REAL(i, j, size, offset) * 2 + 1] = val[1];
        na[NA_REAL(i, j, size, offset) * 2 + 1] = val[1];
    }
    bcell_t get_real(bsize_t i, bsize_t j) const {
        return BOARD_VAL(
            (bsize_t(heng[HENG_REAL(i, j, size, offset) * 2 + 1]) << 1) +
            bsize_t(heng[HENG_REAL(i, j, size, offset) * 2]));
    }
    array_type<bcell_t, 4> get_real_all(bsize_t i, bsize_t j) const {
        return {
            BOARD_VAL(
                (bsize_t(heng[HENG_REAL(i, j, size, offset) * 2 + 1]) << 1) +
                bsize_t(heng[HENG_REAL(i, j, size, offset) * 2])),
            BOARD_VAL(
                (bsize_t(shu[SHU_REAL(i, j, size, offset) * 2 + 1]) << 1) +
                bsize_t(shu[SHU_REAL(i, j, size, offset) * 2])),
            BOARD_VAL(
                (bsize_t(pie[PIE_REAL(i, j, size, offset) * 2 + 1]) << 1) +
                bsize_t(pie[PIE_REAL(i, j, size, offset) * 2])),
            BOARD_VAL((bsize_t(na[NA_REAL(i, j, size, offset) * 2 + 1]) << 1) +
                      bsize_t(na[NA_REAL(i, j, size, offset) * 2])),
        };
    }

   public:
    GomokuBitBoard() {
        for (size_t i = 0; i < size + offset * 2; ++i) {
            for (size_t j = 0; j < offset; ++j) {
                set_real(i, j, INV);
                set_real(i, size + offset * 2 - 1 - j, INV);
                set_real(j, i, INV);
                set_real(size + offset * 2 - 1 - j, i, INV);
            }
        }
    }

   public:
    bitset_type<HENG_MAXLEN(size, offset) * 2> heng;
    bitset_type<SHU_MAXLEN(size, offset) * 2> shu;
    bitset_type<PIE_MAXLEN(size, offset) * 2> pie;
    bitset_type<NA_MAXLEN(size, offset) * 2> na;
};
}// namespace chis