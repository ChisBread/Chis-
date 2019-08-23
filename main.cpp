#include <iostream>
#include "board/operator.hpp"
#include "board/gomoku.hpp"
using namespace std;
void test_operator() {
    for(int i = 0; i < 15; ++i) {
        for(int j = 0; j < 15; ++j) {
            cout << chis::HENG(i,j,15) << " ";
        }
        cout << endl;
    }
    cout << chis::HENG_MAXLEN(15) << endl;
    for(int i = 0; i < 15; ++i) {
        for(int j = 0; j < 15; ++j) {
            cout << chis::SHU(i,j,15) << " ";
        }
        cout << endl;
    }
    cout << chis::SHU_MAXLEN(15) << endl;
    for(int i = 0; i < 15; ++i) {
        for(int j = 0; j < 15; ++j) {
            cout << chis::PIE(i,j,15) << " ";
        }
        cout << endl;
    }
    cout << chis::PIE_MAXLEN(15) << endl;
    for(int i = 0; i < 15; ++i) {
        for(int j = 0; j < 15; ++j) {
            cout << chis::NA(i,j,15) << " ";
        }
        cout << endl;
    }
    cout << chis::NA_MAXLEN(15) << endl;
}
void test_gomoku() {
    chis::GomokuBoard<15, 5> board;
    for(int i = 0; i < 15; ++i) {
        for(int j = 0; j < 15; ++j) {
            board[i][j] = i>j?chis::BLK:chis::WHT;
        }
    }
    for(int i = 0; i < 15; ++i) {
        for(int j = 0; j < 15; ++j) {
            chis::bcell_t cell = board[i][j];
            cout << (cell.val.to_string() == chis::BLK_CELL.val.to_string()?"B":"W") << ",";
        }
        cout << endl;
    }
    
    for(int i = 0; i < 25; ++i) {
        for(int j = 0; j < 25; ++j) {
            chis::bcell_t cell = board.board.get_real(i, j);
            cout << (cell.val.to_string() == chis::INV_CELL.val.to_string()?"X":"O") << ",";
        }
        cout << endl;
    }
}
int main() {
    test_operator();
    test_gomoku();
    return 0;
}