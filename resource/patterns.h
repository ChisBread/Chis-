#pragma once
#include "utils/types.h"
namespace chis
{
extern uint32_t patterns[2724];
class GomokuPatterns {
public:
    GomokuPatterns(){
        pattern_map = vector_type<uint8_t>(1<<23, 0);
        build();
    }
    set_type<uint32_t>& arrangement_padding(uint32_t size) {
        static thread_local set_type<uint32_t> genres[12] = {{0x0}, {0x3,0x2,0x1}};
        if(genres[size].size()) {
            return genres[size];
        }
        set_type<uint32_t> ret;
        for(uint32_t a:arrangement_padding(size-1)) {
            //存在11
            if((a&uint32_t(0x3)) == uint32_t(0x3)) {
                ret.insert((a<<2)|uint32_t(0x3));
            } else {
                ret.insert((a<<2)|uint32_t(0x2));
                ret.insert((a<<2)|uint32_t(0x1));
            }
        }
        genres[size] = std::move(ret);
        return genres[size];
    }
    vector_type<uint32_t> create_more(uint32_t pat) {
        //计算需要填充的位数
        uint32_t paddinglen = 0;
        for(size_t i = 2; i <= 20; i += 2) {
            //找最后一个0x3边界
            if( ((uint32_t(3)<<i)&pat) == (uint32_t(3)<<i) ) {
                paddinglen = 11-i/2;
            }
        }
        pat &= (uint32_t(1)<<(23-paddinglen*2))-1;//去掉左边界标志
        vector_type<uint32_t> ret;
        //左右填充
        for(size_t i = 0; i <= paddinglen; ++i) {
            uint32_t leftlen = i, rightlen = paddinglen-i;
            set_type<uint32_t> leftPadding = arrangement_padding(leftlen);
            set_type<uint32_t> rightPadding = arrangement_padding(rightlen);
            for(uint32_t l:leftPadding) {
                for(uint32_t r:rightPadding) {
                    uint32_t ret_pat = pat << uint32_t(rightlen*2);//腾出右边空间
                    //l为有效边界时，边界必须是INV或者WHT. l == 0x0 时，意味着左侧不填充
                    if((l&uint32_t(0x3)) == uint32_t(0x3) || 
                       (l&uint32_t(0x1)) == uint32_t(0x1) || l == uint32_t(0x0)) {
                        ret_pat |= uint32_t(l) << uint32_t(22-leftlen*2);
                    }
                    if(((r >> (rightlen*2-2))&uint32_t(0x3)) == uint32_t(0x3) || 
                       ((r >> (rightlen*2-2))&uint32_t(0x1)) == uint32_t(0x1) || r == uint32_t(0x0)) {
                        ret_pat |= uint32_t(r);
                    }
                    ret.push_back(ret_pat);
                }
            }
        }
        return ret;
    }
    void build() {
        for(size_t i = 0; i < 2724; ++i) {
            uint32_t pat = patterns[i];
            //取值 后4位
            uint8_t val = pat & 0xF;
            //去掉2位边界和4位值
            pat = pat >> 6;
            //填充
            auto more_patterns = create_more(pat);
            for(uint32_t p:more_patterns) {
                pattern_map[p] = val;
            }
        }
    }
public:
    vector_type<uint8_t> pattern_map;
};
} // namespace chis
