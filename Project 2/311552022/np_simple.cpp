#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>

using namespace std;

class LineCountData {
    public:
        int numberedPipeLineCount;
        vector<string> numberedPipeCommand;
        int numberedPipeBeforeInFD = -1;
        int numberedPipeBeforeOutFD = -1;
        int numberedPipeAfterInFD = -1;
        int numberedPipeAfterOutFD = -1;
};

vector<LineCountData> lineCountTable;
bool isOrdinaryPipe = false;
bool isNumberedPipeTwoType = false;

void ExecuteNPShell();

void HandleChildProcess(int signal);

void InitializeNPShell();

vector<string> SplitSingleLineInput(string singleLineInput);

void PrintEnv(string envVariable);
void SetEnv(string envVariable, string envVariableValue);

bool IsRedirectFile(vector<string> wordListSingleLineInput);

string GetFileName(vector<string> &wordListSingleLineInput);

vector<vector<string>> BuildCommandList(vector<string> wordListSingleLineInput);

bool IsNumberedPipeCommand(vector<string> command);
bool IsNumberedPipeCommandTwoType(vector<string> command);
bool IsOrdinaryPipeCommand(vector<string> numberedPipeCommand);

bool IsReceiveNumberedPipe();
bool IsNotCreateNumberedPipe(int numberedPipeLineCount);

int GetNumberedPipeLineCount(vector<string> command);
vector<string> GetNumberedPipeCommand(vector<string> command);
vector<vector<string>> GetOrdinaryPipeCommandList(vector<string> numberedPipeCommand);

void AddDataForLineCountTable(LineCountData lineCountData);
void UpdateDataListForLineCountTable();
void DeleteDataListForLineCountTable();
pair<int, int> GetNumberedPipeFDForLineCountTable(int numberedPipeLineCount);

void RedirectFile(string fileName);

void ExecuteCommand(vector<string> command);

int main(int argc, char const *argv[]) {
    InitializeNPShell();
    
    int mSock = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    int serverPort = atoi(argv[1]);
    struct sockaddr_in serverLocation;
    serverLocation.sin_family = AF_INET;
    serverLocation.sin_addr.s_addr = INADDR_ANY;
    serverLocation.sin_port = htons(serverPort);
    bind(mSock, (struct sockaddr *)&serverLocation, sizeof(serverLocation));

    listen(mSock, 1);

    while (true) {
        int sSock = accept(mSock, NULL, NULL);

        pid_t childProcessID = fork();
        if (childProcessID == 0) {
            close(mSock);
            dup2(sSock, STDIN_FILENO);
            dup2(sSock, STDOUT_FILENO);
            dup2(sSock, STDERR_FILENO);
            close(sSock);
            ExecuteNPShell();
            exit(0);
        }
        if (childProcessID != 0)
            close(sSock);
    }  

    return 0;
}

