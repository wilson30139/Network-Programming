#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include <arpa/inet.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_SINGLE_LINE_INPUT_SIZE 15005

using namespace std;

/*---------- Class User ----------*/

class User {
    public:
        int ID = -1;
        string nickname = "";
        string IP = "";
        int port = -1;
        int sock = -1;
        string PATHValue = "";
};

vector<User> userTable;

/*---------- Class HappenError ----------*/

class HappenError {
    public:
        bool isForSender = false; // <
        bool isForReceiver = false; // >
};

/*---------- Class UserPipeCommand ----------*/

class UserPipeCommand {
    public:
        bool isForSender = false; // <
        bool isForReceiver = false; // >
        HappenError happenError;
};

/*---------- Class NumberedPipeCommand ----------*/

class NumberedPipeCommand {
    public:
        bool isFirstType = false; // |n
        bool isSecondType = false; // !n
};

/*---------- Class Pipe ----------*/

class Pipe {
    public:
        int inFD = -1;
        int outFD = -1;
};

/*---------- Class RemainingCommandCountData ----------*/

class RemainingCommandCountData {
    public:
        int remainingCommandCount = -1;
        Pipe formerPipe;
        Pipe latterPipe;
};

/*---------- Class UserCommand ----------*/

class UserCommand {
    public:
        int ID = -1;
        string fileName = "";
        UserPipeCommand userPipeCommand;
        NumberedPipeCommand numberedPipeCommand;
        bool isOrdinaryCommand = false;
        bool isOrdinaryPipeCommand = false;
        bool needReceivePipe = false;
        bool needCreatePipe = false;
        bool needConnectPipe = false;
        vector<vector<string>> commandList;
        vector<RemainingCommandCountData> remainingCommandCountTable;
};

vector<UserCommand> userCommandTable;

/*---------- Class UserPipe ----------*/

class UserPipe {
    public:
        vector<string> displayCommand;
        int senderID = -1;
        int mediatorID = -1;
        int receiverID = -1;
        Pipe formerPipe;
        Pipe latterPipe;
};

vector<UserPipe> userPipeTable;

/*---------- Class User ----------*/

void AddUserForUserTable(User user);
User GetUserForUserTable(int userID);
void UpdateUserNicknameForUserTable(int userID, string userNickname);
void UpdateUserPATHValueForUserTable(int userID, string userPATHValue);
void DeleteUserForUserTable(int userID);
void OrderUserListForUserTable();
bool CompareByUserIDForUserTable(const User& former, const User& latter);
bool CheckUserExistForUserTable(int userID);

/*---------- Class HappenError ----------*/

/*---------- Class UserPipeCommand ----------*/

/*---------- Class NumberedPipeCommand ----------*/

/*---------- Class Pipe ----------*/

/*---------- Class RemainingCommandCountData ----------*/

void AddRemainingCommandCountDataForRemainingCommandCountTable(UserCommand &userCommand, RemainingCommandCountData remainingCommandCountData);
Pipe GetLatterPipeForRemainingCommandCountTable(UserCommand userCommand, int remainingCommandCount);
void UpdateRemainingCommandCountDataListForRemainingCommandCountTable(UserCommand &userCommand);
void DeleteRemainingCommandCountDataListForRemainingCommandCountTable(UserCommand &userCommand);

/*---------- Class UserCommand ----------*/

void AddUserCommandForUserCommandTable(UserCommand userCommand);
UserCommand GetUserCommandForUserCommandTable(int userID);
void DeleteUserCommandForUserCommandTable(int userID);
void OrderUserCommandListForUserCommandTable();
bool CompareByUserIDForUserCommandTable(const UserCommand& former, const UserCommand& latter);

/*---------- Class UserPipe ----------*/

void AddUserPipeForUserPipeTable(UserPipe userPipe);
Pipe GetLatterPipeForUserPipeTable(int mediatorID, int receiverID);
void DeleteUserPipeForUserPipeTable(int mediatorID, int receiverID);
bool CheckUserPipeExistForUserPipeTable(int formerID, int latterID, char userPipeSymbol);
void CleanUserPipeForUserPipeTable(int userID);

/*---------- Start ----------*/

void HandleSignal(int signal);

int EnableServerListenSocket(int serverListenPort);

void SetServerPATHValue();

bool CheckReadFDSetActive(int serverListenSock, fd_set &readFDSet);

void SendMessage(int userSock, string message);

/*---------- Handle Server Listen Sock ----------*/

int GetUserID();
string GetUserPATHValue();

/*---------- Handle User Sock ----------*/

void SetUserPATHValue(string userPATHValue);

void InitializeUserCommandSetting(UserCommand &userCommand, bool isChangeFileNameParameter, bool isChangeUserPipeCommandParameter);

void HandleUserExitProcedure(User user, fd_set &activeFDSet);

string RemoveLineFeedAndCarriageReturn(string singleLineInput);

bool CheckClickCtrlC(string singleLineInput);
bool CheckClickCtrlD(string singleLineInput);

vector<string> SplitSingleLineInput(string singleLineInput);

void PrintEnv(string envVariable, User user);
void SetEnv(string envVariable, string envVariableValue);

void ShowWelcomeMessage(User user);
void BroadcastLoginMessage(User user);
void BroadcastLogoutMessage(User user);
void ExecuteWhoCommand(User user);
void ExecuteTellCommand(User sender, int receiverID, vector<string> message);
void ExecuteYellCommand(User user, vector<string> message);
void ExecuteNameCommand(User &user, string userNickname);

bool CheckFileRedirectionSymbol(vector<string> wordListSingleLineInput);
void GetFileName(UserCommand &userCommand, vector<string> wordListSingleLineInput);
void RemoveFileRedirectionSymbol(vector<string> &wordListSingleLineInput);
void RedirectFile(string fileName);

bool CheckUserPipeCommand(char userPipeSymbol, vector<string> wordListSingleLineInput);
void RemoveUserPipeSymbol(char userPipeSymbol, vector<string> &wordListSingleLineInput);

void ShowUserErrorMessage(int userID, int userSock);

void ShowUserPipeErrorMessageForSender(int senderID, int mediatorID, int mediatorSock);
void BroadcastUserPipeMessageForSender(UserPipe userPipe);
void ShowUserPipeErrorMessageForReceiver(int mediatorID, int receiverID, int mediatorSock);
void BroadcastUserPipeMessageForReceiver(UserPipe userPipe);

vector<vector<string>> BuildCommandList(vector<string> wordListSingleLineInput);

bool CheckNumberedPipeCommand(char numberedPipeSymbol, vector<string> command);
bool CheckOrdinaryPipeCommand(vector<string> command);
bool CheckReceivePipe(UserCommand userCommand);
bool CheckCreatePipe(UserCommand userCommand, int remainingCommandCount);
bool CheckConnectPipe(UserCommand userCommand, int remainingCommandCount);

int GetRemainingCommandCountFromNumberedPipeCommand(vector<string> numberedPipeCommand);
void RemoveNumberedPipeSymbol(vector<string> &numberedPipeCommand);

vector<vector<string>> SplitOrdinaryPipeCommand(vector<string> ordinaryPipeCommand);

void ExecuteCommand(vector<string> command);

/*---------- Main ----------*/

