#include "gtest/gtest.h"
#include "board/gomoku.hpp"
#include <string>
using namespace std;
const static chis::bsize_t SIZE = 15;
const static chis::bsize_t OFFSET = 5;
TEST(TESTBoard, Mappings) 
{
    //初始化Mappings
    int board[SIZE*2][SIZE*2] = {};
    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {
            board[i*2][j*2] = chis::HENG(i,j,SIZE);
            board[i*2][j*2+1] = chis::SHU(i,j,SIZE);
            board[i*2+1][j*2] = chis::PIE(i,j,SIZE);
            board[i*2+1][j*2+1] = chis::NA(i,j,SIZE);
        }
    }
    //Mapping 坐标映射功能测试
    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {            
            if(j > 0) {
                //HENG坐标连续性测试 //HENG → j-1
                EXPECT_EQ(board[i*2][j*2], board[i*2][(j-1)*2]+1);
            }
            if(i > 0) {
                //SHU坐标连续性测试 //SHU ↓  i-1
                EXPECT_EQ(board[i*2][j*2+1], board[(i-1)*2][j*2+1]+1);
            }
            if(j > 0 && i < SIZE-1) {
                //PIE坐标连续性测试 //PIE ↗  i+1 j-1 
                EXPECT_EQ(board[i*2+1][j*2], board[(i+1)*2+1][(j-1)*2]+1);
            }
            if(j > 0 && i > 0) { 
                //NA坐标连续性测试 //NA ↘   i-1 j-1
                EXPECT_EQ(board[i*2+1][j*2+1], board[(i-1)*2+1][(j-1)*2+1]+1);
            }
        }
    }
}
TEST(TESTBoard, Board)
{
    //初始化
    srand(time(NULL));
    chis::BOARD_VAL realboard[SIZE][SIZE] = {};
    chis::GomokuBoard<SIZE, OFFSET> board;
    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {
            chis::BOARD_VAL setVal = rand()%2==0?chis::BLK:chis::WHT;
            board[i][j] = setVal;
            realboard[i][j] = setVal;
        }
    }
    //Board 棋盘基本功能测试
    for(int i = 0; i < SIZE+OFFSET*2; ++i) {
        for(int j = 0; j < SIZE+OFFSET*2; ++j) {
            auto cells = board.board.get_real_all(i, j);
            //有效性对比
            if(i-OFFSET >= 0 && i-OFFSET < SIZE && j-OFFSET >= 0 && j-OFFSET < SIZE) {
                EXPECT_EQ(chis::BOARD_VAL(cells[0].val.to_ulong()), realboard[i-OFFSET][j-OFFSET]);
            } else {
                EXPECT_EQ(chis::BOARD_VAL(cells[0].val.to_ulong()), chis::BOARD_VAL::INV);
            }
            //一致性对比
            EXPECT_EQ(cells[0].val.to_ulong(), cells[1].val.to_ulong());
            EXPECT_EQ(cells[0].val.to_ulong(), cells[2].val.to_ulong());
            EXPECT_EQ(cells[0].val.to_ulong(), cells[3].val.to_ulong());   
        }
    }
}