void ExecuteNPShell() {
    string singleLineInput;
    string tempSingleLineInput = "";
    vector<string> wordListSingleLineInput;
    string fileName = "";
    vector<vector<string>> commandList;
    int fd[2];
    pid_t childProcessID;

    while (true) {
        usleep(10000);

        cout << "% ";
        getline(cin, singleLineInput);

        // remove /r and /n
        tempSingleLineInput = "";
        for (int i = 0; i < singleLineInput.size(); i++) {
            if ((singleLineInput[i] != '\r') && (singleLineInput[i] != '\n'))
                tempSingleLineInput += singleLineInput[i];
        }
        singleLineInput = tempSingleLineInput;

        if (singleLineInput.empty())
            continue;

        // if Ctrl+D, disconnect
        if (singleLineInput[0] == 4)
            break;

        // if Ctrl+C, disconnect
        bool isClickCtrlC = false;
        for (int i = 0; i < singleLineInput.size(); i++) {
            if ((i == 0) && (singleLineInput[i] != -1))
                break;
            if ((i == 1) && (singleLineInput[i] != -12))
                break;
            if ((i == 2) && (singleLineInput[i] != -1))
                break;
            if ((i == 3) && (singleLineInput[i] != -3))
                break;
            if ((i == 4) && (singleLineInput[i] != 6))
                break;
            if (i == (singleLineInput.size() - 1))
                isClickCtrlC = true;
        }
        if (isClickCtrlC)
            break;

        wordListSingleLineInput = SplitSingleLineInput(singleLineInput);

        if (wordListSingleLineInput[0] == "exit")
            break;
        if (wordListSingleLineInput[0] == "printenv") {
            if (lineCountTable.size() > 0)
                UpdateDataListForLineCountTable();
            PrintEnv(wordListSingleLineInput[1]);
            continue;
        }
        if (wordListSingleLineInput[0] == "setenv") {
            if (lineCountTable.size() > 0)
                UpdateDataListForLineCountTable();
            SetEnv(wordListSingleLineInput[1], wordListSingleLineInput[2]);
            continue;
        }

        if (IsRedirectFile(wordListSingleLineInput))
            fileName = GetFileName(wordListSingleLineInput);
        
        commandList = BuildCommandList(wordListSingleLineInput);

        for (int i = 0; i < commandList.size(); i++) {
            if (IsNumberedPipeCommand(commandList[i])) {
                if (lineCountTable.size() > 0)
                    UpdateDataListForLineCountTable();

                isNumberedPipeTwoType = IsNumberedPipeCommandTwoType(commandList[i]);

                LineCountData lineCountData;
                lineCountData.numberedPipeLineCount = GetNumberedPipeLineCount(commandList[i]);
                lineCountData.numberedPipeCommand = GetNumberedPipeCommand(commandList[i]);

                isOrdinaryPipe = IsOrdinaryPipeCommand(lineCountData.numberedPipeCommand);
                
                if (isOrdinaryPipe) {
                    vector<vector<string>> ordinaryPipeCommandList = GetOrdinaryPipeCommandList(lineCountData.numberedPipeCommand);
                    pair<int, int> ordinaryPipeBeforeFD = make_pair(-1, -1);
                    for (int j = 0; j < (ordinaryPipeCommandList.size() - 1); j++) {
                        pipe(fd);
                        while (true) {
                            childProcessID = fork();
                            if (childProcessID >= 0)
                                break;
                            usleep(1000);
                        }
                        if (childProcessID == 0) {
                            if (j > 0)                                
                                dup2(ordinaryPipeBeforeFD.first, STDIN_FILENO);
                            dup2(fd[1], STDOUT_FILENO);
                            if (j > 0) {
                                close(ordinaryPipeBeforeFD.first);
                                close(ordinaryPipeBeforeFD.second);
                            }
                            close(fd[0]);
                            close(fd[1]);
                            ExecuteCommand(ordinaryPipeCommandList[j]);
                        }
                        if (j > 0) {
                            close(ordinaryPipeBeforeFD.first);
                            close(ordinaryPipeBeforeFD.second);
                        }
                        // waitpid(childProcessID, NULL, 0);
                        ordinaryPipeBeforeFD.first = fd[0];
                        ordinaryPipeBeforeFD.second = fd[1];
                    }
                    lineCountData.numberedPipeCommand = ordinaryPipeCommandList[ordinaryPipeCommandList.size() - 1];
                    lineCountData.numberedPipeBeforeInFD = ordinaryPipeBeforeFD.first;
                    lineCountData.numberedPipeBeforeOutFD = ordinaryPipeBeforeFD.second;
                }

                if ((lineCountTable.size() == 0) || IsReceiveNumberedPipe() || (!IsNotCreateNumberedPipe(lineCountData.numberedPipeLineCount))) {
                    pipe(fd);
                    lineCountData.numberedPipeAfterInFD = fd[0];
                    lineCountData.numberedPipeAfterOutFD = fd[1];
                }
                if (IsNotCreateNumberedPipe(lineCountData.numberedPipeLineCount)) {
                    pair<int, int> numberedPipeAfterFD = GetNumberedPipeFDForLineCountTable(lineCountData.numberedPipeLineCount);
                    lineCountData.numberedPipeAfterInFD = numberedPipeAfterFD.first;
                    lineCountData.numberedPipeAfterOutFD = numberedPipeAfterFD.second;
                }

                if (IsReceiveNumberedPipe()) {
                    pair<int, int> numberedPipeBeforeFD = GetNumberedPipeFDForLineCountTable(0);
                    lineCountData.numberedPipeBeforeInFD = numberedPipeBeforeFD.first;
                    lineCountData.numberedPipeBeforeOutFD = numberedPipeBeforeFD.second;
                    DeleteDataListForLineCountTable();
                    while (true) {
                        childProcessID = fork();
                        if (childProcessID >= 0)
                            break;
                        usleep(1000);
                    }
                    if (childProcessID == 0) {
                        dup2(lineCountData.numberedPipeBeforeInFD, STDIN_FILENO);
                        dup2(lineCountData.numberedPipeAfterOutFD, STDOUT_FILENO);
                        close(lineCountData.numberedPipeBeforeInFD);
                        close(lineCountData.numberedPipeBeforeOutFD);
                        close(lineCountData.numberedPipeAfterInFD);
                        close(lineCountData.numberedPipeAfterOutFD);
                        ExecuteCommand(lineCountData.numberedPipeCommand);
                    }
                    close(lineCountData.numberedPipeBeforeInFD);
                    close(lineCountData.numberedPipeBeforeOutFD);
                    // waitpid(childProcessID, NULL, 0);
                    AddDataForLineCountTable(lineCountData);
                    continue;
                }
                if (!IsReceiveNumberedPipe()) {
                    while (true) {
                        childProcessID = fork();
                        if (childProcessID >= 0)
                            break;
                        usleep(1000);
                    }
                    if (childProcessID == 0) {
                        if (isOrdinaryPipe)
                            dup2(lineCountData.numberedPipeBeforeInFD, STDIN_FILENO);
                        dup2(lineCountData.numberedPipeAfterOutFD, STDOUT_FILENO);
                        if (isNumberedPipeTwoType)
                            dup2(lineCountData.numberedPipeAfterOutFD, STDERR_FILENO);
                        if (isOrdinaryPipe) {
                            close(lineCountData.numberedPipeBeforeInFD);
                            close(lineCountData.numberedPipeBeforeOutFD);
                        }
                        close(lineCountData.numberedPipeAfterInFD);
                        close(lineCountData.numberedPipeAfterOutFD);
                        ExecuteCommand(lineCountData.numberedPipeCommand);
                    }
                    if (isOrdinaryPipe) {
                        close(lineCountData.numberedPipeBeforeInFD);
                        close(lineCountData.numberedPipeBeforeOutFD);
                    }
                    // waitpid(childProcessID, NULL, 0);
                    AddDataForLineCountTable(lineCountData);
                    isOrdinaryPipe = false;
                    isNumberedPipeTwoType = false;
                    continue;
                }
            }
            if (IsOrdinaryPipeCommand(commandList[i])) {
                if (lineCountTable.size() > 0)
                    UpdateDataListForLineCountTable();

                pair<int, int> numberedPipeBeforeFD;
                if (IsReceiveNumberedPipe())
                    numberedPipeBeforeFD = GetNumberedPipeFDForLineCountTable(0);
                
                vector<vector<string>> ordinaryPipeCommandList = GetOrdinaryPipeCommandList(commandList[i]);
                pair<int, int> ordinaryPipeBeforeFD = make_pair(-1, -1);
                for (int j = 0; j < ordinaryPipeCommandList.size(); j++) {
                    if (j < (ordinaryPipeCommandList.size() - 1))
                        pipe(fd);
                    while (true) {
                        childProcessID = fork();
                        if (childProcessID >= 0)
                            break;
                        usleep(1000);
                    }
                    if (childProcessID == 0) {
                        if ((j == 0) && IsReceiveNumberedPipe())
                            dup2(numberedPipeBeforeFD.first, STDIN_FILENO);
                        if (j > 0)
                            dup2(ordinaryPipeBeforeFD.first, STDIN_FILENO);
                        if (j < (ordinaryPipeCommandList.size() - 1))
                            dup2(fd[1], STDOUT_FILENO);
                        if ((j == 0) && IsReceiveNumberedPipe()) {
                            close(numberedPipeBeforeFD.first);
                            close(numberedPipeBeforeFD.second);
                        }
                        if (j > 0) {
                            close(ordinaryPipeBeforeFD.first);
                            close(ordinaryPipeBeforeFD.second);
                        }
                        if (j < (ordinaryPipeCommandList.size() - 1)) {
                            close(fd[0]);
                            close(fd[1]);
                        }
                        if((j == (ordinaryPipeCommandList.size() - 1)) && (fileName != ""))
                            RedirectFile(fileName);
                        ExecuteCommand(ordinaryPipeCommandList[j]);
                    }
                    if ((j == 0) && IsReceiveNumberedPipe()) {
                        close(numberedPipeBeforeFD.first);
                        close(numberedPipeBeforeFD.second);
                    }
                    if (j > 0) {
                        close(ordinaryPipeBeforeFD.first);
                        close(ordinaryPipeBeforeFD.second);
                    }
                    // waitpid(childProcessID, NULL, 0);
                    if (j == (ordinaryPipeCommandList.size() - 1))
                        waitpid(childProcessID, NULL, 0);
                    if (j < (ordinaryPipeCommandList.size() - 1)) {
                        ordinaryPipeBeforeFD.first = fd[0];
                        ordinaryPipeBeforeFD.second = fd[1];
                    }
                }
                fileName = "";

                if (IsReceiveNumberedPipe())
                    DeleteDataListForLineCountTable();

                continue;
            }
            if ((!IsNumberedPipeCommand(commandList[i])) && (!IsOrdinaryPipeCommand(commandList[i]))) {
                if (lineCountTable.size() > 0)
                    UpdateDataListForLineCountTable();

                if(IsReceiveNumberedPipe()) {
                    pair<int, int> numberedPipeBeforeFD = GetNumberedPipeFDForLineCountTable(0);
                    DeleteDataListForLineCountTable();
                    while (true) {
                        childProcessID = fork();
                        if (childProcessID >= 0)
                            break;
                        usleep(1000);
                    }
                    if (childProcessID == 0) {
                        dup2(numberedPipeBeforeFD.first, STDIN_FILENO);
                        close(numberedPipeBeforeFD.first);
                        close(numberedPipeBeforeFD.second);
                        if (fileName != "")
                            RedirectFile(fileName);
                        ExecuteCommand(commandList[i]);
                    }
                    close(numberedPipeBeforeFD.first);
                    close(numberedPipeBeforeFD.second);
                    waitpid(childProcessID, NULL, 0);
                    fileName = "";
                    continue;
                }
                if (!IsReceiveNumberedPipe()) {
                    while (true) {
                        childProcessID = fork();
                        if (childProcessID >= 0)
                            break;
                        usleep(1000);
                    }
                    if (childProcessID == 0) {
                        if (fileName != "")
                            RedirectFile(fileName);
                        ExecuteCommand(commandList[i]);
                    }
                    waitpid(childProcessID, NULL, 0);
                    fileName = "";
                    continue;
                }
            }
        }
    }
}