int main(int argc, char const *argv[]) {
    // Start
    signal(SIGCHLD, HandleSignal);

    int serverListenPort = atoi(argv[1]);
    int serverListenSock = EnableServerListenSocket(serverListenPort);

    fd_set activeFDSet;
    FD_ZERO(&activeFDSet);
    FD_SET(serverListenSock, &activeFDSet);

    while (true) {
        SetServerPATHValue();

        fd_set readFDSet;
        memcpy(&readFDSet, &activeFDSet, sizeof(readFDSet));

        int maxFDValue = serverListenSock;
        if (userTable.size() > 0) {
            for (int i = 0; i < userTable.size(); i++) {
                if (maxFDValue < userTable[i].sock)
                    maxFDValue = userTable[i].sock;
            }
        }

        select(maxFDValue + 1, &readFDSet, NULL, NULL, NULL);

        if (CheckReadFDSetActive(serverListenSock, readFDSet))
            continue;

        // Handle Server Listen Sock
        if (FD_ISSET(serverListenSock, &readFDSet)) {
            struct sockaddr_in userLocation;
            int userLocationSize = sizeof(userLocation);
            int userSock = accept(serverListenSock, (struct sockaddr *) &userLocation, (socklen_t *) &userLocationSize);
            FD_SET(userSock, &activeFDSet);

            User user;
            user.ID = GetUserID();
            if (user.ID == -1) {
                FD_CLR(userSock, &activeFDSet);
                close(userSock);
            }
            user.nickname = "(no name)";
            user.IP = inet_ntoa(userLocation.sin_addr);
            user.port = ntohs(userLocation.sin_port);
            user.sock = userSock;
            user.PATHValue = GetUserPATHValue();
            AddUserForUserTable(user);

            UserCommand userCommand;
            userCommand.ID = user.ID;
            AddUserCommandForUserCommandTable(userCommand);

            ShowWelcomeMessage(user);

            BroadcastLoginMessage(user);

            SendMessage(user.sock, "% ");
        }

        // Handle User Sock
        if (userTable.size() > 0) {
            for (int i = 0; i < userTable.size(); i++) {
                if (FD_ISSET(userTable[i].sock, &readFDSet)) {
                    SetUserPATHValue(userTable[i].PATHValue);

                    bool isChangeFileNameParameter = true;
                    bool isChangeUserPipeCommandParameter = true;
                    InitializeUserCommandSetting(userCommandTable[i], isChangeFileNameParameter, isChangeUserPipeCommandParameter);
                    
                    char buffer[MAX_SINGLE_LINE_INPUT_SIZE] = {0};
                    int bufferSize = read(userTable[i].sock, buffer, MAX_SINGLE_LINE_INPUT_SIZE);

                    // If user close connection, remove the user from user table
                    if (bufferSize == 0) {
                        HandleUserExitProcedure(userTable[i], activeFDSet);
                        continue;
                    }

                    string singleLineInput(buffer);

                    singleLineInput = RemoveLineFeedAndCarriageReturn(singleLineInput);

                    if (singleLineInput.empty()) {
                        SendMessage(userTable[i].sock, "% ");
                        continue;
                    }
                    
                    // If user click Ctrl C or D, remove the user from user table
                    if (CheckClickCtrlC(singleLineInput) || CheckClickCtrlD(singleLineInput)) {
                        HandleUserExitProcedure(userTable[i], activeFDSet);
                        continue;
                    }

                    vector<string> wordListSingleLineInput = SplitSingleLineInput(singleLineInput);

                    // If user input exit, remove the user from user table
                    if (wordListSingleLineInput[0] == "exit") {
                        HandleUserExitProcedure(userTable[i], activeFDSet);
                        continue;
                    }

                    if (wordListSingleLineInput[0] == "printenv") {
                        if (userCommandTable[i].remainingCommandCountTable.size() > 0)
                            UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommandTable[i]);
                        PrintEnv(wordListSingleLineInput[1], userTable[i]);
                        SendMessage(userTable[i].sock, "% ");
                        continue;
                    }

                    if (wordListSingleLineInput[0] == "setenv") {
                        if (userCommandTable[i].remainingCommandCountTable.size() > 0)
                            UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommandTable[i]);
                        SetEnv(wordListSingleLineInput[1], wordListSingleLineInput[2]);
                        if (wordListSingleLineInput[1] == "PATH") {
                            userTable[i].PATHValue = wordListSingleLineInput[2];
                            UpdateUserPATHValueForUserTable(userTable[i].ID, wordListSingleLineInput[2]);
                        }
                        SendMessage(userTable[i].sock, "% ");
                        continue;
                    }

                    if (wordListSingleLineInput[0] == "who") {
                        if (userCommandTable[i].remainingCommandCountTable.size() > 0)
                            UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommandTable[i]);
                        ExecuteWhoCommand(userTable[i]);
                        SendMessage(userTable[i].sock, "% ");
                        continue;
                    }

                    if (wordListSingleLineInput[0] == "tell") {
                        if (userCommandTable[i].remainingCommandCountTable.size() > 0)
                            UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommandTable[i]);
                        ExecuteTellCommand(userTable[i], stoi(wordListSingleLineInput[1]), wordListSingleLineInput);
                        SendMessage(userTable[i].sock, "% ");
                        continue;
                    }

                    if (wordListSingleLineInput[0] == "yell") {
                        if (userCommandTable[i].remainingCommandCountTable.size() > 0)
                            UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommandTable[i]);
                        ExecuteYellCommand(userTable[i], wordListSingleLineInput);
                        SendMessage(userTable[i].sock, "% ");
                        continue;
                    }

                    if (wordListSingleLineInput[0] == "name") {
                        if (userCommandTable[i].remainingCommandCountTable.size() > 0)
                            UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommandTable[i]);
                        ExecuteNameCommand(userTable[i], wordListSingleLineInput[1]);
                        SendMessage(userTable[i].sock, "% ");
                        continue;
                    }

                    int fd[2];
                    pid_t childProcessID;
                    UserPipe userPipe;
                    userPipe.displayCommand = wordListSingleLineInput;

                    if (CheckFileRedirectionSymbol(wordListSingleLineInput)) {
                        GetFileName(userCommandTable[i], wordListSingleLineInput);
                        RemoveFileRedirectionSymbol(wordListSingleLineInput);
                    }

                    if (CheckUserPipeCommand('<', wordListSingleLineInput)) {
                        userCommandTable[i].userPipeCommand.isForSender = true;

                        for (int j = 0; j < wordListSingleLineInput.size(); j++) {
                            if ((wordListSingleLineInput[j][0] == '<') && (wordListSingleLineInput[j].size() > 1)) {
                                userPipe.senderID = stoi(wordListSingleLineInput[j].substr(1));
                                break;
                            }
                        }
                        userPipe.mediatorID = userTable[i].ID;

                        RemoveUserPipeSymbol('<', wordListSingleLineInput);
                        
                        if (!CheckUserExistForUserTable(userPipe.senderID)) {
                            ShowUserErrorMessage(userPipe.senderID, userTable[i].sock);
                            userCommandTable[i].userPipeCommand.happenError.isForSender = true;
                        }

                        if (!userCommandTable[i].userPipeCommand.happenError.isForSender) {
                            if (!CheckUserPipeExistForUserPipeTable(userPipe.senderID, userPipe.mediatorID, '<')) {
                                ShowUserPipeErrorMessageForSender(userPipe.senderID, userPipe.mediatorID, userTable[i].sock);
                                userCommandTable[i].userPipeCommand.happenError.isForSender = true;
                            }
                        }

                        if (!userCommandTable[i].userPipeCommand.happenError.isForSender) {
                            BroadcastUserPipeMessageForSender(userPipe);
                            userPipe.formerPipe = GetLatterPipeForUserPipeTable(userPipe.senderID, userPipe.mediatorID);
                        }
                    }

                    if (CheckUserPipeCommand('>', wordListSingleLineInput)) {
                        userCommandTable[i].userPipeCommand.isForReceiver = true;

                        if (!userCommandTable[i].userPipeCommand.isForSender)
                            userPipe.mediatorID = userTable[i].ID;
                        for (int j = 0; j < wordListSingleLineInput.size(); j++) {
                            if ((wordListSingleLineInput[j][0] == '>') && (wordListSingleLineInput[j].size() > 1)) {
                                userPipe.receiverID = stoi(wordListSingleLineInput[j].substr(1));
                                break;
                            }
                        }

                        RemoveUserPipeSymbol('>', wordListSingleLineInput);
                        
                        if (!CheckUserExistForUserTable(userPipe.receiverID)) {
                            ShowUserErrorMessage(userPipe.receiverID, userTable[i].sock);
                            userCommandTable[i].userPipeCommand.happenError.isForReceiver = true;
                        }

                        if (!userCommandTable[i].userPipeCommand.happenError.isForReceiver) {
                            if (CheckUserPipeExistForUserPipeTable(userPipe.mediatorID, userPipe.receiverID, '>')) {
                                ShowUserPipeErrorMessageForReceiver(userPipe.mediatorID, userPipe.receiverID, userTable[i].sock);
                                userCommandTable[i].userPipeCommand.happenError.isForReceiver = true;
                            }
                        }

                        if (!userCommandTable[i].userPipeCommand.happenError.isForReceiver)
                            BroadcastUserPipeMessageForReceiver(userPipe);
                    }

                    vector<vector<string>> commandList = BuildCommandList(wordListSingleLineInput);
                    for (int j = 0; j < commandList.size(); j++) {
                        RemainingCommandCountData remainingCommandCountData;

                        bool isChangeFileNameParameter = false;
                        bool isChangeUserPipeCommandParameter = false;
                        InitializeUserCommandSetting(userCommandTable[i], isChangeFileNameParameter, isChangeUserPipeCommandParameter);

                        if (userCommandTable[i].remainingCommandCountTable.size() > 0)
                            UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommandTable[i]);

                        if (CheckNumberedPipeCommand('|', commandList[j])) {
                            userCommandTable[i].numberedPipeCommand.isFirstType = true;
                            remainingCommandCountData.remainingCommandCount = GetRemainingCommandCountFromNumberedPipeCommand(commandList[j]);
                            RemoveNumberedPipeSymbol(commandList[j]);
                        }
                        if (CheckNumberedPipeCommand('!', commandList[j])) {
                            userCommandTable[i].numberedPipeCommand.isSecondType = true;
                            remainingCommandCountData.remainingCommandCount = GetRemainingCommandCountFromNumberedPipeCommand(commandList[j]);
                            RemoveNumberedPipeSymbol(commandList[j]);
                        }
                        if (!CheckOrdinaryPipeCommand(commandList[j])) {
                            userCommandTable[i].isOrdinaryCommand = true;
                            userCommandTable[i].commandList.push_back(commandList[j]);
                        }
                        if (CheckOrdinaryPipeCommand(commandList[j])) {
                            userCommandTable[i].isOrdinaryPipeCommand = true;
                            userCommandTable[i].commandList = SplitOrdinaryPipeCommand(commandList[j]);
                        }
                        if (CheckReceivePipe(userCommandTable[i]))
                            userCommandTable[i].needReceivePipe = true;
                        if (CheckCreatePipe(userCommandTable[i], remainingCommandCountData.remainingCommandCount))
                            userCommandTable[i].needCreatePipe = true;
                        if (CheckConnectPipe(userCommandTable[i], remainingCommandCountData.remainingCommandCount))
                            userCommandTable[i].needConnectPipe = true;

                        if (userCommandTable[i].isOrdinaryCommand) {
                            if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                if ((!userCommandTable[i].needReceivePipe) && (!userCommandTable[i].needConnectPipe))
                                    remainingCommandCountData.formerPipe = userPipe.formerPipe;
                            }
                            if (userCommandTable[i].needReceivePipe)
                                remainingCommandCountData.formerPipe = GetLatterPipeForRemainingCommandCountTable(userCommandTable[i], 0);
                            if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                if (userCommandTable[i].needCreatePipe) {
                                    pipe(fd);
                                    remainingCommandCountData.latterPipe.inFD = fd[0];
                                    remainingCommandCountData.latterPipe.outFD = fd[1];
                                }
                            }
                            if (userCommandTable[i].userPipeCommand.isForReceiver && (!userCommandTable[i].userPipeCommand.happenError.isForReceiver)) {
                                pipe(fd);
                                remainingCommandCountData.latterPipe.inFD = fd[0];
                                remainingCommandCountData.latterPipe.outFD = fd[1];
                                userPipe.latterPipe = remainingCommandCountData.latterPipe;
                            }
                            if (userCommandTable[i].needConnectPipe)
                                remainingCommandCountData.latterPipe = GetLatterPipeForRemainingCommandCountTable(userCommandTable[i], remainingCommandCountData.remainingCommandCount);
                            childProcessID = fork();
                            if (childProcessID == 0) {
                                if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                    if ((!userCommandTable[i].needReceivePipe) && (!userCommandTable[i].needConnectPipe))
                                        dup2(remainingCommandCountData.formerPipe.inFD, STDIN_FILENO);
                                }
                                if (userCommandTable[i].needReceivePipe)
                                    dup2(remainingCommandCountData.formerPipe.inFD, STDIN_FILENO);
                                if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                    if (userCommandTable[i].needCreatePipe || userCommandTable[i].needConnectPipe)
                                        dup2(remainingCommandCountData.latterPipe.outFD, STDOUT_FILENO);
                                }
                                if (userCommandTable[i].userPipeCommand.isForReceiver && (!userCommandTable[i].userPipeCommand.happenError.isForReceiver))
                                    dup2(remainingCommandCountData.latterPipe.outFD, STDOUT_FILENO);
                                if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                    if ((!userCommandTable[i].needCreatePipe) && (!userCommandTable[i].needConnectPipe)) {
                                        if (userCommandTable[i].fileName == "")
                                            dup2(userTable[i].sock, STDOUT_FILENO);
                                        if (userCommandTable[i].fileName != "")
                                            RedirectFile(userCommandTable[i].fileName);
                                    }
                                }
                                if (userCommandTable[i].userPipeCommand.happenError.isForSender || userCommandTable[i].userPipeCommand.happenError.isForReceiver) {
                                    int trashFileFD = open("/dev/null", O_RDWR);
                                    if (userCommandTable[i].userPipeCommand.happenError.isForSender)
                                        dup2(trashFileFD, STDIN_FILENO);
                                    if (userCommandTable[i].userPipeCommand.happenError.isForReceiver)
                                        dup2(trashFileFD, STDOUT_FILENO);
                                    close(trashFileFD);
                                }
                                if (!userCommandTable[i].numberedPipeCommand.isSecondType)
                                    dup2(userTable[i].sock, STDERR_FILENO);
                                if (userCommandTable[i].numberedPipeCommand.isSecondType)
                                    dup2(remainingCommandCountData.latterPipe.outFD, STDERR_FILENO);
                                if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                    if ((!userCommandTable[i].needReceivePipe) && (!userCommandTable[i].needConnectPipe)) {
                                        close(remainingCommandCountData.formerPipe.inFD);
                                        close(remainingCommandCountData.formerPipe.outFD);
                                    }
                                }
                                if (userCommandTable[i].needReceivePipe) {
                                    close(remainingCommandCountData.formerPipe.inFD);
                                    close(remainingCommandCountData.formerPipe.outFD);
                                }
                                if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                    if (userCommandTable[i].needCreatePipe || userCommandTable[i].needConnectPipe) {
                                        close(remainingCommandCountData.latterPipe.inFD);
                                        close(remainingCommandCountData.latterPipe.outFD);
                                    }
                                }
                                if (userCommandTable[i].userPipeCommand.isForReceiver && (!userCommandTable[i].userPipeCommand.happenError.isForReceiver)) {
                                    close(remainingCommandCountData.latterPipe.inFD);
                                    close(remainingCommandCountData.latterPipe.outFD);
                                }
                                ExecuteCommand(userCommandTable[i].commandList[0]);
                            }
                            if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                if ((!userCommandTable[i].needReceivePipe) && (!userCommandTable[i].needConnectPipe)) {
                                    close(remainingCommandCountData.formerPipe.inFD);
                                    close(remainingCommandCountData.formerPipe.outFD);
                                }
                            }
                            if (userCommandTable[i].needReceivePipe) {
                                close(remainingCommandCountData.formerPipe.inFD);
                                close(remainingCommandCountData.formerPipe.outFD);
                            }
                            if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                if ((!userCommandTable[i].needCreatePipe) && (!userCommandTable[i].needConnectPipe))
                                    waitpid(childProcessID, NULL, 0);
                            }
                            if (userCommandTable[i].userPipeCommand.happenError.isForSender || userCommandTable[i].userPipeCommand.happenError.isForReceiver)
                                waitpid(childProcessID, NULL, 0);
                            if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                if (userCommandTable[i].needCreatePipe || userCommandTable[i].needConnectPipe)
                                    AddRemainingCommandCountDataForRemainingCommandCountTable(userCommandTable[i], remainingCommandCountData);
                            }
                            if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                if ((!userCommandTable[i].needCreatePipe) && (!userCommandTable[i].needConnectPipe))
                                    DeleteUserPipeForUserPipeTable(userPipe.senderID, userPipe.mediatorID);
                            }
                            if (userCommandTable[i].userPipeCommand.isForReceiver && (!userCommandTable[i].userPipeCommand.happenError.isForReceiver))
                                AddUserPipeForUserPipeTable(userPipe);
                                
                        }

                        Pipe formerPipe;
                        Pipe latterPipe;
                        if (userCommandTable[i].isOrdinaryPipeCommand) {
                            for (int k = 0; k < userCommandTable[i].commandList.size(); k++) {
                                if (k == 0) {
                                    if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                        if ((!userCommandTable[i].needReceivePipe) && (!userCommandTable[i].needConnectPipe))
                                            remainingCommandCountData.formerPipe = userPipe.formerPipe;
                                    }
                                    if (userCommandTable[i].needReceivePipe)
                                        remainingCommandCountData.formerPipe = GetLatterPipeForRemainingCommandCountTable(userCommandTable[i], 0);
                                }
                                if (k > 0) {
                                    formerPipe.inFD = latterPipe.inFD;
                                    formerPipe.outFD = latterPipe.outFD;
                                }
                                if (k < (userCommandTable[i].commandList.size() - 1)) {
                                    pipe(fd);
                                    latterPipe.inFD = fd[0];
                                    latterPipe.outFD = fd[1];
                                }
                                if (k == (userCommandTable[i].commandList.size() - 1)) {
                                    if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                        if (userCommandTable[i].needCreatePipe) {
                                            pipe(fd);
                                            remainingCommandCountData.latterPipe.inFD = fd[0];
                                            remainingCommandCountData.latterPipe.outFD = fd[1];
                                        }
                                    }
                                    if (userCommandTable[i].userPipeCommand.isForReceiver && (!userCommandTable[i].userPipeCommand.happenError.isForReceiver)) {
                                        pipe(fd);
                                        remainingCommandCountData.latterPipe.inFD = fd[0];
                                        remainingCommandCountData.latterPipe.outFD = fd[1];
                                        userPipe.latterPipe = remainingCommandCountData.latterPipe;
                                    }
                                    if (userCommandTable[i].needConnectPipe)
                                        remainingCommandCountData.latterPipe = GetLatterPipeForRemainingCommandCountTable(userCommandTable[i], remainingCommandCountData.remainingCommandCount);
                                }
                                childProcessID = fork();
                                if (childProcessID == 0) {
                                    if (k == 0) {
                                        if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                            if ((!userCommandTable[i].needReceivePipe) && (!userCommandTable[i].needConnectPipe))
                                                dup2(remainingCommandCountData.formerPipe.inFD, STDIN_FILENO);
                                        }
                                        if (userCommandTable[i].needReceivePipe)
                                            dup2(remainingCommandCountData.formerPipe.inFD, STDIN_FILENO);
                                    }
                                    if (k > 0)
                                        dup2(formerPipe.inFD, STDIN_FILENO);
                                    if (k < (userCommandTable[i].commandList.size() - 1))
                                        dup2(latterPipe.outFD, STDOUT_FILENO);
                                    if (k == (userCommandTable[i].commandList.size() - 1)) {
                                        if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                            if (userCommandTable[i].needCreatePipe || userCommandTable[i].needConnectPipe)
                                                dup2(remainingCommandCountData.latterPipe.outFD, STDOUT_FILENO);
                                        }
                                        if (userCommandTable[i].userPipeCommand.isForReceiver && (!userCommandTable[i].userPipeCommand.happenError.isForReceiver))
                                            dup2(remainingCommandCountData.latterPipe.outFD, STDOUT_FILENO);
                                        if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                            if ((!userCommandTable[i].needCreatePipe) && (!userCommandTable[i].needConnectPipe)) {
                                                if (userCommandTable[i].fileName == "")
                                                    dup2(userTable[i].sock, STDOUT_FILENO);
                                                if (userCommandTable[i].fileName != "")
                                                    RedirectFile(userCommandTable[i].fileName);
                                            }
                                        }
                                        if (userCommandTable[i].userPipeCommand.happenError.isForSender || userCommandTable[i].userPipeCommand.happenError.isForReceiver) {
                                            int trashFileFD = open("/dev/null", O_RDWR);
                                            if (userCommandTable[i].userPipeCommand.happenError.isForSender)
                                                dup2(trashFileFD, STDIN_FILENO);
                                            if (userCommandTable[i].userPipeCommand.happenError.isForReceiver)
                                                dup2(trashFileFD, STDOUT_FILENO);
                                            close(trashFileFD);
                                        }
                                    }
                                    if (k < (userCommandTable[i].commandList.size() - 1)) {
                                        if (!userCommandTable[i].numberedPipeCommand.isSecondType)
                                            dup2(userTable[i].sock, STDERR_FILENO);
                                        if (userCommandTable[i].numberedPipeCommand.isSecondType)
                                            dup2(latterPipe.outFD, STDERR_FILENO);
                                    }
                                    if (k == (userCommandTable[i].commandList.size() - 1)) {
                                        if (!userCommandTable[i].numberedPipeCommand.isSecondType)
                                            dup2(userTable[i].sock, STDERR_FILENO);
                                        if (userCommandTable[i].numberedPipeCommand.isSecondType)
                                            dup2(remainingCommandCountData.latterPipe.outFD, STDERR_FILENO);
                                    }
                                    if (k == 0) {
                                        if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                            if ((!userCommandTable[i].needReceivePipe) && (!userCommandTable[i].needConnectPipe)) {
                                                close(remainingCommandCountData.formerPipe.inFD);
                                                close(remainingCommandCountData.formerPipe.outFD);
                                            }
                                        }
                                        if (userCommandTable[i].needReceivePipe) {
                                            close(remainingCommandCountData.formerPipe.inFD);
                                            close(remainingCommandCountData.formerPipe.outFD);
                                        }
                                    }
                                    if (k > 0) {
                                        close(formerPipe.inFD);
                                        close(formerPipe.outFD);
                                    }
                                    if (k < (userCommandTable[i].commandList.size() - 1)) {
                                        close(latterPipe.inFD);
                                        close(latterPipe.outFD);
                                    }
                                    if (k == (userCommandTable[i].commandList.size() - 1)) {
                                        if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                            if (userCommandTable[i].needCreatePipe || userCommandTable[i].needConnectPipe) {
                                                close(remainingCommandCountData.latterPipe.inFD);
                                                close(remainingCommandCountData.latterPipe.outFD);
                                            }
                                        }
                                        if (userCommandTable[i].userPipeCommand.isForReceiver && (!userCommandTable[i].userPipeCommand.happenError.isForReceiver)) {
                                            close(remainingCommandCountData.latterPipe.inFD);
                                            close(remainingCommandCountData.latterPipe.outFD);
                                        }
                                    }
                                    ExecuteCommand(userCommandTable[i].commandList[k]);
                                }
                                if (k == 0) {
                                    if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                        if ((!userCommandTable[i].needReceivePipe) && (!userCommandTable[i].needConnectPipe)) {
                                            close(remainingCommandCountData.formerPipe.inFD);
                                            close(remainingCommandCountData.formerPipe.outFD);
                                        }
                                    }
                                    if (userCommandTable[i].needReceivePipe) {
                                        close(remainingCommandCountData.formerPipe.inFD);
                                        close(remainingCommandCountData.formerPipe.outFD);
                                    }
                                }
                                if (k > 0) {
                                    close(formerPipe.inFD);
                                    close(formerPipe.outFD);
                                }    
                            }
                            if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                if ((!userCommandTable[i].needCreatePipe) && (!userCommandTable[i].needConnectPipe))
                                    waitpid(childProcessID, NULL, 0);
                            }
                            if (userCommandTable[i].userPipeCommand.happenError.isForSender || userCommandTable[i].userPipeCommand.happenError.isForReceiver)
                                waitpid(childProcessID, NULL, 0);
                            if (!userCommandTable[i].userPipeCommand.isForReceiver) {
                                if (userCommandTable[i].needCreatePipe || userCommandTable[i].needConnectPipe)
                                    AddRemainingCommandCountDataForRemainingCommandCountTable(userCommandTable[i], remainingCommandCountData);
                            }
                            if (userCommandTable[i].userPipeCommand.isForSender && (!userCommandTable[i].userPipeCommand.happenError.isForSender)) {
                                if ((!userCommandTable[i].needCreatePipe) && (!userCommandTable[i].needConnectPipe))
                                    DeleteUserPipeForUserPipeTable(userPipe.senderID, userPipe.mediatorID);
                            }
                            if (userCommandTable[i].userPipeCommand.isForReceiver && (!userCommandTable[i].userPipeCommand.happenError.isForReceiver))
                                AddUserPipeForUserPipeTable(userPipe);
                        }

                        DeleteRemainingCommandCountDataListForRemainingCommandCountTable(userCommandTable[i]);
                    }

                    usleep(10000);
                    SendMessage(userTable[i].sock, "% ");
                }
            }
        }
    }

    return 0;
}

