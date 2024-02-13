#include <bits/stdc++.h>
#include <windows.h>

using namespace std;

#define BUFSIZE 4096    

HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

bool createPipe(){
    SECURITY_ATTRIBUTES saAttr; 
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 

    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) 
        return 0;
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
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

bool exec(const char* name){
    if (!createPipe())
        return 0;
    
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE; 

    TCHAR szCmdline[1000];
    strcpy(szCmdline, name);

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
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

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    DWORD exitcode;
    GetExitCodeProcess(piProcInfo.hProcess, &exitcode);

    CloseHandle(g_hChildStd_OUT_Wr);
    if (!readFromPipe())
        return 0;
    // Close process and thread handles. 
    CloseHandle(g_hChildStd_OUT_Rd);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    if (exitcode != 0) return 0;
    return 1;
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

    freopen("compiler_message.txt", "w", stdout);
    if (argc <= 2) return 1;
    string compile_path = flags["i"], program_path = flags["o"];
    string command = "g++ -pedantic -Wall -Wextra -Wshadow -g3 -O0 " + compile_path + " -o " + program_path;

    if (!exec(command.c_str()))
        return 1;
    return 0;
}