void HandleChildProcess(int signal) {
    while (true) {
        if (waitpid(-1, NULL, WNOHANG) <= 0)
            break;
    }
}

void InitializeNPShell() {
    SetEnv("PATH", "bin:.");
    signal(SIGCHLD, HandleChildProcess);
}

vector<string> SplitSingleLineInput(string singleLineInput) {
    vector<string> wordListSingleLineInput;
    string wordSingleLineInput;
    istringstream iss(singleLineInput);
    while (iss >> wordSingleLineInput)
        wordListSingleLineInput.push_back(wordSingleLineInput);
    return wordListSingleLineInput;
}

void PrintEnv(string envVariable) {
    const char* envVariableValue = getenv(envVariable.c_str());
    if (envVariableValue != NULL)
        cout << envVariableValue << endl;
}

void SetEnv(string envVariable, string envVariableValue) {
    setenv(envVariable.c_str(), envVariableValue.c_str(), 1);
}

bool IsRedirectFile(vector<string> wordListSingleLineInput) {
    for (int i = 0; i < wordListSingleLineInput.size(); i++) {
        if ((wordListSingleLineInput[i][0] == '>') && (wordListSingleLineInput[i].size() == 1))
            return true;
    }
    return false;
}

string GetFileName(vector<string> &wordListSingleLineInput) {
    string fileName = wordListSingleLineInput[wordListSingleLineInput.size() - 1];
    for (int times = 1; times <= 2; times++)
        wordListSingleLineInput.pop_back();
    return fileName;
}