/*---------- Class User ----------*/

void AddUserForUserTable(User user) {
    userTable.push_back(user);
    OrderUserListForUserTable();
}

User GetUserForUserTable(int userID) {
    User user;
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++) {
            if (userID == userTable[i].ID) {
                user = userTable[i];
                break;
            }
        }
    }
    return user;
}

void UpdateUserNicknameForUserTable(int userID, string userNickname) {
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++) {
            if (userID == userTable[i].ID) {
                userTable[i].nickname = userNickname;
                break;
            }
        }
    }
}

void UpdateUserPATHValueForUserTable(int userID, string userPATHValue) {
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++) {
            if (userID == userTable[i].ID) {
                userTable[i].PATHValue = userPATHValue;
                break;
            }
        }
    }
}

void DeleteUserForUserTable(int userID) {
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++) {
            if (userID == userTable[i].ID) {
                userTable.erase(userTable.begin() + i);
                break;
            }
        }
    }
}

void OrderUserListForUserTable() {
    sort(userTable.begin(), userTable.end(), CompareByUserIDForUserTable);
}

bool CompareByUserIDForUserTable(const User& former, const User& latter) {
    return former.ID < latter.ID;
}

bool CheckUserExistForUserTable(int userID) {
    for (int i = 0; i < userTable.size(); i++) {
        if (userID == userTable[i].ID)
            return true;
    }
    return false;
}

