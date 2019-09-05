#include <map>
#include <set>
#include <string>
#include "board/gomoku.hpp"
#include "gtest/gtest.h"
#include "resource/patterns.h"
using namespace std;
const static size_t SIZE = 15;
const static size_t OFFSET = 5;
TEST(TESTBoard, BoardBase) {
    //初始化
    srand(time(NULL));
    chis::BOARD_VAL realboard[SIZE][SIZE] = {};
    chis::GomokuBoard<SIZE, OFFSET> board;
    vector<uint64_t> hash_chain;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            chis::BOARD_VAL setVal = chis::BOARD_VAL(rand() % 3);
            board[i][j] = setVal;
            if (!hash_chain.empty()) {
                EXPECT_NE(hash_chain.back(), board.Hash());
            }
            hash_chain.push_back(board.Hash());
            realboard[i][j] = setVal;
        }
    }
    // Board 棋盘基本功能测试
    for (int i = 0; i < SIZE + OFFSET * 2; ++i) {
        for (int j = 0; j < SIZE + OFFSET * 2; ++j) {
            auto cell = board.board.GetReal(i, j);
            //有效性对比
            if (i - OFFSET >= 0 && i - OFFSET < SIZE && j - OFFSET >= 0 && j - OFFSET < SIZE) {
                EXPECT_EQ(cell, realboard[i - OFFSET][j - OFFSET]);
            } else {
                EXPECT_EQ(cell, chis::BOARD_VAL::INV);
            }
        }
    }
    // Hash 功能测试
    while (!hash_chain.empty()) {
        EXPECT_EQ(hash_chain.back(), board.Hash());
        hash_chain.pop_back();
        board.Undo();
    }
}
TEST(TESTBoard, BoardPattern) {
    //初始化
    srand(time(NULL));
    chis::BOARD_VAL realboard[SIZE][SIZE] = {};
    chis::GomokuBoard<SIZE, OFFSET> board;
    auto check_pattern = [&](uint32_t pat) {
        auto [h, s, p, n] = board.GetPattern(7, 7);
        EXPECT_EQ(h, pat);
    };
    auto check_pattern_type = [&](uint8_t t) {
        auto [h, s, p, n] = board.GetPatternType(7, 7);
        EXPECT_EQ(h, t);
    };
    // GetPattern 测试
    //活一
    board[7][7] = chis::BOARD_VAL::BLK;
    check_pattern(0b0000000000100000000000);
    check_pattern_type(2);
    {  //眠一
        board[7][8] = chis::BOARD_VAL::WHT;
        check_pattern(0b0000000001100000000000);
        check_pattern_type(1);
        board.Undo();
    }
    //活二
    board[7][8] = chis::BOARD_VAL::BLK;
    check_pattern(0b0000000010100000000000);
    check_pattern_type(6);
    {  //眠二
        board[7][6] = chis::BOARD_VAL::WHT;
        check_pattern(0b0000000010100100000000);
        check_pattern_type(3);
        board.Undo();
    }
    //活三
    board[7][6] = chis::BOARD_VAL::BLK;
    check_pattern(0b0000000010101000000000);
    check_pattern_type(9);
    {  //眠三
        board[7][5] = chis::BOARD_VAL::WHT;
        check_pattern(0b0000000010101001000000);
        check_pattern_type(7);
        board.Undo();
    }
    //活四
    board[7][5] = chis::BOARD_VAL::BLK;
    check_pattern(0b0000000010101010000000);
    check_pattern_type(12);
    {  //眠四
        board[7][9] = chis::BOARD_VAL::WHT;
        check_pattern(0b0000000110101010000000);
        check_pattern_type(10);
        board.Undo();
    }
    //成5
    board[7][9] = chis::BOARD_VAL::BLK;
    check_pattern(0b0000001010101010000000);
    check_pattern_type(13);
}
TEST(TESTBoard, Patterns) {
    uint8_t pattern_map[1U << 22] = {};
    // GomokuPatterns基础操作测试
    // reverse_pattern
    // reverse_bit
    // arrangement_padding
    // create_more

    //初始化测试
    for (size_t i = 0; i < 2724; ++i) {
        uint32_t pat = chis::patterns[i];
        //取值 后4位
        uint8_t val = pat & 0xF;
        //去掉2位边界和4位值
        pat = pat >> 6;
        //填充
        auto more_patterns = chis::GomokuPatterns::create_more(pat);
        for (uint32_t p : more_patterns) {
            if ((p >> 22) == 0) {
                //值合法性
                ASSERT_LT(p, (1U << 22));
                //覆盖值一致性
                if ((pattern_map[p] & 0x0F) != 0) {
                    EXPECT_EQ((pattern_map[p] & 0x0F), val);
                }
                pattern_map[p] &= 0xF0U;  //去掉低四位
                pattern_map[p] |= val;    //低四位赋值
            } else {
                p &= 0x003FFFFFU;
                //值合法性
                ASSERT_LT(p, (1U << 22));
                //覆盖值一致性
                if ((pattern_map[p] >> 4) != 0) {
                    EXPECT_EQ((pattern_map[p] >> 4), val);
                }
                pattern_map[p] &= 0x0FU;       //去掉高四位
                pattern_map[p] |= (val << 4);  //高四位赋值
            }
        }
    }
    auto rev_pattern_test = [&](uint32_t i) {
        ASSERT_LT(i, (1U << 22));
        uint8_t valblk = pattern_map[i] & 0x0F;
        uint8_t valwht = (pattern_map[i] >> 4);
        {  //置反一致性校验 黑子白子互换结果是否一致 a.黑白颠倒 b.棋型一致
            uint32_t ni = chis::GomokuPatterns::reverse_pattern(i);
            uint8_t nvalblk = pattern_map[ni] & 0x0F;
            uint8_t nvalwht = (pattern_map[ni] >> 4);
            EXPECT_EQ(valblk, nvalwht);
            EXPECT_EQ(valwht, nvalblk);
        }
        {  //对称一致性校验 中心对称翻转结果是否一致 a.黑白颠倒 b.棋型一致
            uint32_t ri = chis::GomokuPatterns::reverse_22bit(i);
            uint8_t rvalblk = pattern_map[ri] & 0x0F;
            uint8_t rvalwht = (pattern_map[ri] >> 4);
            EXPECT_EQ(valblk, rvalwht);
            EXPECT_EQ(valwht, rvalblk);
        }
        {  //置反+对称 a.黑白不颠倒 b.棋型一致
            uint32_t nri = chis::GomokuPatterns::reverse_22bit(chis::GomokuPatterns::reverse_pattern(i));
            uint8_t nrvalblk = pattern_map[nri] & 0x0F;
            uint8_t nrvalwht = (pattern_map[nri] >> 4);
            EXPECT_EQ(valblk, nrvalblk);
            EXPECT_EQ(valwht, nrvalwht);
        }
    };
    //棋型属性测试 1.各种对称性
    for (uint32_t i = 0; i < (1U << 22); ++i) {
        rev_pattern_test(i);
    }
    //棋型可用性测试:构造30~60个常见棋型以及其变种,校验棋型表可用性
    map<uint32_t, uint8_t> real_patterns = {
        //|死棋 |0 |____XOX____
        {0b1111111111101111111111U, 0},
        //|眠一 |1 |____XO____X
        {0b1111111111100000000011U, 1},  //测试基础棋型是否正确
        {0b1111111101100000000011U, 1},  //测试生成是否全面
        {0b1111111001100000000001U, 1},  //测试生成是否全面
        //|活一 |2 |___X_O____X
        {0b1111111100100000000011U, 2},
        //|眠二 |3 |____XO___OX
        {0b1111111111100000001011U, 3},
        //|活二A|4 |___X_O__O_X
        {0b1111111100100000100011U, 4},
        //|活二B|5 |__X__O_O__X
        {0b1111110000100010000011U, 5},
        //|活二C|6 |_X___OO___X
        {0b1111000000101000000011U, 6},
        //|眠三 |7 |____XO__OOX
        {0b1111111111100000101011U, 7},
        //|活三A|8 |___X_O_OO_X
        {0b1111111100100010100011U, 8},
        //|活三B|9 |__X__OOO__X
        {0b1111110000101010000011U, 9},
        //|眠四 |10|____XO_OOOX
        {0b1111111111100010101011U, 10},
        //|活四A|11|__XO_OOO_OX
        {0b1111111000101010001011U, 11},
        //|活四B|12|___X_OOOO_X
        {0b1111111100101010100011U, 12},
        //|成五 |13|____XOOOOOX
        {0b1111111111101010101011U, 13},
    };
    for (auto &item : real_patterns) {
        EXPECT_EQ(item.second, pattern_map[item.first]);
        rev_pattern_test(item.first);
    }
}