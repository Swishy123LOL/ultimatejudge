#include <bits/stdc++.h>
#include <windows.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

int exec(const char* name){
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE; 

    TCHAR szCmdline[1000];
    strcpy(szCmdline, name);

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcess(NULL, 
        szCmdline,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example. 

    DWORD exitcode;
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);
    GetExitCodeProcess(piProcInfo.hProcess, &exitcode);
    // Close process and thread handles. 
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return exitcode;
}

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
    vector<string> args;
    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);
    map<string, string> flags = flag_parser(args);

    long long time_limit, memory_limit;
    if (!flags.count("i")){
        cout << "Source code path missing";
        return 0;
    }
    if (!flags.count("o")){
        cout << "Program destination path missing";
        return 0;
    }
    if (!flags.count("t")){
        cout << "Time limit missing (specify in miliseconds)";
        return 0;
    }
    if (!flags.count("m")){
        cout << "Memory limit missing (specify in bytes)";
        return 0;
    }
    if (!flags.count("tj")){
        cout << "testinfo.json missing";
        return 0;
    }

    string compile_path = flags["i"], program_path = flags["o"];
    string command = "compiler.exe -i " + compile_path + " -o " + program_path;
    
    if (exec(command.c_str()) == 1){
        cout << "\033[1;31mCompilation failed\033[1;0m | See compiler message in compiler_message.txt";
        return 0;
    }   

    cout << "Compilation successful! Attempting to run test...\n";
    try{
        time_limit = stoll(flags["t"]);
    }
    catch (const exception& e){
        cout << "Error in parsing time limit string";
        return 0;
    }
    try{
        memory_limit = stoll(flags["m"]);
    }
    catch (const exception& e){
        cout << "Error in parsing memory limit string";
        return 0;
    }

    if (time_limit <= 0 || time_limit > 10000){
        cout << "Time limit must be between 0 (exclusive) and 10000 (inclusive)";
        return 0;
    }

    if (memory_limit <= 0 || time_limit > 1073741824){
        cout << "Memory limit must be between 0 (exclusive) and 1073741824 (inclusive)";
        return 0;
    }

    ifstream f(flags["tj"]);
    json j = json::parse(f);

    vector<ns::info> v = j["testcases"];
    for (int i = 1; i <= v.size(); i++){
        cout << "Running on test " + to_string(i) << "...\n";
        command = "executor.exe -i " + v[i - 1].input + " -o " + "executor_result.txt" + " -p " + program_path + " -t " + to_string(time_limit) + " -m " + to_string(memory_limit);
        exec(command.c_str());
        
        ifstream exe_f("executor_result.json");
        json exe_j = json::parse(exe_f);
        
        if (exe_j["Process result"] == 0){
            string cmm = "\"";
            command = "checker.exe " + cmm + v[i - 1].output + cmm + " executor_result.txt " + cmm + v[i - 1].output + cmm;
            exec(command.c_str());
        }
        else if (exe_j["Process result"] == 1)
            cout << "\033[1;33mTIME LIMIT EXCEEDED\033[1;0m | ";
        else if (exe_j["Process result"] == 2)
            cout << "\033[1;33mRUNTIME ERROR\033[1;0m | Exit code: " << exe_j["Exit code"] << " | ";
        else if (exe_j["Process result"] == 3)
            cout << "\033[1;33mMEMORY LIMIT EXCEEDED\033[1;0m | ";
        else if (exe_j["Process result"] == 4)
            cout << "\033[1;31mINTERNAL ERROR\033[1;0m | Error message: " << exe_j["Error message"] << " | ";

        cout << exe_j["Running time"] << "ms " << (double)exe_j["Max memory usage"] / (double)(1024 * 1024) << "MB\n";
        exe_f.close(); 
    }

    f.close();
}