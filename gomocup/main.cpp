#include "search/Solution.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
using namespace std;
    
struct GlobalInfo {// 配置
    int timeout_turn = 30000;           // 步时
    int timeout_match = 180000;          // 局时
    int max_memory = 367001600;             // 内存
    int time_left = 1714;              // 余时
    int game_type = 1;              // 类型
    int rule = 0;                   // 规则
    string folder;  // 目录

    int MAX_DEPTH = 5;
};
GlobalInfo g_info;
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
int Start(chis::Solution* &slu) {
    if (slu) delete slu;
    int size;
    cin >> size;
    try {
        slu = chis::MakeSolution(size);
    } catch(std::exception e) {
        cout << "ERROR Exception:" <<  e.what() << endl;
        return -1;
    }
    cout << "OK" << endl;
    return 0;
}
int Turn(chis::Solution* &slu) {
    string line;
    getline(cin, line);
    int x, y;
    if(sscanf(line.c_str(), "%*s%d,%d", &x, &y) != 2) {
        cout << "ERROR ShouldBe:`Turn %d,%d`" << endl;
    }
    if(slu->Get(x, y) != chis::BOARD_VAL::EMP) {
        cout << "ERROR InvalidMove:"<<x<<","<<y<< endl;
    }
    slu->Do(x, y);
    auto rets = slu->Search(g_info.MAX_DEPTH);
    auto[i,j] = rets.front().first;
    slu->Do(i,j);
    cout << i << "," << j << endl;
    return 0;
}
int Begin(chis::Solution* &slu) {
    auto rets = slu->Search(g_info.MAX_DEPTH);
    auto[i,j] = rets.front().first;
    slu->Do(i,j);
    cout << i << "," << j << endl;
    return 0;
}
int Board(chis::Solution* &slu) {
    string line;
    while(getline(cin, line)) {
        if(line.empty()) {
            continue;
        }
        cout << "DEBUG BoardInput:" << line << endl;
        if(line == "DONE") {
            break;
        }
        int x, y, t;
        if(sscanf(line.c_str(), "%d,%d,%d", &x, &y, &t) != 3) {
            cout << "ERROR ShouldBe:`Turn %d,%d` "  <<":" <<x<<","<<y<<","<<t << endl;
            return -1;
        }
        if(slu->Get(x, y) != chis::BOARD_VAL::EMP) {
            cout << "ERROR InvalidMove:"<<x<<","<<y<< endl;
            return -2;
        }
        slu->Do(x, y);
    }
    auto rets = slu->Search(g_info.MAX_DEPTH);
    auto[i,j] = rets.front().first;
    slu->Do(i,j);
    cout << i << "," << j << endl;
    return 0;
}
int Info() {
    string key, val;
    cin >> key >> val;
    if ( key == "timeout_turn" ) { g_info.timeout_turn  = atoi(val.c_str()); }
	else if ( key == "timeout_match") { g_info.timeout_match = atoi(val.c_str()); }
	else if ( key == "max_memory"   ) { g_info.max_memory    = atoi(val.c_str()); }
	else if ( key == "time_left"    ) { g_info.time_left     = atoi(val.c_str()); }
	else if ( key == "game_type"    ) { g_info.game_type     = atoi(val.c_str()); }
	else if ( key == "rule"         ) { g_info.rule          = atoi(val.c_str()); }
	else if ( key == "folder"       ) { g_info.folder = val; }
    return 0;
}
int Restart(chis::Solution* &slu) {
    slu->Reset();
    return 0;
}
int main() {
    string cmd;
    chis::Solution* slu = nullptr;
    while (cin >> cmd) { 
        if (!CMDSTR2INT.count(cmd)) {
            //cout << "ERROR Command Error" << endl;
            cout << "OK" << endl;
		}
        int code = 0;
        switch (CMDSTR2INT[cmd]) {
            case END:
                return 0;
            case START: 
                Start(slu);
                break;
            case TURN: 
                Turn(slu); 
                break;
            case BEGIN:
                Begin(slu); 
                break;
            case BOARD:
                Board(slu); 
                break;
            case INFO:
                Info(); 
                break;
            case ABOUT:
                cout << "name=\"chis++\"," << endl
                     << "version=\"2019.8\"," << endl
                     << "author=\"bread\", " << endl
        		     << "country=\"China\", " << endl
		             << "email=\"chisbread@gmail.com\"" << endl;
                break;
            case RESTART:
                Restart(slu); 
                break;
            default:
                break;
        }
	}
    return 0;
}