/*---------- Class HappenError ----------*/

/*---------- Class UserPipeCommand ----------*/

/*---------- Class NumberedPipeCommand ----------*/

/*---------- Class Pipe ----------*/

/*---------- Class RemainingCommandCountData ----------*/

void AddRemainingCommandCountDataForRemainingCommandCountTable(UserCommand &userCommand, RemainingCommandCountData remainingCommandCountData) {
    userCommand.remainingCommandCountTable.push_back(remainingCommandCountData);
}

Pipe GetLatterPipeForRemainingCommandCountTable(UserCommand userCommand, int remainingCommandCount) {
    Pipe latterPipe;
    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
        if (remainingCommandCount == userCommand.remainingCommandCountTable[i].remainingCommandCount) {
            latterPipe = userCommand.remainingCommandCountTable[i].latterPipe;
            break;
        }
    }
    return latterPipe;
}

void UpdateRemainingCommandCountDataListForRemainingCommandCountTable(UserCommand &userCommand) {
    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++)
        userCommand.remainingCommandCountTable[i].remainingCommandCount -= 1;
}

void DeleteRemainingCommandCountDataListForRemainingCommandCountTable(UserCommand &userCommand) {
    vector<int> deleteIndex;
    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
        if (userCommand.remainingCommandCountTable[i].remainingCommandCount == 0)
            deleteIndex.push_back(i);
    }
    for (int i = 0; i < deleteIndex.size(); i++)
        userCommand.remainingCommandCountTable.erase(userCommand.remainingCommandCountTable.begin() + deleteIndex[deleteIndex.size() - 1 - i]);
}

