#include "testlib.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[]) {
    setName("compare sequences of tokens");
    registerTestlibCmd(argc, argv);

    int n = 0;
    string j, p;

    json js;
    ofstream out("checker_result.json");
    while (!ans.seekEof() && !ouf.seekEof()) {
        n++;

        ans.readWordTo(j);
        ouf.readWordTo(p);
        
        if (j != p){
            js["verdict"] = "wa";
            js["comment"] = to_string(n) + englishEnding(n) + " words differ - expected: " + compress(j) + ", found: " + compress(p);

            out << js; out.close();
            quitf(_wa, "%d%s words differ - expected: '%s', found: '%s'", n, englishEnding(n).c_str(),
                  compress(j).c_str(), compress(p).c_str());
        }
    }

    if (ans.seekEof() && ouf.seekEof()) {
        if (n == 1){
            js["verdict"] = "ok";
            js["comment"] = to_string(n) + " tokens";

            out << js; out.close();
            quitf(_ok, "\"%s\"", compress(j).c_str());
        }
        else{
            js["verdict"] = "ok";
            js["comment"] = "\"" + compress(j) + "\"";

            out << js; out.close();
            quitf(_ok, "%d tokens", n);
        }
    } else {
        if (ans.seekEof()){
            js["verdict"] = "wa";
            js["comment"] = "Participant output contains extra tokens";

            out << js; out.close();
            quitf(_wa, "Participant output contains extra tokens");
        }
        else{
            js["verdict"] = "wa";
            js["comment"] = "Unexpected EOF in the participants output";

            out << js; out.close();
            quitf(_wa, "Unexpected EOF in the participants output");
        }
    }
}