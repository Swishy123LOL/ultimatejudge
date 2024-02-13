#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
#include <filesystem>

using namespace std;
using json = nlohmann::json;
using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

namespace ns{
    struct info{
        string input, output;
    };
    void to_json(json& j, const info& info) {
        j = json{{"input", info.input}, {"output", info.output}};
    }
    void from_json(const json& j, info& info) {
        j.at("input").get_to(info.input);
        j.at("output").get_to(info.output);
    }
}

map<string, string> flag_parser(vector<string> &args){
    map<string, string> res;
    string crr = "", flag = "";

    bool flagb = 0;
    for (int i = 0; i <= args.size(); i++){
        if (i == args.size() || args[i][0] == '-'){
            res[flag] = crr; crr = "";
            if (i < args.size())
                flag = args[i].substr(1, args.size() - 1);
            flagb = 0;
        }
        else{
            if (flagb) crr += " "; flagb = 1;
            crr += args[i];
        }
    }

    return res;
}

int main(int argc, char *argv[]){
    string PATH = filesystem::current_path().string();
    string DES_PATH = filesystem::current_path().string();

    vector<string> args;
    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);
    map<string, string> flags = flag_parser(args);

    if (flags.count("i"))
        PATH = flags["i"];
    if (flags.count("o"))
        DES_PATH = flags["o"];

    json j;
    map<string, bool> hasin, hasout;

    for (const auto& dirEntry : recursive_directory_iterator(PATH.c_str())){
        if (filesystem::is_directory(dirEntry))
            continue;

        string path = dirEntry.path().string();
        string name = path.substr(path.find_last_of("/\\") + 1);
        string exten = name.substr(name.find_last_of('.'));
        name = name.substr(0, name.find_last_of('.'));

        if (exten == ".in") hasin[name] = 1;
        if (exten == ".out") hasout[name] = 1;
    }

    vector<ns::info> v;
    for (auto name : hasin){
        if (hasout[name.first])
            v.push_back({DES_PATH + "\\" + name.first + ".in", DES_PATH + "\\" + name.first + ".out"});
    }

    j["testcases"] = v;
    ofstream out("testinfo.json");

    out << setw(4) << j;
}