#include "search/Solution.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
using namespace std;
    
struct GlobalInfo {// 配置
    int timeout_turn;           // 步时
    int timeout_match;          // 局时
    int max_memory;             // 内存
    int time_left;              // 余时
    int game_type;              // 类型
    int rule;                   // 规则
    char folder[FILENAME_MAX];  // 目录
};
enum CMD_TYPE {
	INVALID_CMD, 
	END, START,
    TURN, BEGIN, 
	BOARD, INFO,
	ABOUT, RESTART,
	TAKEBACK, PLAY,
};
std::map<string, CMD_TYPE> CMDSTR2INT = {
    {"END", END},  {"START", START}, {"TURN", TURN},    {"BEGIN", BEGIN},    {"BOARD", BOARD},
    {"INFO", INFO}, {"ABOUT", ABOUT}, {"RESTART", RESTART}, {"TAKEBACK", TAKEBACK}, {"PLAY", PLAY},
};

int main() {
    string line;
    chis::Solution *slu = nullptr;
    while (getline(cin, line)) { 
        if (!CMDSTR2INT.count(line)) {
            cout << "Command Error" << endl;
		}
        switch (CMDSTR2INT[line]) {
            case END:
                return 0;
            case START: {
                if (slu) {
                    delete slu;
				}
                int size;
                cin >> size;

            }
            default:
                break;
        }
	}
    return 0;
}