/*---------- Class UserCommand ----------*/

void AddUserCommandForUserCommandTable(UserCommand userCommand) {
    userCommandTable.push_back(userCommand);
    OrderUserCommandListForUserCommandTable();
}

UserCommand GetUserCommandForUserCommandTable(int userID) {
    UserCommand userCommand;
    if (userCommandTable.size() > 0) {
        for (int i = 0; i < userCommandTable.size(); i++) {
            if (userID == userCommandTable[i].ID) {
                userCommand = userCommandTable[i];
                break;
            }
        }
    }
    return userCommand;
}

void DeleteUserCommandForUserCommandTable(int userID) {
    if (userCommandTable.size() > 0) {
        for (int i = 0; i < userCommandTable.size(); i++) {
            if (userID == userCommandTable[i].ID) {
                userCommandTable.erase(userCommandTable.begin() + i);
                break;
            }
        }
    }
}

void OrderUserCommandListForUserCommandTable() {
    sort(userCommandTable.begin(), userCommandTable.end(), CompareByUserIDForUserCommandTable);
}

bool CompareByUserIDForUserCommandTable(const UserCommand& former, const UserCommand& latter) {
    return former.ID < latter.ID;
}

/*---------- Class UserPipe ----------*/

void AddUserPipeForUserPipeTable(UserPipe userPipe) {
    userPipeTable.push_back(userPipe);
}

