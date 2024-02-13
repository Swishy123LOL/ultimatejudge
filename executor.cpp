#include <bits/stdc++.h>
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <nlohmann/json.hpp>

#define BUFSIZE 1024

using namespace std;
using json = nlohmann::json;

enum proc_result{
    succeed = 0, tle = 1, rte = 2, mle = 3, internal = 4
};

struct result{
    proc_result res;
    int exitcode, runtime, max_memusage;
    string error_msg = "ok";

    result(proc_result _res, int _exitcode, int _runtime, int _max_memusage) : res(_res), exitcode(_exitcode), runtime(_runtime), max_memusage(_max_memusage){}; 
    result(proc_result _res, string msg){
        res = _res; error_msg = msg;
        exitcode = runtime = max_memusage = 0;
    }
};

long long memory_limit = 300000000; //bytes
long long time_limit = 2000; //miliseconds

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;
HANDLE g_hInputFile = NULL;

string input_file = "input.txt", output_file = "output.txt";

bool createPipe(){
    SECURITY_ATTRIBUTES saAttr; 
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 

    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) 
        return 0;
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        return 0;   
    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
        return 0;   
    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        return 0;   
    return 1;
}

bool writeToPipe() { 
    DWORD dwRead, dwWritten; 
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;
    for (;;){ 
        bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
        if (!dwRead)
            break;

        if (!bSuccess) return 0;
        
        bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
        if (!bSuccess){
            auto err = GetLastError();
            cout << err << endl;
            return 0;
        }
    } 

    if (!CloseHandle(g_hChildStd_IN_Wr))
        return 0;
         
    return 1;
} 
 
bool readFromPipe(){
    DWORD dwRead, dwWritten; 
    CHAR chBuf[BUFSIZE]; 
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    for (;;){ 
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (dwRead == 0)
            break;
        if (!bSuccess) return 0;

        bSuccess = WriteFile(hParentStdOut, chBuf, dwRead, &dwWritten, NULL);
        if (!bSuccess) return 0;
    } 
    return 1;
} 

result exec(const char* name){
    if (!createPipe())
        return result((proc_result)4, "failed to create pipe");

    TCHAR szCmdline[1000];
    strcpy(szCmdline, name);

    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE; 

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
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

    if (!bSuccess)
        return result((proc_result)4, "failed to create process");

    g_hInputFile = CreateFile(
        input_file.c_str(), 
        GENERIC_READ, 
        0, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_READONLY, 
        NULL); ;

    DWORD exitcode;
    PROCESS_MEMORY_COUNTERS pmc;

    auto closeHandle = [](auto &piProcInfo){
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    };

    CloseHandle(g_hChildStd_OUT_Wr);
    CloseHandle(g_hChildStd_IN_Rd);

    long long max_mem = 0;
    auto begin = chrono::high_resolution_clock::now();
    auto current = chrono::high_resolution_clock::now();
    long long crr_time = 0;

    if (!writeToPipe()){
        closeHandle(piProcInfo);
        return result((proc_result)4, "failed to write to pipe");
    }

    while (1){
        GetExitCodeProcess(piProcInfo.hProcess, &exitcode);
        if (exitcode != 259)
            break;

        current = chrono::high_resolution_clock::now();
        crr_time = chrono::duration_cast<std::chrono::milliseconds>(current - begin).count();

        cout << crr_time << endl;
        if (crr_time >= time_limit){
            UINT excode = 0;
            TerminateProcess(piProcInfo.hProcess, excode);
            closeHandle(piProcInfo);
            return result((proc_result)1, excode, crr_time, max_mem);
        }

        if (max_mem >= memory_limit){
            UINT excode = 0;
            TerminateProcess(piProcInfo.hProcess, excode);
            closeHandle(piProcInfo);
            return result((proc_result)3, excode, crr_time, max_mem);
        }

        if (GetProcessMemoryInfo(piProcInfo.hProcess, &pmc, sizeof(pmc)))
            max_mem = pmc.PeakWorkingSetSize;
    }

    closeHandle(piProcInfo);
    if (exitcode == 0)
        return result((proc_result)0, 0, crr_time, max_mem);   
    else
        return result((proc_result)2, exitcode, crr_time, max_mem);
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

string program_name = "test.exe";
int main(int argc, char *argv[]){
    vector<string> args;
    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);
    map<string, string> flags = flag_parser(args);

    if (flags.count("i"))
        input_file = flags["i"];
    if (flags.count("o"))
        output_file = flags["o"];
    if (flags.count("p"))
        program_name = flags["p"];
    if (flags.count("t"))
        time_limit = stoll(flags["t"]);
    if (flags.count("m"))
        memory_limit = stod(flags["m"]);

    freopen(output_file.c_str(), "w", stdout);
    result res = exec(program_name.c_str());

    json j;
    j["Process result"] = res.res;
    j["Exit code"] = res.exitcode;
    j["Max memory usage"] = res.max_memusage;
    j["Running time"] = res.runtime;
    j["Error message"] = res.error_msg;

    ofstream out("executor_result.json");
    out << j.dump();
    out.close();
}