vector<vector<string>> BuildCommandList(vector<string> wordListSingleLineInput) {
    vector<vector<string>> commandList;
    vector<string> command;
    int start = 0;
    int end = 0;

    for (int i = 0; i < wordListSingleLineInput.size(); i++) {
        if ((((wordListSingleLineInput[i][0] == '|') || (wordListSingleLineInput[i][0] == '!')) && (wordListSingleLineInput[i].size() > 1)) || (i == (wordListSingleLineInput.size() - 1))) {
            end = i;
            for (int j = start; j <= end; j++)
                command.push_back(wordListSingleLineInput[j]);
            start = i + 1;
            commandList.push_back(command);
            command.clear();
        }
    }

    return commandList;
}

bool IsNumberedPipeCommand(vector<string> command) {
    int finalIndex = command.size() - 1;
    if (((command[finalIndex][0] == '|') || (command[finalIndex][0] == '!')) && (command[finalIndex].size() > 1)) {
        for (int i = 1; i < command[finalIndex].size(); i++) {
            char ch = command[finalIndex][i];
            int value = ch - '0';
            for (int j = 0; j <= 9; j++) {
                if (value == j) {
                    break;
                }
                if (j == 9)
                    return false;
            }
        }
        return true;
    }
    return false;
}

bool IsNumberedPipeCommandTwoType(vector<string> command) {
    int finalIndex = command.size() - 1;
    if ((command[finalIndex][0] == '!') && (command[finalIndex].size() > 1)) {
        for (int i = 1; i < command[finalIndex].size(); i++) {
            char ch = command[finalIndex][i];
            int value = ch - '0';
            for (int j = 0; j <= 9; j++) {
                if (value == j)
                    break;
                if (j == 9)
                    return false;
            }
        }
        return true;
    }
    return false;
}