Pipe GetLatterPipeForUserPipeTable(int mediatorID, int receiverID) {
    Pipe latterPipe;
    for (int i = 0; i < userPipeTable.size(); i++) {
        if ((mediatorID == userPipeTable[i].mediatorID) && (receiverID == userPipeTable[i].receiverID)) {
            latterPipe = userPipeTable[i].latterPipe;
        }
    }
    return latterPipe;
}

void DeleteUserPipeForUserPipeTable(int mediatorID, int receiverID) {
    for (int i = 0; i < userPipeTable.size(); i++) {
        if ((mediatorID == userPipeTable[i].mediatorID) && (receiverID == userPipeTable[i].receiverID)) {
            userPipeTable.erase(userPipeTable.begin() + i);
            break;
        }
    }
}

bool CheckUserPipeExistForUserPipeTable(int formerID, int latterID, char userPipeSymbol) {
    if (userPipeSymbol == '<') {
        for (int i = 0; i < userPipeTable.size(); i++) {
            if ((formerID == userPipeTable[i].mediatorID) && (latterID == userPipeTable[i].receiverID))
                return true;
        }
    }
    if (userPipeSymbol == '>') {
        for (int i = 0; i < userPipeTable.size(); i++) {
            if ((formerID == userPipeTable[i].mediatorID) && (latterID == userPipeTable[i].receiverID))
                return true;
        }
    }
    return false;
}

void CleanUserPipeForUserPipeTable(int userID) {
    pid_t childProcessID;
    int trashFileFD = -1;
    vector<string> command;
    command.push_back("ls");
    for (int i = 0; i < userPipeTable.size(); i++) {
        if ((userID == userPipeTable[i].mediatorID) || (userID == userPipeTable[i].receiverID)) {
            childProcessID = fork();
            if (childProcessID == 0) {
                dup2(userPipeTable[i].latterPipe.inFD, STDIN_FILENO);
                trashFileFD = open("/dev/null", O_RDWR);
                dup2(trashFileFD, STDOUT_FILENO);
                close(trashFileFD);
                close(userPipeTable[i].latterPipe.inFD);
                close(userPipeTable[i].latterPipe.outFD);
                ExecuteCommand(command);
            }
            close(userPipeTable[i].latterPipe.inFD);
            close(userPipeTable[i].latterPipe.outFD);
            waitpid(childProcessID, NULL, 0);
            DeleteUserPipeForUserPipeTable(userPipeTable[i].mediatorID, userPipeTable[i].receiverID);
        }
    }
}

/*---------- Start ----------*/

void HandleSignal(int signal) {
    if (signal == SIGCHLD) {
        while (true) {
            if (waitpid(-1, NULL, WNOHANG) <= 0)
                break;
        }
        return;
    }
}

