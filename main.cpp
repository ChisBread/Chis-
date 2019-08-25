#include <iostream>
#include <string>
#include "board/operator.hpp"
#include "board/gomoku.hpp"
#include "resource/patterns.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
using namespace std;
const static chis::bsize_t SIZE = 15;
const static chis::bsize_t OFFSET = 5;

void test_operator() {
    int board[SIZE*2][SIZE*2] = {};
    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {
            board[i*2][j*2] = chis::HENG(i,j,SIZE);
            board[i*2][j*2+1] = chis::SHU(i,j,SIZE);
            board[i*2+1][j*2] = chis::PIE(i,j,SIZE);
            board[i*2+1][j*2+1] = chis::NA(i,j,SIZE);
        }
    }
    cout << string(SIZE*8, '-') << endl << "|";
    for(int i = 0; i < SIZE*2; ++i) {
        for(int j = 0; j < SIZE*2; ++j) {
            printf("%03d%s", board[i][j],j%2==1?"|":",");
        }
        cout << endl;
        if(i%2) {
            cout << string(SIZE*8, '-') << endl;
        } 
        cout << "|";
    }
    cout << chis::HENG_MAXLEN(SIZE) << "," << chis::SHU_MAXLEN(SIZE) << "|" << endl;
    cout << "|" << chis::PIE_MAXLEN(SIZE) << "," << chis::NA_MAXLEN(SIZE) <<"|" << endl;
}
void test_gomoku() {
    srand(time(NULL));
    chis::GomokuBoard<SIZE, OFFSET> board;
    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {
            board[i][j] = rand()%2==0?chis::BLK:chis::WHT;
        }
    }
    bool hasErrors = false;
    for(int i = 0; i < SIZE+OFFSET*2; ++i) {
        for(int j = 0; j < SIZE+OFFSET*2; ++j) {
            auto cells = board.board.get_real_all(i, j);
            if( !(
                cells[0].val.to_string() == cells[1].val.to_string() &&
                cells[0].val.to_string() == cells[2].val.to_string() &&
                cells[0].val.to_string() == cells[3].val.to_string() )) {
                cout << "! ";
                hasErrors = true;
                continue;
            }
            switch (chis::BOARD_VAL(cells[0].val.to_ulong()))
                {
                case chis::EMP :
                    cout << "E ";
                    break;
                case chis::WHT :
                    cout << "W ";
                    break;
                case chis::BLK :
                    cout << "B ";
                    break;
                case chis::INV :
                    cout << "I ";
                    break;
                default:
                    hasErrors = true;
                    cout << "? ";
                    break;
                }
        }
        cout << endl;
    }
    cout << (hasErrors?"something wrong":"everythins is OK") << endl;
}
void test_pattern() {
    chis::GomokuPatterns patterns;
}
int main() {
    //test_operator();
    //test_gomoku();
    test_pattern();
    return 0;
}