bool IsOrdinaryPipeCommand(vector<string> numberedPipeCommand) {
    for (int i = 0; i < numberedPipeCommand.size(); i++) {
        if ((numberedPipeCommand[i] == "|") && (numberedPipeCommand[i].size() == 1))
            return true;
    }
    return false;
}

bool IsReceiveNumberedPipe() {
    for (int i = 0; i < lineCountTable.size(); i++) {
        if (lineCountTable[i].numberedPipeLineCount == 0)
            return true;
    }
    return false;
}

bool IsNotCreateNumberedPipe(int numberedPipeLineCount) {
    for (int i = 0; i < lineCountTable.size(); i++) {
        if (numberedPipeLineCount == lineCountTable[i].numberedPipeLineCount)
            return true;
    }
    return false;
}

int GetNumberedPipeLineCount(vector<string> command) {
    int finalIndex = command.size() - 1;
    int numberedPipeLineCount = stoi(command[finalIndex].substr(1));
    return numberedPipeLineCount;
}

vector<string> GetNumberedPipeCommand(vector<string> command) {
    command.pop_back();
    return command;
}

vector<vector<string>> GetOrdinaryPipeCommandList(vector<string> numberedPipeCommand) {
    vector<vector<string>> ordinaryPipeCommandList;
    vector<string> ordinaryPipeCommand;
    int start = 0;
    int end = 0;

    for (int i = 0; i < numberedPipeCommand.size(); i++) {
        if (numberedPipeCommand[i] == "|") {
            end = i - 1;
            for (int j = start; j <= end; j++)
                ordinaryPipeCommand.push_back(numberedPipeCommand[j]);
            start = i + 1;
            ordinaryPipeCommandList.push_back(ordinaryPipeCommand);
            ordinaryPipeCommand.clear();
        }
        if (i == (numberedPipeCommand.size() - 1)) {
            end = i;
            for (int j = start; j <= end; j++)
                ordinaryPipeCommand.push_back(numberedPipeCommand[j]);
            ordinaryPipeCommandList.push_back(ordinaryPipeCommand);
        }
    }

    return ordinaryPipeCommandList;
}

void AddDataForLineCountTable(LineCountData lineCountData) {
    lineCountTable.push_back(lineCountData);
}

void UpdateDataListForLineCountTable() {
    for (int i = 0; i < lineCountTable.size(); i++)
        lineCountTable[i].numberedPipeLineCount -= 1;
}

void DeleteDataListForLineCountTable() {
    vector<LineCountData> tempLineCountTable;
    for (int i = 0; i < lineCountTable.size(); i++) {
        if (lineCountTable[i].numberedPipeLineCount != 0)
            tempLineCountTable.push_back(lineCountTable[i]);
    }
    lineCountTable = tempLineCountTable;
}

pair<int, int> GetNumberedPipeFDForLineCountTable(int numberedPipeLineCount) {
    pair<int, int> numberedPipeFD;
    for (int i = 0; i < lineCountTable.size(); i++) {
        if (numberedPipeLineCount == lineCountTable[i].numberedPipeLineCount) {
            numberedPipeFD.first = lineCountTable[i].numberedPipeAfterInFD;
            numberedPipeFD.second = lineCountTable[i].numberedPipeAfterOutFD;
            break;
        }
    }
    return numberedPipeFD;
}

void RedirectFile(string fileName) {
    int fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(fd, STDOUT_FILENO);
    close(fd);
}

void ExecuteCommand(vector<string> command) {
    const char *args[command.size() + 1];
    for (int i = 0; i < command.size(); i++)
        args[i] = command[i].c_str();
    args[command.size()] = NULL;
    if (execvp(args[0], const_cast<char* const*>(args)) == -1) {
        cerr << "Unknown command: [" << args[0] << "]." << endl;
        exit(1);
    }
}