int EnableServerListenSocket(int serverListenPort) {
    int serverListenSock = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(serverListenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serverLocation;
    serverLocation.sin_family = AF_INET;
    serverLocation.sin_addr.s_addr = INADDR_ANY;
    serverLocation.sin_port = htons(serverListenPort);
    bind(serverListenSock, (struct sockaddr *) &serverLocation, sizeof(serverLocation));

    listen(serverListenSock, 30);

    return serverListenSock;
}

void SetServerPATHValue() {
    SetEnv("PATH", "bin:.");
}

bool CheckReadFDSetActive(int serverListenSock, fd_set &readFDSet) {
    bool isHaveRemainingCommandCountData = false;
    if (userCommandTable.size() > 0) {
        for (int i = 0; i < userCommandTable.size(); i++) {
            if (userCommandTable[i].remainingCommandCountTable.size() > 0) {
                isHaveRemainingCommandCountData = true;
                break;
            }
        }
    }
    if (isHaveRemainingCommandCountData || (userPipeTable.size() > 0)) {
        if (!FD_ISSET(serverListenSock, &readFDSet))
            return false;
        if (userTable.size() > 0) {
            for (int i = 0; i < userTable.size(); i++) {
                if (!FD_ISSET(userTable[i].sock, &readFDSet))
                    return false;
            }
        }
        return true;
    }
    return false;
}

void SendMessage(int userSock, string message) {
    write(userSock, message.c_str(), message.length());
}

/*---------- Handle Server Listen Sock ----------*/

int GetUserID() {
    int userID = -1;

    if (userTable.size() >= 30)
        return userID;

    if (userTable.size() == 0) {
        userID = 1;
        return userID;
    }

    for (int i = 1; i <= 30; i++) {
        for (int j = 0; j < userTable.size(); j++) {
            if (i == userTable[j].ID)
                break;
            if (j == (userTable.size() - 1))
                userID = i;
        }
        if (userID != -1)
            break;
    }

    return userID;
}

string GetUserPATHValue() {
    const char* userPATHValueC = getenv("PATH");
    string userPATHValueStr = userPATHValueC;
    return userPATHValueStr;
}

/*---------- Handle User Sock ----------*/

void SetUserPATHValue(string userPATHValue) {
    SetEnv("PATH", userPATHValue);
}

void InitializeUserCommandSetting(UserCommand &userCommand, bool isChangeFileNameParameter, bool isChangeUserPipeCommandParameter) {
    if (isChangeFileNameParameter)
        userCommand.fileName = "";
    if (isChangeUserPipeCommandParameter) {
        userCommand.userPipeCommand.isForSender = false;
        userCommand.userPipeCommand.isForReceiver = false;
        userCommand.userPipeCommand.happenError.isForSender = false;
        userCommand.userPipeCommand.happenError.isForReceiver = false;
    }
    userCommand.numberedPipeCommand.isFirstType = false;
    userCommand.numberedPipeCommand.isSecondType = false;
    userCommand.isOrdinaryCommand = false;
    userCommand.isOrdinaryPipeCommand = false;
    userCommand.needReceivePipe = false;
    userCommand.needCreatePipe = false;
    userCommand.needConnectPipe = false;
    userCommand.commandList.clear();
}

void HandleUserExitProcedure(User user, fd_set &activeFDSet) {
    BroadcastLogoutMessage(user);
    close(user.sock);
    FD_CLR(user.sock, &activeFDSet);
    DeleteUserForUserTable(user.ID);
    DeleteUserCommandForUserCommandTable(user.ID);
    CleanUserPipeForUserPipeTable(user.ID);
}

string RemoveLineFeedAndCarriageReturn(string singleLineInput) {
    string tempSingleLineInput = "";
    for (int i = 0; i < singleLineInput.size(); i++) {
        if ((singleLineInput[i] != '\r') && (singleLineInput[i] != '\n'))
            tempSingleLineInput += singleLineInput[i];
    }
    return tempSingleLineInput;
}

bool CheckClickCtrlC(string singleLineInput) {
    for (int i = 0; i < singleLineInput.size(); i++) {
        if ((i == 0) && (singleLineInput[i] != -1))
            return false;
        if ((i == 1) && (singleLineInput[i] != -12))
            return false;
        if ((i == 2) && (singleLineInput[i] != -1))
            return false;
        if ((i == 3) && (singleLineInput[i] != -3))
            return false;
        if ((i == 4) && (singleLineInput[i] != 6))
            return false;
        if (i == (singleLineInput.size() - 1))
            return true;
    }
    return true;
}

bool CheckClickCtrlD(string singleLineInput) {
    if (singleLineInput[0] == 4)
        return true;
    return false;
}

vector<string> SplitSingleLineInput(string singleLineInput) {
    vector<string> wordListSingleLineInput;
    string wordSingleLineInput;
    istringstream iss(singleLineInput);
    while (iss >> wordSingleLineInput)
        wordListSingleLineInput.push_back(wordSingleLineInput);
    return wordListSingleLineInput;
}

void PrintEnv(string envVariable, User user) {
    const char* envVariableValue = getenv(envVariable.c_str());
    if (envVariableValue != NULL) {
        ostringstream oss;
        oss << envVariableValue << endl;
        string envVariableValueMessage = oss.str();
        SendMessage(user.sock, envVariableValueMessage);
    }
}

void SetEnv(string envVariable, string envVariableValue) {
    setenv(envVariable.c_str(), envVariableValue.c_str(), 1);
}

void ShowWelcomeMessage(User user) {
    ostringstream oss;
    oss << "****************************************" << endl;
    oss << "** Welcome to the information server. **" << endl;
    oss << "****************************************" << endl;
    string welcomeMessage = oss.str();
    SendMessage(user.sock, welcomeMessage);
}

void BroadcastLoginMessage(User user) {
    ostringstream oss;
    oss << "*** User '" << user.nickname << "' entered from " << user.IP << ":" << user.port << ". ***" << endl;
    string loginMessage = oss.str();
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++)
            SendMessage(userTable[i].sock, loginMessage);
    }
}

void BroadcastLogoutMessage(User user) {
    ostringstream oss;
    oss << "*** User '" << user.nickname << "' left. ***" << endl;
    string logoutMessage = oss.str();
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++) {
            if (user.ID != userTable[i].ID)
                SendMessage(userTable[i].sock, logoutMessage);
        }
    }
}

void ExecuteWhoCommand(User user) {
    ostringstream oss;
    oss << "<ID>\t<nickname>\t<IP:port>\t<indicate me>" << endl;
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++) {
            oss << userTable[i].ID << "\t" << userTable[i].nickname << "\t" << userTable[i].IP << ":" << userTable[i].port;
            if (user.ID == userTable[i].ID)
                oss << "\t<-me";
            oss << endl;
        }
    }
    string whoMessage = oss.str();
    SendMessage(user.sock, whoMessage);
}

void ExecuteTellCommand(User sender, int receiverID, vector<string> message) {
    User receiver = GetUserForUserTable(receiverID);
    
    ostringstream oss;
    string tellMessage = "";
    if (receiver.ID == -1) {
        oss << "*** Error: user #" << receiverID << " does not exist yet. ***" << endl;
        tellMessage = oss.str();
        SendMessage(sender.sock, tellMessage);
        return;
    }

    oss << "*** " << sender.nickname << " told you ***: ";
    for (int i = 2; i < message.size(); i++) {
        oss << message[i];
        if (i < (message.size() - 1))
            oss << " ";
    }
    oss << endl;
    tellMessage = oss.str();
    SendMessage(receiver.sock, tellMessage);
}

void ExecuteYellCommand(User user, vector<string> message) {
    ostringstream oss;
    oss << "*** " << user.nickname << " yelled ***: ";
    for (int i = 1; i < message.size(); i++) {
        oss << message[i];
        if (i < (message.size() - 1))
            oss << " ";
    }
    oss << endl;
    string yellMessage = oss.str();
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++)
            SendMessage(userTable[i].sock, yellMessage);
    }
}

void ExecuteNameCommand(User &user, string userNickname) {
    ostringstream oss;
    string nameMessage = "";
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++) {
            if (userNickname == userTable[i].nickname) {
                oss << "*** User '" << userTable[i].nickname << "' already exists. ***" << endl;
                nameMessage = oss.str();
                SendMessage(user.sock, nameMessage);
                return;
            }
        }
    }

    user.nickname = userNickname;
    UpdateUserNicknameForUserTable(user.ID, userNickname);
    oss << "*** User from " << user.IP << ":" << user.port << " is named '" << user.nickname <<"'. ***" << endl;
    nameMessage = oss.str();
    if (userTable.size() > 0) {
        for (int i = 0; i < userTable.size(); i++)
            SendMessage(userTable[i].sock, nameMessage);
    }
}

bool CheckFileRedirectionSymbol(vector<string> wordListSingleLineInput) {
    if (wordListSingleLineInput.size() > 0) {
        for (int i = 0; i < wordListSingleLineInput.size(); i++) {
            if ((wordListSingleLineInput[i][0] == '>') && (wordListSingleLineInput[i].size() == 1))
                return true;
        }
    }
    return false;
}

void GetFileName(UserCommand &userCommand, vector<string> wordListSingleLineInput) {
    if (wordListSingleLineInput.size() > 0) {
        for (int i = 0; i < wordListSingleLineInput.size(); i++) {
            if ((wordListSingleLineInput[i][0] == '>') && (wordListSingleLineInput[i].size() == 1)) {
                userCommand.fileName = wordListSingleLineInput[i + 1];
                break;
            }
        }
    }
}

void RemoveFileRedirectionSymbol(vector<string> &wordListSingleLineInput) {
    if (wordListSingleLineInput.size() > 0) {
        for (int i = 0; i < wordListSingleLineInput.size(); i++) {
            if ((wordListSingleLineInput[i][0] == '>') && (wordListSingleLineInput[i].size() == 1)) {
                for (int time = 1; time <= 2; time++)
                    wordListSingleLineInput.erase(wordListSingleLineInput.begin() + i);
                break;
            }
        }
    }
}

