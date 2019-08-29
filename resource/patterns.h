#pragma once
#include "utils/types.h"
namespace chis {
//原始棋型 1~4位为值位，5~30位为棋型位。wind23棋型表转换规则
//re.match(r'XX_*(X[O_]*X)_*XX', col[-1]).group(1) then X->11 O->10 _->00
extern const uint32_t patterns[2724];

//目标棋型 1~22位为棋型位 11为边界 00为空 10为黑子 01为白子
//值： 1~4位为黑棋类型, 5~8为白棋类型
class GomokuPatterns {
   public:
    GomokuPatterns() {
        build();
    }
    static set_type<uint32_t> &arrangement_padding(uint32_t size) {
        static thread_local set_type<uint32_t> genres[12] = {
            {0x0}, {0x3, 0x2, 0x1, 0x0}};
        if (genres[size].size()) {
            return genres[size];
        }
        set_type<uint32_t> ret;
        for (uint32_t a : arrangement_padding(size - 1)) {
            //存在11            
            ret.insert((a << 2) | 0x3U);
            ret.insert((a << 2) | 0x2U);
            ret.insert((a << 2) | 0x1U);
            ret.insert((a << 2) | 0x0U);
        }
        genres[size] = std::move(ret);
        return genres[size];
    }
    //将10 01互换
    static uint32_t reverse_pattern(uint32_t pat) {
        uint32_t reverse =
            (pat ^ (0x003FFFFFU));  // 11->00 10->01 01->10 00->11
        for (uint32_t i = 0; i <= 20; i += 2) {
            //如果对应的位为11或00
            if (((reverse >> i) & 0x3U) == 0x3U ||
                ((reverse >> i) & 0x3U) == 0x0U) {
                reverse ^= (0x3U << i);  //翻转
            }
        }
        return reverse;
    }
    //22bit中心对称翻转
    static uint32_t reverse_22bit(uint32_t v) {
        v = ((v >> 1)  & 0x55555555U) | ((v << 1)  & 0xAAAAAAAAU);
        v = ((v >> 2)  & 0x33333333U) | ((v << 2)  & 0xCCCCCCCCU);
        v = ((v >> 4)  & 0x0F0F0F0FU) | ((v << 4)  & 0xF0F0F0F0U);
        v = ((v >> 8)  & 0x00FF00FFU) | ((v << 8)  & 0xFF00FF00U);
        v = ((v >> 16) & 0x0000FFFFU) | ((v << 16) & 0xFFFF0000U);
        return (v>>10);
    }
    static vector_type<uint32_t> create_more(uint32_t pat) {
        //计算需要填充的位数
        uint32_t paddinglen = 0;
        for (size_t i = 2; i <= 22; i += 2) {
            //找最后一个0x3边界
            if (((uint32_t(3) << i) & pat) == (uint32_t(3) << i)) {
                paddinglen = 11 - i / 2;
            }
        }
        pat &= (uint32_t(1) << (22 - paddinglen * 2)) - 1;  //去掉左边界标志
        vector_type<uint32_t> ret;
        //左右填充
        for (size_t i = 0; i <= paddinglen; ++i) {
            uint32_t leftlen = i, rightlen = paddinglen - i;
            //边界不能覆盖棋型中点，否则会发生混淆
            if (leftlen > 5 || rightlen > 5) {
                continue;
            }
            set_type<uint32_t> leftPadding = arrangement_padding(leftlen);
            set_type<uint32_t> rightPadding = arrangement_padding(rightlen);
            for (uint32_t l : leftPadding) {
                for (uint32_t r : rightPadding) {
                    uint32_t ret_pat =
                        pat << uint32_t(rightlen * 2);  //腾出右边空间
                    // l为有效边界时，边界必须是INV或者WHT. l == 0x0
                    // 时，意味着左侧不填充
                    if ((l & 0x3U) == 0x3U || (l & 0x3U) == 0x1U) {
                        ret_pat |= uint32_t(l) << uint32_t(22 - leftlen * 2);
                    } else if (leftlen != 0) {
                        continue;
                    }
                    if (((r >> (rightlen * 2 - 2)) & 0x3U) == 0x3U ||
                        ((r >> (rightlen * 2 - 2)) & 0x3U) == 0x1U) {
                        ret_pat |= uint32_t(r);
                    } else if (rightlen != 0) {
                        continue;
                    }
                    
                    ret.push_back(ret_pat);
                    //翻转黑白，使得黑白棋型一致
                    ret.push_back(reverse_pattern(ret_pat)^0x00400000U);//同时给第23位置1
                }
            }
        }
        return ret;
    }
    void build() {
        for (size_t i = 0; i < 2724; ++i) {
            uint32_t pat = patterns[i];
            //取值 后4位
            uint8_t val = pat & 0xF;
            //去掉2位边界和4位值
            pat = pat >> 6;
            auto more_patterns = create_more(pat);//填充
            for (uint32_t p : more_patterns) {
                if((p >> 22) == 0) {
                    pattern_map[p] &= 0xF0U;//去掉低四位
                    pattern_map[p] |= val;//低四位赋值
                } else {
                    p &= 0x003FFFFFU;
                    pattern_map[p] &= 0x0FU;//去掉高四位
                    pattern_map[p] |= (val<<4);//高四位赋值
                }
            }
        }
    }
   public:
    uint8_t pattern_map[1U << 22] = {};
};
}  // namespace chis