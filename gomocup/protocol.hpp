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
    int max_memory = 100001600;  // 内存(bytes)
    int time_left = 180000;      // 余时
    int game_type = 1;           // 类型
    int rule = 0;                // 规则
    std::string folder;          // 目录

    int MAX_DEPTH = 5;
};
inline std::string upperstr(const std::string &str) {
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return ret;
}
inline std::string lowerstr(const std::string &str) {
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return ret;
}
class stream_wrapper {
   public:
    stream_wrapper(std::istream &in, std::ostream &out)
        : inputer(in), outer(out) {}
    stream_wrapper &operator>>(std::string &str) {
        inputer >> str;
        str = upperstr(str);
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
        ShowStat();
        return rets.front().first;
    }
    void Do(int x, int y) {
        slu->Do(x, y);
        io.Debug() << "///////////AFTER MOVE"
                   << "(" << x << "," << y << ")///////////" << std::endl;
        ShowPointPattrtn(x, y);
        ShowGlobalPattrtn();
    }

   public:  //命令
    int Start() {
        if (slu) delete slu;
        int size;
        io >> size;
        try {
            slu = chis::MakeSolution(size, config.max_memory * 0.8);
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
        Do(x, y);
        auto [i, j] = Search();
        Do(i, j);
        io << i << "," << j << std::endl;
        return 0;
    }
    int Begin() {
        auto [i, j] = Search();
        Do(i, j);
        io << i << "," << j << std::endl;
        return 0;
    }
    int Board() {
        Restart();
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
            Do(x, y);
        }
        auto [i, j] = Search();
        Do(i, j);
        io << i << "," << j << std::endl;
        return 0;
    }
    int Info() {
        std::string key, val;
        std::cin >> key >> val;
        key = lowerstr(key);
        if (key == "timeout_turn") {
            config.timeout_turn = atoi(val.c_str());
            io.Debug() << "accepted timeout_turn:" << config.timeout_turn
                       << endl;
        } else if (key == "timeout_match") {
            config.timeout_match = atoi(val.c_str());
            io.Debug() << "accepted timeout_match:" << config.timeout_match
                       << endl;
        } else if (key == "max_memory") {
            config.max_memory = atoi(val.c_str());
            io.Debug() << "accepted max_memory:" << config.max_memory << endl;
            auto dos = slu->GetDoChain();
            Restart();  //重设内存
            slu->Do(dos);
            io.Debug() << "mem reset" << endl;
        } else if (key == "time_left") {
            config.time_left = atoi(val.c_str());
            io.Debug() << "accepted time_left:" << config.time_left << endl;
        } else if (key == "game_type") {
            config.game_type = atoi(val.c_str());
            io.Debug() << "accepted game_type:" << config.game_type << endl;
        } else if (key == "rule") {
            config.rule = atoi(val.c_str());
            io.Debug() << "accepted rule:" << config.rule << endl;
        } else if (key == "folder") {
            config.folder = val;
            io.Debug() << "accepted folder:" << config.folder << endl;
        }
        return 0;
    }
    int Restart() {
        slu->Reset(config.max_memory * 0.8);
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
    int ShowStat() {
        io.Debug() << "叶子节点" << slu->stat.leaf_cnt << std::endl;
        io.Debug() << "胜利节点" << slu->stat.ending_cnt << std::endl;
        io.Debug() << "置换表写入" << slu->stat.tt_record_cnt << std::endl;
        io.Debug() << "置换表命中" << slu->stat.tt_hit_cnt << std::endl;
        io.Debug() << "节点置换(end)" << slu->stat.tt_ending_pass_cnt
                   << std::endl;
        io.Debug() << "节点置换(pv)" << slu->stat.tt_pv_pass_cnt << std::endl;
        io.Debug() << "节点置换(alpha)" << slu->stat.tt_alpha_pass_cnt
                   << std::endl;
        io.Debug() << "节点置换(beta)" << slu->stat.tt_beta_pass_cnt
                   << std::endl;
        io.Debug() << "最佳着法尝试" << slu->stat.bestmove_try_cnt << std::endl;
        io.Debug() << "最佳着法剪枝" << slu->stat.bestmove_pass_cnt
                   << std::endl;
        io.Debug() << "主要变例搜索尝试" << slu->stat.pvs_try_cnt << std::endl;
        io.Debug() << "主要变例搜索剪枝" << slu->stat.pvs_pass_cnt << std::endl;
        io.Debug() << "总节点数" << slu->stat.node_cnt << std::endl;
        slu->stat = statInfo{};
        return 0;
    }
    int ShowGlobalPattrtn() {
        static const std::string patternName[] = {
            "死棋", "眠一",  "活一",  "眠二", "活二A", "活二B", "活二C",
            "眠三", "活三A", "活三B", "眠四", "活四A", "活四B", "成五",
        };
        for (int i = 0; i < 14; ++i) {
            if (!slu->PatternInfo().pattern_cnt_blk[i] &&
                !slu->PatternInfo().pattern_cnt_wht[i]) {
                continue;
            }
            io.Debug() << "GLOBAL PATTERN " << patternName[i];
            io << " 黑:" << slu->PatternInfo().pattern_cnt_blk[i];
            io << "白:" << slu->PatternInfo().pattern_cnt_wht[i] << std::endl;
        }
        return 0;
    }
    int ShowPointPattrtn(int x, int y) {
        static const std::string patternName[] = {
            "死棋", "眠一",  "活一",  "眠二", "活二A", "活二B", "活二C",
            "眠三", "活三A", "活三B", "眠四", "活四A", "活四B", "成五",
        };
        int cnts_blk[16] = {}, cnts_wht[16] = {};
        {
            uint8_t pats[4];
            std::tie(pats[0], pats[1], pats[2], pats[3]) =
                slu->GetPatternType(x, y);
            for (auto pat : pats) {
                ++cnts_blk[pat & 0xf];
                ++cnts_wht[pat >> 4];
            }
        }
        for (int i = 0; i < 14; ++i) {
            if (!cnts_blk[i] && !cnts_wht[i]) {
                continue;
            }
            io.Debug() << "POINT PATTERN "
                       << "(" << x << "," << y << ")";
            io << patternName[i];
            io << " 黑:" << cnts_blk[i];
            io << "白:" << cnts_wht[i] << std::endl;
        }
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
