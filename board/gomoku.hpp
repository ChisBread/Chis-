#pragma once
#include "board/mappings.hpp"
#include "board/types.h"
namespace chis {
//五子棋位棋盘-基本操作
template <bsize_t size = 15, bsize_t offset = 5> class GomokuBitBoard {
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
    vector_type<bcell_t> get_real_all(bsize_t i, bsize_t j) const {
        vector_type<bcell_t> ret = {
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
        return ret;
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
//只考虑黑棋棋形，白棋转成黑棋再判断
using pinfo_t = uint8_t;
static const pinfo_t WON = 0x80;

template <uint32_t offset = 5> class PatternMap {
  public:
    struct PointPattern {
        uint32_t pattern[4];
    };

  public:
    static pinfo_t is_five(uint32_t pattern) {
        const uint32_t blk_five = 0x2AA; // 1010101010
        for (uint32_t i = 0; i <= 12; i += 2) {
            uint32_t bfive = blk_five << i;
            if ((bfive & pattern) == bfive) {
                return WON; // 1000 0000
            }
        }
        return 0;
    }
    // 7:标志位 代表是否成五
    // 6~4:
    pinfo_t search(uint32_t pattern, pinfo_t step = 0) {
        if (patterns[pattern] != 0) {
            return patterns[pattern];
        }
        for (uint32_t i = 0; i <= 20; ++i) {
            //最后两位是00代表可以落子
            if ((pattern >> i) % 4 != 0) {
                continue;
            }
        }
        return false;
    }

  public:
    // 0代表未初始化
    uint8_t patterns[1U << ((offset * 2U + 1U) * 2)] = {};
};

//五子棋棋盘-状态/Hash/...
template <bsize_t size = 15, bsize_t offset = 5> class GomokuBoard {
  public:
    struct do_info {
        bsize_t i;
        bsize_t j;
        bsize_t v;
    };
    class hash_func {
      public:
        uint64_t operator()(const GomokuBoard<size, offset> &b) const {
            return b.hashval;
        }
    };
    class equal_func {
      public:
        uint64_t operator()(const GomokuBoard<size, offset> &b,
                            const GomokuBoard<size, offset> &c) const {
            return b.hashval == c.hashval;
        }
    };
    struct point_pattern {
        uint8_t heng;
        uint8_t shu;
        uint8_t pie;
        uint8_t na;
    };

  public:
    //重载[]
    array_tmp<GomokuBoard<size, offset>> operator[](size_t i) {
        return array_tmp<GomokuBoard<size, offset>>{*this, i};
    }
    //落子
    GomokuBoard<size, offset> &Do(bsize_t i, bsize_t j, const bcell_t &v) {
        board.set(i, j, v);
        bsize_t val = v.val.to_ulong();
        hashval ^= zobrist[i][j][val];
        doChain.push_back(do_info{i, j, val});
        return *this;
    }
    //撤销落子
    GomokuBoard<size, offset> &Uodo() {
        board.set(doChain.back().i, doChain.back().j, EMP); //置空
        hashval ^=
            zobrist[doChain.back().i][doChain.back().j][doChain.back().v];
        doChain.pop_back();
        return *this;
    }
    bcell_t Get(bsize_t i, bsize_t j) const { return board.get(i, j); }
    //评估函数
    int32_t Evaluation() { return 0; }
    //得到点附近的棋型
    point_pattern GetPattern(bsize_t i, bsize_t j) { return {}; }

  public:
    GomokuBoard() {
        hashval = rand_uint64();
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                zobrist[i][j][0] = rand_uint64();
                zobrist[i][j][1] = rand_uint64();
                zobrist[i][j][2] = rand_uint64();
                zobrist[i][j][3] = rand_uint64();
            }
        }
    }

  public:
    // status
    vector_type<do_info> doChain;
    GomokuBitBoard<size, offset> board; //四种值的位棋盘
    // hashing
    uint64_t hashval;
    uint64_t zobrist[size][size][4];
};
template <typename BTy, typename VTy>
using GomokuBoardMap = std::unordered_map<BTy, VTy, typename BTy::hash_func,
                                          typename BTy::equal_func>;
} // namespace chis