#pragma once
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <future>
#include <iostream>
#include <map>
#include <string>
#include "search/solution.hpp"
namespace chis {
struct GomocupConfig {
    int timeout_turn = 30000;    // 步时
    int timeout_match = 180000;  // 局时
    int max_memory = 367001600;  // 内存(bytes)
    int time_left = 180000;      // 余时
    int game_type = 1;           // 类型
    int rule = 0;                // 规则
    std::string folder;          // 目录

    int MAX_DEPTH = 5;
};

class stream_wrapper {
   public:
    stream_wrapper(std::istream &in, std::ostream &out)
        : inputer(in), outer(out) {}
    stream_wrapper &operator>>(std::string &str) {
        inputer >> str;
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        return *this;
    }
    template <typename T>
    stream_wrapper &operator>>(T &t) {
        inputer >> t;
        return *this;
    }
    template <typename T>
    stream_wrapper &operator<<(const T &t) {
        outer << t;
        return *this;
    }
    stream_wrapper &operator<<(std::ostream &(*pf)(std::ostream &)) {
        outer << pf;
        return *this;
    }
    bool InOK() { return inputer.operator bool(); }
    bool OutOK() { return outer.operator bool(); }
    stream_wrapper &Error() {
        outer << "ERROR ";
        return *this;
    }
    stream_wrapper &Debug() {
        outer << "DEBUG ";
        return *this;
    }
    std::istream &inputer;
    std::ostream &outer;
};

class GomocupProto {
   public:
   public:
    GomocupProto(std::istream &in = std::cin, std::ostream &out = std::cout)
        : io(in, out) {}
    int Run() {
        std::string cmd;
        while ((io >> cmd).InOK() && !is_ending) {
            if (!CMD2Func.count(cmd)) {
                io.Error() << "command \"" << cmd << "\" not found"
                           << std::endl;
            }
            if (int code = (this->*CMD2Func[cmd])(); code) {
                io.Debug() << "command \"" << cmd << "\" return code " << code
                           << std::endl;
            }
        }
        return 0;
    }
   public:
    std::tuple<int, int> Search() {
        //搜索 实现简单的超时控
        auto rets_future = std::async(std::launch::async, [&]() {
            return slu->Search(config.MAX_DEPTH);
        });
        std::future_status status;
        do {
            status = rets_future.wait_for(
                std::chrono::milliseconds(config.timeout_turn));
            if (status == std::future_status::deferred) {
                io.Debug() << "Search Deferred" << std::endl;
                slu->StopSearch();
            } else if (status == std::future_status::timeout) {
                io.Debug() << "Search Timeout" << std::endl;
                slu->StopSearch();
            }
        } while (status != std::future_status::ready);
        if (slu->IsStop()) {
            slu->StartSearch();
        }
        auto rets = rets_future.get();
        return rets.front().first;
    }
   public:  //命令
    int Start() {
        if (slu) delete slu;
        int size;
        io >> size;
        try {
            slu = chis::MakeSolution(size, config.max_memory * (9.0 / 10));
        } catch (std::exception e) {
            io.Error() << "Exception:" << e.what() << std::endl;
            return -1;
        }
        io << "OK" << std::endl;
        return 0;
    }
    int Turn() {
        std::string line;
        int x, y;
        while (getline(io.inputer, line)) {
            if (line.empty()) {
                continue;
            }
            if (sscanf(line.c_str(), "%d,%d", &x, &y) != 2) {
                io.Error() << "ShouldBe:`Turn %d,%d`" << std::endl;
                return -1;
            }
            break;
        }
        if (slu->Get(x, y) != chis::BOARD_VAL::EMP) {
            io.Error() << "InvalidMove:" << x << "," << y << std::endl;
        }
        slu->Do(x, y);
        auto [i, j] = Search();
        slu->Do(i, j);
        io << i << "," << j << std::endl;
        return 0;
    }
    int Begin() {
        auto [i, j] = Search();
        slu->Do(i, j);
        io << i << "," << j << std::endl;
        return 0;
    }
    int Board() {
        std::string line;
        while (getline(io.inputer, line)) {
            if (line.empty()) {
                continue;
            }
            // io.Debug() << " BoardInput:" << line << std::endl;
            if (line == "DONE") {
                break;
            }
            int x, y, t;
            if (sscanf(line.c_str(), "%d,%d,%d", &x, &y, &t) != 3) {
                io.Error() << "ShouldBe:`%d,%d,%d` "
                           << ":" << x << "," << y << "," << t << std::endl;
                return -1;
            }
            if (slu->Get(x, y) != chis::BOARD_VAL::EMP) {
                io.Error() << "InvalidMove:" << x << "," << y << std::endl;
                return -2;
            }
            slu->Do(x, y);
        }
        auto [i, j] = Search();
        slu->Do(i, j);
        io << i << "," << j << std::endl;
        return 0;
    }
    int Info() {
        std::string key, val;
        io >> key >> val;
        if (key == "timeout_turn") {
            config.timeout_turn = atoi(val.c_str());
        } else if (key == "timeout_match") {
            config.timeout_match = atoi(val.c_str());
        } else if (key == "max_memory") {
            config.max_memory = atoi(val.c_str());
            Restart();//重设内存
        } else if (key == "time_left") {
            config.time_left = atoi(val.c_str());
        } else if (key == "game_type") {
            config.game_type = atoi(val.c_str());
        } else if (key == "rule") {
            config.rule = atoi(val.c_str());
        } else if (key == "folder") {
            config.folder = val;
        }
        return 0;
    }
    int Restart() {
        slu->Reset(config.max_memory * (9.0 / 10));
        return 0;
    }
    int Takeback() {
        slu->Undo();
        return 0;
    }
    int Play() { return 0; }
    int About() {
        io << "name=\"chis++\"," << std::endl;
        io << "version=\"2019.8\"," << std::endl;
        io << "author=\"bread\", " << std::endl;
        io << "country=\"China\", " << std::endl;
        io << "email=\"chisbread@gmail.com\"" << std::endl;
        return 0;
    }
    int End() {
        is_ending = true;
        return 0;
    }
    int OK() {
        io.Debug() << "I'm OK too" << std::endl;
        return 0;
    }
    int ShowInfo() {
        io << "timeout_turn=" << config.timeout_turn << std::endl;
        io << "timeout_match=" << config.timeout_match << std::endl;
        io << "max_memory=" << config.max_memory << std::endl;
        io << "time_left=" << config.time_left << std::endl;
        io << "game_type=" << config.game_type << std::endl;
        io << "rule=" << config.rule << std::endl;
        io << "folder=" << config.folder << std::endl;

        return 0;
    }

   public:
    bool is_ending = false;
    stream_wrapper io;
    GomocupConfig config;
    chis::Solution *slu = nullptr;
    std::map<std::string, int (GomocupProto::*)()> CMD2Func = {
        {"END", &GomocupProto::End},
        {"START", &GomocupProto::Start},
        {"TURN", &GomocupProto::Turn},
        {"BEGIN", &GomocupProto::Begin},
        {"BOARD", &GomocupProto::Board},
        {"INFO", &GomocupProto::Info},
        {"ABOUT", &GomocupProto::About},
        {"RESTART", &GomocupProto::Restart},
        {"TAKEBACK", &GomocupProto::Takeback},
        {"PLAY", &GomocupProto::Play},
        {"OK", &GomocupProto::OK},
        {"SHOWINFO", &GomocupProto::ShowInfo}};
};
}  // namespace chis