void RedirectFile(string fileName) {
    int fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(fd, STDOUT_FILENO);
    close(fd);
}

bool CheckUserPipeCommand(char userPipeSymbol, vector<string> wordListSingleLineInput) {
    for (int i = 0; i < wordListSingleLineInput.size(); i++) {
        if ((wordListSingleLineInput[i][0] == userPipeSymbol) && (wordListSingleLineInput[i].size() > 1)) {
            for (int j = 1; j < wordListSingleLineInput[i].size(); j++) {
                char ch = wordListSingleLineInput[i][j];
                int value = ch - '0';
                for (int k = 0; k <= 9; k++) {
                    if (value == k)
                        break;
                    if (k == 9)
                        return false;
                }
            }
            return true;
        }
    }
    return false;
}

void RemoveUserPipeSymbol(char userPipeSymbol, vector<string> &wordListSingleLineInput) {
    for (int i = 0; i < wordListSingleLineInput.size(); i++) {
        if ((wordListSingleLineInput[i][0] == userPipeSymbol) && (wordListSingleLineInput[i].size() > 1)) {
            wordListSingleLineInput.erase(wordListSingleLineInput.begin() + i);
            break;
        }
    }
}

void ShowUserErrorMessage(int userID, int userSock) {
    ostringstream oss;
    oss << "*** Error: user #" << userID << " does not exist yet. ***" << endl;
    string userPipeErrorMessage = oss.str();
    SendMessage(userSock, userPipeErrorMessage);
}

void ShowUserPipeErrorMessageForSender(int senderID, int mediatorID, int mediatorSock) {
    ostringstream oss;
    oss << "*** Error: the pipe #" << senderID << "->#" << mediatorID << " does not exist yet. ***" << endl;
    string userPipeErrorMessage = oss.str();
    SendMessage(mediatorSock, userPipeErrorMessage);
}

void BroadcastUserPipeMessageForSender(UserPipe userPipe) {
    User sender = GetUserForUserTable(userPipe.senderID);
    User mediator = GetUserForUserTable(userPipe.mediatorID);
    ostringstream oss;
    oss << "*** " << mediator.nickname << " (#" << mediator.ID << ") just received from " << sender.nickname << " (#" << sender.ID << ") by '";
    for (int i = 0; i < userPipe.displayCommand.size(); i++) {
        oss << userPipe.displayCommand[i];
        if (i < (userPipe.displayCommand.size() - 1))
            oss << " ";
    }
    oss << "' ***" << endl;
    string userPipeMessage = oss.str();
    for (int i = 0; i < userTable.size(); i++)
        SendMessage(userTable[i].sock, userPipeMessage);
}

void ShowUserPipeErrorMessageForReceiver(int mediatorID, int receiverID, int mediatorSock) {
    ostringstream oss;
    oss << "*** Error: the pipe #" << mediatorID << "->#" << receiverID << " already exists. ***" << endl;
    string userPipeErrorMessage = oss.str();
    SendMessage(mediatorSock, userPipeErrorMessage);
}

void BroadcastUserPipeMessageForReceiver(UserPipe userPipe) {
    User mediator = GetUserForUserTable(userPipe.mediatorID);
    User receiver = GetUserForUserTable(userPipe.receiverID);
    ostringstream oss;
    oss << "*** " << mediator.nickname << " (#" << mediator.ID << ") just piped '";
    for (int i = 0; i < userPipe.displayCommand.size(); i++) {
        oss << userPipe.displayCommand[i];
        if (i < (userPipe.displayCommand.size() - 1))
            oss << " ";
    }
    oss << "' to " << receiver.nickname << " (#" << receiver.ID << ") ***" << endl;
    string userPipeMessage = oss.str();
    for (int i = 0; i < userTable.size(); i++)
        SendMessage(userTable[i].sock, userPipeMessage);
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

bool CheckNumberedPipeCommand(char numberedPipeSymbol, vector<string> command) {
    int finalIndex = command.size() - 1;
    if ((command[finalIndex][0] == numberedPipeSymbol) && (command[finalIndex].size() > 1)) {
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

bool CheckOrdinaryPipeCommand(vector<string> command) {
    for (int i = 0; i < command.size(); i++) {
        if ((command[i] == "|") && (command[i].size() == 1))
            return true;
    }
    return false;
}

bool CheckReceivePipe(UserCommand userCommand) {
    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
        if (userCommand.remainingCommandCountTable[i].remainingCommandCount == 0)
            return true;
    }
    return false;
}

bool CheckCreatePipe(UserCommand userCommand, int remainingCommandCount) {
    if (userCommand.numberedPipeCommand.isFirstType && (!userCommand.numberedPipeCommand.isSecondType)) {
        if (userCommand.remainingCommandCountTable.size() == 0)
            return true;
        if (!CheckConnectPipe(userCommand, remainingCommandCount))
            return true;
    }
    if ((!userCommand.numberedPipeCommand.isFirstType) && userCommand.numberedPipeCommand.isSecondType) {
        if (userCommand.remainingCommandCountTable.size() == 0)
            return true;
        if (!CheckConnectPipe(userCommand, remainingCommandCount))
            return true;
    }
    if ((!userCommand.numberedPipeCommand.isFirstType) && (!userCommand.numberedPipeCommand.isSecondType))
        return false;
    return false;
}

bool CheckConnectPipe(UserCommand userCommand, int remainingCommandCount) {
    if (userCommand.numberedPipeCommand.isFirstType && (!userCommand.numberedPipeCommand.isSecondType)) {
        for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
            if (remainingCommandCount == userCommand.remainingCommandCountTable[i].remainingCommandCount)
                return true;
        }
    }
    if ((!userCommand.numberedPipeCommand.isFirstType) && userCommand.numberedPipeCommand.isSecondType) {
        for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
            if (remainingCommandCount == userCommand.remainingCommandCountTable[i].remainingCommandCount)
                return true;
        }
    }
    if ((!userCommand.numberedPipeCommand.isFirstType) && (!userCommand.numberedPipeCommand.isSecondType))
        return false;
    return false;
}

int GetRemainingCommandCountFromNumberedPipeCommand(vector<string> numberedPipeCommand) {
    int finalIndex = numberedPipeCommand.size() - 1;
    int remainingCommandCount = stoi(numberedPipeCommand[finalIndex].substr(1));
    return remainingCommandCount;
}

void RemoveNumberedPipeSymbol(vector<string> &numberedPipeCommand) {
    numberedPipeCommand.pop_back();
}

vector<vector<string>> SplitOrdinaryPipeCommand(vector<string> ordinaryPipeCommand) {
    vector<vector<string>> commandList;
    vector<string> command;
    int start = 0;
    int end = 0;

    for (int i = 0; i < ordinaryPipeCommand.size(); i++) {
        if (ordinaryPipeCommand[i] == "|") {
            end = i - 1;
            for (int j = start; j <= end; j++)
                command.push_back(ordinaryPipeCommand[j]);
            start = i + 1;
            commandList.push_back(command);
            command.clear();
        }
        if (i == (ordinaryPipeCommand.size() - 1)) {
            end = i;
            for (int j = start; j <= end; j++)
                command.push_back(ordinaryPipeCommand[j]);
            commandList.push_back(command);
        }
    }

    return commandList;
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