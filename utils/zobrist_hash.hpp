#pragma once
#include "utils/types.h"
namespace chis {
template <size_t size, size_t val_cnt>
class ZobristHash {
   public:
    ZobristHash() {
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                for (size_t k = 0; k < val_cnt; ++k) {
                    zobrist[i][j][k] = rand_uint64();
                }
            }
        }
    }
    void Set(size_t i, size_t j, size_t v) { hashval ^= zobrist[i][j][v]; }
    uint64_t Hash() const { return hashval; }

   private:
    // hashing
    uint64_t hashval = 0;
    uint64_t zobrist[size][size][val_cnt] = {};
};

}  // namespace chis