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
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

/*---------- Class User ----------*/

#define MAX_USER_NICKNAME_SIZE 25
#define MAX_USER_IP_SIZE 20
#define MAX_USER_PATH_VALUE_SIZE 1000
#define USER_TABLE_SHARED_MEMORY_KEY ((key_t) 7890)
#define MAX_USER_COUNT 30

/*---------- Class Message ----------*/

#define MAX_MESSAGE_SIZE 1030
#define MESSAGE_SHARED_MEMORY_KEY ((key_t) 7891)

/*---------- Class FIFOFile ----------*/

#define FIFO_FILE_SHARED_MEMORY_KEY ((key_t) 7892)

/*---------- Class UserPipe ----------*/

#define USER_PIPE_TABLE_SHARED_MEMORY_KEY ((key_t) 7893)

/*---------- Start ----------*/

#define	MAX_SINGLE_LINE_INPUT_SIZE 15005

#define FIFO_DIRECTORY_NAME "./user_pipe/"

using namespace std;

/*---------- Class User ----------*/

class User {
    public:
        bool isActive = false;
        int ID = -1;
        char nickname[MAX_USER_NICKNAME_SIZE] = {0};
        char IP[MAX_USER_IP_SIZE] = {0};
        int port = -1;
        pid_t processID = -1;
        char PATHValue[MAX_USER_PATH_VALUE_SIZE] = {0};
};

int userTableSharedMemoryID = -1;
User *userTableSharedMemoryPointer;

/*---------- Class Message ----------*/

class Message {
    public:
        char content[MAX_MESSAGE_SIZE] = {0};
};

int messageSharedMemoryID = -1;
Message *messageSharedMemoryPointer;

/*---------- Class FIFOFile ----------*/

class FIFOFile {
    public:
        int name = 0;
};

int fifoFileSharedMemoryID = -1;
FIFOFile *fifoFileSharedMemoryPointer;

/*---------- Class UserPipe ----------*/

class UserPipe {
    public:
        bool isActive = false;
};

int userPipeTableSharedMemoryID = -1;
UserPipe *userPipeTableSharedMemoryPointer;

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

/*---------- Class RemainingCommandCountData ----------*/

class RemainingCommandCountData {
    public:
        int remainingCommandCount = -1;
        string formerFIFOFileName = "";
        string latterFIFOFileName = "";
        bool isConnect = false;
        int sortNumber = 1;
};

/*---------- Class UserCommand ----------*/

class UserCommand {
    public:
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

class UserPipeMessageType {
    public:
        bool isUserErrorMessage = false;
        bool isUserPipeErrorMessage = false;
        bool isUserPipeSuccessMessage = false;
};

class UserPipeMessageContent {
    public:
        string userErrorMessage;
        string userPipeErrorMessage;
}; 

class UserPipeMessage {
    public:
        UserPipeMessageType userPipeMessageTypeForSender;
        UserPipeMessageType userPipeMessageTypeForReceiver;
        UserPipeMessageContent userPipeMessageContentForSender;
        UserPipeMessageContent userPipeMessageContentForReceiver;
        string userPipeSuccessMessage;
};

/*---------- Class User ----------*/

void AddUserForUserTable(int userID, string userNickname, string userIP, int userPort, string userPATHValue);
void UpdateUserNicknameForUserTable(int userID, string userNickname);
void UpdateUserPATHValueForUserTable(int userID, string userPATHValue);
void DeleteUserForUserTable(int userID);
bool CheckUserExistForUserTable(int userID);

/*---------- Class Message ----------*/

/*---------- Class FIFOFile ----------*/

/*---------- Class UserPipe ----------*/

void AddUserPipeForUserPipeTable(int formerID, int latterID);
bool CheckUserPipeExistForUserPipeTable(int formerID, int latterID);
void DeleteUserPipeForUserPipeTable(int formerID, int latterID);
void CleanUserPipeForUserPipeTable(int userID);

/*---------- Class UserPipeCommand ----------*/

/*---------- Class NumberedPipeCommand ----------*/

/*---------- Class RemainingCommandCountData ----------*/

void AddRemainingCommandCountDataForRemainingCommandCountTable(UserCommand &userCommand, RemainingCommandCountData remainingCommandCountData);
string GetLatterFIFOFileNameForRemainingCommandCountTable(UserCommand userCommand, int remainingCommandCount);
int GetSortNumberForRemainingCommandCountTable(UserCommand userCommand, int remainingCommandCount);
void UpdateRemainingCommandCountDataListForRemainingCommandCountTable(UserCommand &userCommand);
void DeleteRemainingCommandCountDataListForRemainingCommandCountTable(UserCommand &userCommand);
bool CheckDealWithConnectForRemainingCommandCountTable(UserCommand userCommand);
void DealWithConnectForRemainingCommandCountTable(UserCommand &userCommand);

/*---------- Class UserCommand ----------*/

/*---------- Start ----------*/

void HandleServerSignal(int signal);
void HandleUserSignal(int signal);

pthread_mutex_t userTableMutexLock;
pthread_mutex_t messageMutexLock;
pthread_mutex_t fifoFileMutexLock;
pthread_mutex_t userPipeTableMutexLock;
void InitializeMutexLock();

void InitializeSharedMemory();

void InitializeFIFO();

int serverListenSock = -1;
int userSock = -1;
void EnableServerListenSocket(int serverListenPort);

void EnterUserEnv(sockaddr_in userLocation);

int GetUserID();
string GetUserPATHValue();

void ShowCommandLinePrompt();

void InitializeUserCommandSetting(UserCommand &userCommand, bool isChangeFileNameParameter, bool isChangeUserPipeCommandParameter);

string RemoveLineFeedAndCarriageReturn(string singleLineInput);

bool CheckClickCtrlC(string singleLineInput);
bool CheckClickCtrlD(string singleLineInput);

vector<string> SplitSingleLineInput(string singleLineInput);

void PrintEnv(string envVariable, int userID);
void SetEnv(string envVariable, string envVariableValue);

void ShowWelcomeMessage(int userID);
void BroadcastLoginMessage(int userID);
void BroadcastLogoutMessage(int userID);
void ExecuteWhoCommand(int userID);
void ExecuteTellCommand(int senderID, int receiverID, vector<string> message);
void ExecuteYellCommand(int userID, vector<string> message);
void ExecuteNameCommand(int userID, string userNickname);

bool CheckFileRedirectionSymbol(vector<string> wordListSingleLineInput);
void GetFileName(UserCommand &userCommand, vector<string> wordListSingleLineInput);
void RemoveFileRedirectionSymbol(vector<string> &wordListSingleLineInput);
void RedirectFile(string fileName);

bool CheckUserPipeCommand(char userPipeSymbol, vector<string> wordListSingleLineInput);
void RemoveUserPipeSymbol(char userPipeSymbol, vector<string> &wordListSingleLineInput);

string ShowUserErrorMessage(int errorUserID);

void ShowUserPipeErrorMessageForSender(int senderID, int mediatorID, UserPipeMessage &userPipeMessage);
void BroadcastUserPipeMessageForSender(int senderID, int mediatorID, vector<string> displayCommand, UserPipeMessage &userPipeMessage);
void ShowUserPipeErrorMessageForReceiver(int mediatorID, int receiverID, UserPipeMessage &userPipeMessage);
void BroadcastUserPipeMessageForReceiver(int mediatorID, int receiverID, vector<string> displayCommand, UserPipeMessage &userPipeMessage);

void ShowUserPipeMessage(UserPipeMessage userPipeMessage, int userID);

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
    signal(SIGINT, HandleServerSignal);
    signal(SIGCHLD, HandleServerSignal);

    SetEnv("PATH", "bin:.");

    InitializeMutexLock();

    InitializeSharedMemory();

    InitializeFIFO();

    int serverListenPort = atoi(argv[1]);
    EnableServerListenSocket(serverListenPort);

    while (true) {
        struct sockaddr_in userLocation;
        int userLocationSize = sizeof(userLocation);
        userSock = accept(serverListenSock, (struct sockaddr *) &userLocation, (socklen_t *) &userLocationSize);

        pid_t childProcessID = fork();
        if (childProcessID == 0) {
            close(serverListenSock);
            EnterUserEnv(userLocation);
        }
        if (childProcessID > 0)
            close(userSock);
    }

    return 0;
}

/*---------- Class User ----------*/

void AddUserForUserTable(int userID, string userNickname, string userIP, int userPort, string userPATHValue) {
    pthread_mutex_lock(&userTableMutexLock);
    if (!userTableSharedMemoryPointer[userID - 1].isActive) {
        userTableSharedMemoryPointer[userID - 1].isActive = true;
        userTableSharedMemoryPointer[userID - 1].ID = userID;
        strcpy(userTableSharedMemoryPointer[userID - 1].nickname, userNickname.c_str());
        strcpy(userTableSharedMemoryPointer[userID - 1].IP, userIP.c_str());
        userTableSharedMemoryPointer[userID - 1].port = userPort;
        userTableSharedMemoryPointer[userID - 1].processID = getpid();
        strcpy(userTableSharedMemoryPointer[userID - 1].PATHValue, userPATHValue.c_str());
    }
    pthread_mutex_unlock(&userTableMutexLock);
}

void UpdateUserNicknameForUserTable(int userID, string userNickname) {
    pthread_mutex_lock(&userTableMutexLock);
    strcpy(userTableSharedMemoryPointer[userID - 1].nickname, userNickname.c_str());
    pthread_mutex_unlock(&userTableMutexLock);
}

void UpdateUserPATHValueForUserTable(int userID, string userPATHValue) {
    pthread_mutex_lock(&userTableMutexLock);
    strcpy(userTableSharedMemoryPointer[userID - 1].PATHValue, userPATHValue.c_str());
    pthread_mutex_unlock(&userTableMutexLock);
}

void DeleteUserForUserTable(int userID) {
    pthread_mutex_lock(&userTableMutexLock);
    if (userTableSharedMemoryPointer[userID - 1].isActive) {
        userTableSharedMemoryPointer[userID - 1].isActive = false;
        userTableSharedMemoryPointer[userID - 1].ID = -1;
        strcpy(userTableSharedMemoryPointer[userID - 1].nickname, "");
        strcpy(userTableSharedMemoryPointer[userID - 1].IP, "");
        userTableSharedMemoryPointer[userID - 1].port = -1;
        userTableSharedMemoryPointer[userID - 1].processID = -1;
        strcpy(userTableSharedMemoryPointer[userID - 1].PATHValue, "");
    }
    pthread_mutex_unlock(&userTableMutexLock);
}

bool CheckUserExistForUserTable(int userID) {
    if ((userID < 1) || (userID > 30))
        return false;
    pthread_mutex_lock(&userTableMutexLock);
    if (userTableSharedMemoryPointer[userID - 1].isActive) {
        pthread_mutex_unlock(&userTableMutexLock);
        return true;
    }
    pthread_mutex_unlock(&userTableMutexLock);
    return false;
}

/*---------- Class Message ----------*/

/*---------- Class FIFOFile ----------*/

/*---------- Class UserPipe ----------*/

void AddUserPipeForUserPipeTable(int formerID, int latterID) {
    pthread_mutex_lock(&userPipeTableMutexLock);
    userPipeTableSharedMemoryPointer[(formerID - 1) * MAX_USER_COUNT + (latterID - 1)].isActive = true;
    pthread_mutex_unlock(&userPipeTableMutexLock);
}

bool CheckUserPipeExistForUserPipeTable(int formerID, int latterID) {
    pthread_mutex_lock(&userPipeTableMutexLock);
    if (userPipeTableSharedMemoryPointer[(formerID - 1) * MAX_USER_COUNT + (latterID - 1)].isActive) {
        pthread_mutex_unlock(&userPipeTableMutexLock);
        return true;
    }
    pthread_mutex_unlock(&userPipeTableMutexLock);
    return false;
}
void DeleteUserPipeForUserPipeTable(int formerID, int latterID) {
    pthread_mutex_lock(&userPipeTableMutexLock);
    userPipeTableSharedMemoryPointer[(formerID - 1) * MAX_USER_COUNT + (latterID - 1)].isActive = false;
    pthread_mutex_unlock(&userPipeTableMutexLock);
}

void CleanUserPipeForUserPipeTable(int userID) {
    bool isHaveUserPipe = false;
    pid_t childProcessID;
    string userPipeFileName = "";
    int fd;
    int trashFileFD = -1;
    vector<string> command;
    command.push_back("ls");
    
    for (int i = 1; i <= MAX_USER_COUNT; i++) {
        for (int j = 1; j <= MAX_USER_COUNT; j++) {
            if ((i == userID) || (j == userID)) {
                if (i != j) {
                    isHaveUserPipe = false;
                    pthread_mutex_lock(&userTableMutexLock);
                    isHaveUserPipe = userPipeTableSharedMemoryPointer[(i - 1) * MAX_USER_COUNT + (j - 1)].isActive;
                    pthread_mutex_unlock(&userTableMutexLock);
                    if (isHaveUserPipe) {
                        childProcessID = fork();
                        if (childProcessID == 0) {
                            userPipeFileName = FIFO_DIRECTORY_NAME + to_string(i) + "_to_" + to_string(j);
                            fd = open(userPipeFileName.c_str(), O_RDONLY);
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                            trashFileFD = open("/dev/null", O_RDWR);
                            dup2(trashFileFD, STDOUT_FILENO);
                            close(trashFileFD);
                            ExecuteCommand(command);
                        }
                        waitpid(childProcessID, NULL, 0);
                        DeleteUserPipeForUserPipeTable(i, j);
                    }
                }
            }
        }
    }
}

/*---------- Class UserPipeCommand ----------*/

/*---------- Class NumberedPipeCommand ----------*/

/*---------- Class RemainingCommandCountData ----------*/

void AddRemainingCommandCountDataForRemainingCommandCountTable(UserCommand &userCommand, RemainingCommandCountData remainingCommandCountData) {
    userCommand.remainingCommandCountTable.push_back(remainingCommandCountData);
}

string GetLatterFIFOFileNameForRemainingCommandCountTable(UserCommand userCommand, int remainingCommandCount) {
    string latterFIFOFileName = "";
    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
        if (remainingCommandCount == userCommand.remainingCommandCountTable[i].remainingCommandCount) {
            latterFIFOFileName = userCommand.remainingCommandCountTable[i].latterFIFOFileName;
            break;
        }
    }
    return latterFIFOFileName;
}

int GetSortNumberForRemainingCommandCountTable(UserCommand userCommand, int remainingCommandCount) {
    int sortNumber = 1;
    bool isChangeSortNumber = false;
    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
        if (remainingCommandCount == userCommand.remainingCommandCountTable[i].remainingCommandCount) {
            if (sortNumber < userCommand.remainingCommandCountTable[i].sortNumber)
                sortNumber = userCommand.remainingCommandCountTable[i].sortNumber;
            isChangeSortNumber = true;
        }
    }
    if (isChangeSortNumber)
        sortNumber += 1;
    return sortNumber;
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

bool CheckDealWithConnectForRemainingCommandCountTable(UserCommand userCommand) {
    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
        if (userCommand.remainingCommandCountTable[i].remainingCommandCount == 0) {
            if (userCommand.remainingCommandCountTable[i].isConnect)
                return true;
        }
    }
    return false;
}

void DealWithConnectForRemainingCommandCountTable(UserCommand &userCommand) {   
    vector<RemainingCommandCountData> connectRemainingCommandCountTable;

    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
        if (userCommand.remainingCommandCountTable[i].remainingCommandCount == 0)
            connectRemainingCommandCountTable.push_back(userCommand.remainingCommandCountTable[i]);
    }

    int fd;
    char bufferC[99999];
    ssize_t bytesRead;
    string bufferStr = "";
    string totalBufferStr = "";
    for (int i = 0; i < connectRemainingCommandCountTable.size(); i++) {
        for (int j = 0; j < connectRemainingCommandCountTable.size(); j++) {
            if ((i + 1) == connectRemainingCommandCountTable[j].sortNumber) {
                fd = open(connectRemainingCommandCountTable[j].latterFIFOFileName.c_str(), O_RDONLY);
                while (true) {
                    bzero(bufferC, 99999);
                    bytesRead = read(fd, bufferC, sizeof(bufferC));
                    if (bytesRead <= 0)
                        break;
                    bufferStr = bufferC;
                    totalBufferStr += bufferStr;
                    bufferStr = "";
                }
                close(fd);
                break;
            }
        }
    }

    int fifoFileName = -1;
    pthread_mutex_lock(&fifoFileMutexLock);
    fifoFileName = fifoFileSharedMemoryPointer->name;
    fifoFileName += 1;
    fifoFileSharedMemoryPointer->name = fifoFileName;
    pthread_mutex_unlock(&fifoFileMutexLock);
    
    string latterFIFOFileName = FIFO_DIRECTORY_NAME + to_string(fifoFileName);
    mknod(latterFIFOFileName.c_str(), S_IFIFO | 0666, 0);
    
    pid_t childProcessID = fork();
    if (childProcessID == 0) {
        fd = open(latterFIFOFileName.c_str(), O_WRONLY);
        write(fd, totalBufferStr.c_str(), totalBufferStr.size());
        close(fd);
        exit(0);
    }
    
    for (int i = 0; i < userCommand.remainingCommandCountTable.size(); i++) {
        if (userCommand.remainingCommandCountTable[i].remainingCommandCount == 0)
            userCommand.remainingCommandCountTable[i].latterFIFOFileName = latterFIFOFileName;
    }
}

/*---------- Class UserCommand ----------*/

/*---------- Start ----------*/

void HandleServerSignal(int signal) {
    if (signal == SIGINT) {
        int fifoFileCount = -1;
        pthread_mutex_lock(&fifoFileMutexLock);
        fifoFileCount = fifoFileSharedMemoryPointer->name;
        pthread_mutex_unlock(&fifoFileMutexLock);

        cout << "Start to remove shared memory." << endl;
        shmdt(userTableSharedMemoryPointer);
        shmctl(userTableSharedMemoryID, IPC_RMID, NULL);
        shmdt(messageSharedMemoryPointer);
        shmctl(messageSharedMemoryID, IPC_RMID, NULL);
        shmdt(fifoFileSharedMemoryPointer);
        shmctl(fifoFileSharedMemoryID, IPC_RMID, NULL);
        shmdt(userPipeTableSharedMemoryPointer);
        shmctl(userPipeTableSharedMemoryID, IPC_RMID, NULL);
        cout << "Finish to remove shared memory." << endl << endl;

        cout << "Start to clean FIFO File." << endl;
        string fifoFileName = "";
        for (int i = 1; i <= 30; i++) {
            for (int j = 1; j <= 30; j++) {
                fifoFileName = FIFO_DIRECTORY_NAME + to_string(i) + "_to_" + to_string(j);
                unlink(fifoFileName.c_str());
            }
        }
        for (int i = 1; i <= fifoFileCount; i++) {
            fifoFileName = FIFO_DIRECTORY_NAME + to_string(i);
            unlink(fifoFileName.c_str());
        }
        cout << "Finish to clean FIFO File." << endl << endl;

        cout << "Start to close server listen sock." << endl;
        close(serverListenSock);
        cout << "Finish to close server listen sock." << endl << endl;

        cout << "Server is exiting..." << endl;
        exit(0);
        return;
    }
    if (signal == SIGCHLD) {
        while (true) {
            if (waitpid(-1, NULL, WNOHANG) <= 0)
                break;
        }
        return;
    }
}

void HandleUserSignal(int signal) {
    if (signal == SIGUSR1) {
        char messageC[MAX_MESSAGE_SIZE] = {0};
        pthread_mutex_lock(&messageMutexLock);
        strcpy(messageC, messageSharedMemoryPointer->content);
        pthread_mutex_unlock(&messageMutexLock);
        string messageStr(messageC);
        write(userSock, messageStr.c_str(), messageStr.length());
        return;
    }
    if (signal == SIGCHLD) {
        while (true) {
            if (waitpid(-1, NULL, WNOHANG) <= 0)
                break;
        }
        return;
    }
}

void InitializeMutexLock() {
    pthread_mutexattr_t userTableMutexLockAttribute;
    pthread_mutexattr_init(&userTableMutexLockAttribute);
    pthread_mutexattr_setpshared(&userTableMutexLockAttribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&userTableMutexLock, &userTableMutexLockAttribute);

    pthread_mutexattr_t messageMutexLockAttribute;
    pthread_mutexattr_init(&messageMutexLockAttribute);
    pthread_mutexattr_setpshared(&messageMutexLockAttribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&messageMutexLock, &messageMutexLockAttribute);

    pthread_mutexattr_t fifoFileMutexLockAttribute;
    pthread_mutexattr_init(&fifoFileMutexLockAttribute);
    pthread_mutexattr_setpshared(&fifoFileMutexLockAttribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&fifoFileMutexLock, &fifoFileMutexLockAttribute);

    pthread_mutexattr_t userPipeTableMutexLockAttribute;
    pthread_mutexattr_init(&userPipeTableMutexLockAttribute);
    pthread_mutexattr_setpshared(&userPipeTableMutexLockAttribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&userPipeTableMutexLock, &userPipeTableMutexLockAttribute);
}

void InitializeSharedMemory() {
    userTableSharedMemoryID = shmget(USER_TABLE_SHARED_MEMORY_KEY, sizeof(User) * MAX_USER_COUNT, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    userTableSharedMemoryPointer = (User *) shmat(userTableSharedMemoryID, NULL, 0);

    messageSharedMemoryID = shmget(MESSAGE_SHARED_MEMORY_KEY, sizeof(Message), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    messageSharedMemoryPointer = (Message *) shmat(messageSharedMemoryID, NULL, 0);

    fifoFileSharedMemoryID = shmget(FIFO_FILE_SHARED_MEMORY_KEY, sizeof(FIFOFile), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    fifoFileSharedMemoryPointer = (FIFOFile *) shmat(fifoFileSharedMemoryID, NULL, 0);

    userPipeTableSharedMemoryID = shmget(USER_PIPE_TABLE_SHARED_MEMORY_KEY, sizeof(UserPipe) * MAX_USER_COUNT * MAX_USER_COUNT, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    userPipeTableSharedMemoryPointer = (UserPipe *) shmat(userPipeTableSharedMemoryID, NULL, 0);
}

void InitializeFIFO() {
    string fifoFileName = "";
    for (int i = 1; i <= 30; i++) {
        for (int j = 1; j <= 30; j++) {
            fifoFileName = FIFO_DIRECTORY_NAME + to_string(i) + "_to_" + to_string(j);
            mknod(fifoFileName.c_str(), S_IFIFO | 0666, 0);
        }
    }
}

void EnableServerListenSocket(int serverListenPort) {
    serverListenSock = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(serverListenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serverLocation;
    serverLocation.sin_family = AF_INET;
    serverLocation.sin_addr.s_addr = INADDR_ANY;
    serverLocation.sin_port = htons(serverListenPort);
    bind(serverListenSock, (struct sockaddr *) &serverLocation, sizeof(serverLocation));

    listen(serverListenSock, 30);
}

void EnterUserEnv(sockaddr_in userLocation) {
    signal(SIGUSR1, HandleUserSignal);
    signal(SIGCHLD, HandleUserSignal);

    int userID = GetUserID();
    if (userID == -1) {
        close(userSock);
        exit(0);
    }
    string userPATHValue = GetUserPATHValue();
    AddUserForUserTable(userID, "(no name)", inet_ntoa(userLocation.sin_addr), ntohs(userLocation.sin_port), userPATHValue);
    UserCommand userCommand;
    
    ShowWelcomeMessage(userID);

    BroadcastLoginMessage(userID);

    while (true) {
        usleep(10000);
        ShowCommandLinePrompt();

        bool isChangeFileNameParameter = true;
        bool isChangeUserPipeCommandParameter = true;
        InitializeUserCommandSetting(userCommand, isChangeFileNameParameter, isChangeUserPipeCommandParameter);

        char buffer[MAX_SINGLE_LINE_INPUT_SIZE] = {0};
        int bufferSize = read(userSock, buffer, MAX_SINGLE_LINE_INPUT_SIZE);

        // If user close connection, remove the user from user table
        if (bufferSize == 0) {
            BroadcastLogoutMessage(userID);
            close(userSock);
            DeleteUserForUserTable(userID);
            CleanUserPipeForUserPipeTable(userID);
            exit(0);
        }

        string singleLineInput(buffer);

        singleLineInput = RemoveLineFeedAndCarriageReturn(singleLineInput);

        if (singleLineInput.empty())
            continue;

        // If user click Ctrl C or D, remove the user from user table
        if (CheckClickCtrlC(singleLineInput) || CheckClickCtrlD(singleLineInput)) {
            BroadcastLogoutMessage(userID);
            close(userSock);
            DeleteUserForUserTable(userID);
            CleanUserPipeForUserPipeTable(userID);
            exit(0);
        }

        vector<string> wordListSingleLineInput = SplitSingleLineInput(singleLineInput);

        // If user input exit, remove the user from user table
        if (wordListSingleLineInput[0] == "exit") {
            BroadcastLogoutMessage(userID);
            close(userSock);
            DeleteUserForUserTable(userID);
            CleanUserPipeForUserPipeTable(userID);
            exit(0);
        }

        if (wordListSingleLineInput[0] == "printenv") {
            if (userCommand.remainingCommandCountTable.size() > 0)
                UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommand);
            PrintEnv(wordListSingleLineInput[1], userID);
            continue;
        }

        if (wordListSingleLineInput[0] == "setenv") {
            if (userCommand.remainingCommandCountTable.size() > 0)
                UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommand);
            SetEnv(wordListSingleLineInput[1], wordListSingleLineInput[2]);
            if (wordListSingleLineInput[1] == "PATH") {
                UpdateUserPATHValueForUserTable(userID, wordListSingleLineInput[2]);
            }
            continue;
        }

        if (wordListSingleLineInput[0] == "who") {
            if (userCommand.remainingCommandCountTable.size() > 0)
                UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommand);
            ExecuteWhoCommand(userID);
            continue;
        }

        if (wordListSingleLineInput[0] == "tell") {
            if (userCommand.remainingCommandCountTable.size() > 0)
                UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommand);
            ExecuteTellCommand(userID, stoi(wordListSingleLineInput[1]), wordListSingleLineInput);
            continue;
        }

        if (wordListSingleLineInput[0] == "yell") {
            if (userCommand.remainingCommandCountTable.size() > 0)
                UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommand);
            ExecuteYellCommand(userID, wordListSingleLineInput);
            continue;
        }

        if (wordListSingleLineInput[0] == "name") {
            if (userCommand.remainingCommandCountTable.size() > 0)
                UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommand);
            ExecuteNameCommand(userID, wordListSingleLineInput[1]);
            continue;
        }

        int fifoFileName = -1;
        string userPipeFileName = "";
        pid_t childProcessID;
        int fd;
        int senderID = -1;
        int mediatorID = -1;
        int receiverID = -1;
        vector<string> displayCommand = wordListSingleLineInput;
        UserPipeMessage userPipeMessage;

        if (CheckFileRedirectionSymbol(wordListSingleLineInput)) {
            GetFileName(userCommand, wordListSingleLineInput);
            RemoveFileRedirectionSymbol(wordListSingleLineInput);
        }

        if (CheckUserPipeCommand('<', wordListSingleLineInput)) {
            userCommand.userPipeCommand.isForSender = true;

            for (int j = 0; j < wordListSingleLineInput.size(); j++) {
                if ((wordListSingleLineInput[j][0] == '<') && (wordListSingleLineInput[j].size() > 1)) {
                    senderID = stoi(wordListSingleLineInput[j].substr(1));
                    break;
                }
            }
            mediatorID = userID;

            RemoveUserPipeSymbol('<', wordListSingleLineInput);
            
            if (!CheckUserExistForUserTable(senderID)) {
                userPipeMessage.userPipeMessageContentForSender.userErrorMessage =  ShowUserErrorMessage(senderID);
                userPipeMessage.userPipeMessageTypeForSender.isUserErrorMessage = true;
                userCommand.userPipeCommand.happenError.isForSender = true;
            }

            if (!userCommand.userPipeCommand.happenError.isForSender) {
                if (!CheckUserPipeExistForUserPipeTable(senderID, mediatorID)) {
                    ShowUserPipeErrorMessageForSender(senderID, mediatorID, userPipeMessage);
                    userCommand.userPipeCommand.happenError.isForSender = true;
                }
            }

            if (!userCommand.userPipeCommand.happenError.isForSender) {
                BroadcastUserPipeMessageForSender(senderID, mediatorID, displayCommand, userPipeMessage);
                userPipeMessage.userPipeMessageTypeForSender.isUserPipeSuccessMessage = true;
            }
        }

        if (CheckUserPipeCommand('>', wordListSingleLineInput)) {
            userCommand.userPipeCommand.isForReceiver = true;

            if (!userCommand.userPipeCommand.isForSender)
                mediatorID = userID;
            for (int j = 0; j < wordListSingleLineInput.size(); j++) {
                if ((wordListSingleLineInput[j][0] == '>') && (wordListSingleLineInput[j].size() > 1)) {
                    receiverID = stoi(wordListSingleLineInput[j].substr(1));
                    break;
                }
            }

            RemoveUserPipeSymbol('>', wordListSingleLineInput);
            
            if (!CheckUserExistForUserTable(receiverID)) {
                userPipeMessage.userPipeMessageContentForReceiver.userErrorMessage = ShowUserErrorMessage(receiverID);
                userPipeMessage.userPipeMessageTypeForReceiver.isUserErrorMessage = true;
                userCommand.userPipeCommand.happenError.isForReceiver = true;
            }

            if (!userCommand.userPipeCommand.happenError.isForReceiver) {
                if (CheckUserPipeExistForUserPipeTable(mediatorID, receiverID)) {
                    ShowUserPipeErrorMessageForReceiver(mediatorID, receiverID, userPipeMessage);
                    userCommand.userPipeCommand.happenError.isForReceiver = true;
                }
            }

            if (!userCommand.userPipeCommand.happenError.isForReceiver)
                BroadcastUserPipeMessageForReceiver(mediatorID, receiverID, displayCommand, userPipeMessage);
        }

        if (userCommand.userPipeCommand.isForSender || userCommand.userPipeCommand.isForReceiver)
            ShowUserPipeMessage(userPipeMessage, userID);

        vector<vector<string>> commandList = BuildCommandList(wordListSingleLineInput);
        for (int i = 0; i < commandList.size(); i++) {
            RemainingCommandCountData remainingCommandCountData;

            bool isChangeFileNameParameter = false;
            bool isChangeUserPipeCommandParameter = false;
            InitializeUserCommandSetting(userCommand, isChangeFileNameParameter, isChangeUserPipeCommandParameter);

            if (userCommand.remainingCommandCountTable.size() > 0)
                UpdateRemainingCommandCountDataListForRemainingCommandCountTable(userCommand);

            if (CheckNumberedPipeCommand('|', commandList[i])) {
                userCommand.numberedPipeCommand.isFirstType = true;
                remainingCommandCountData.remainingCommandCount = GetRemainingCommandCountFromNumberedPipeCommand(commandList[i]);
                RemoveNumberedPipeSymbol(commandList[i]);
            }
            if (CheckNumberedPipeCommand('!', commandList[i])) {
                userCommand.numberedPipeCommand.isSecondType = true;
                remainingCommandCountData.remainingCommandCount = GetRemainingCommandCountFromNumberedPipeCommand(commandList[i]);
                RemoveNumberedPipeSymbol(commandList[i]);
            }
            if (!CheckOrdinaryPipeCommand(commandList[i])) {
                userCommand.isOrdinaryCommand = true;
                userCommand.commandList.push_back(commandList[i]);
            }
            if (CheckOrdinaryPipeCommand(commandList[i])) {
                userCommand.isOrdinaryPipeCommand = true;
                userCommand.commandList = SplitOrdinaryPipeCommand(commandList[i]);
            }
            if (CheckReceivePipe(userCommand))
                userCommand.needReceivePipe = true;
            if (CheckCreatePipe(userCommand, remainingCommandCountData.remainingCommandCount))
                userCommand.needCreatePipe = true;
            if (CheckConnectPipe(userCommand, remainingCommandCountData.remainingCommandCount)) {
                userCommand.needConnectPipe = true;
                remainingCommandCountData.isConnect = true;
                remainingCommandCountData.sortNumber = GetSortNumberForRemainingCommandCountTable(userCommand, remainingCommandCountData.remainingCommandCount);
            }

            if (CheckDealWithConnectForRemainingCommandCountTable(userCommand))
                DealWithConnectForRemainingCommandCountTable(userCommand);

            if (userCommand.isOrdinaryCommand) {
                if (userCommand.needReceivePipe)
                    remainingCommandCountData.formerFIFOFileName = GetLatterFIFOFileNameForRemainingCommandCountTable(userCommand, 0);
                if (!userCommand.userPipeCommand.isForReceiver) {
                    if (userCommand.needCreatePipe) {
                        pthread_mutex_lock(&fifoFileMutexLock);
                        fifoFileName = fifoFileSharedMemoryPointer->name;
                        fifoFileName += 1;
                        fifoFileSharedMemoryPointer->name = fifoFileName;
                        pthread_mutex_unlock(&fifoFileMutexLock);
                        
                        remainingCommandCountData.latterFIFOFileName = FIFO_DIRECTORY_NAME + to_string(fifoFileName);
                        mknod(remainingCommandCountData.latterFIFOFileName.c_str(), S_IFIFO | 0666, 0);
                    }
                }
                if (userCommand.needConnectPipe) {
                    pthread_mutex_lock(&fifoFileMutexLock);
                    fifoFileName = fifoFileSharedMemoryPointer->name;
                    fifoFileName += 1;
                    fifoFileSharedMemoryPointer->name = fifoFileName;
                    pthread_mutex_unlock(&fifoFileMutexLock);
                    
                    remainingCommandCountData.latterFIFOFileName = FIFO_DIRECTORY_NAME + to_string(fifoFileName);
                    mknod(remainingCommandCountData.latterFIFOFileName.c_str(), S_IFIFO | 0666, 0);
                }
                childProcessID = fork();
                if (childProcessID == 0) {
                    if (userCommand.userPipeCommand.isForSender && (!userCommand.userPipeCommand.happenError.isForSender)) {
                        if ((!userCommand.needReceivePipe) && (!userCommand.needConnectPipe)) {
                            userPipeFileName = FIFO_DIRECTORY_NAME + to_string(senderID) + "_to_" + to_string(mediatorID);
                            fd = open(userPipeFileName.c_str(), O_RDONLY);
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                        }
                    }
                    if (userCommand.needReceivePipe) {
                        fd = open(remainingCommandCountData.formerFIFOFileName.c_str(), O_RDONLY);
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                    }
                    if (!userCommand.userPipeCommand.isForReceiver) {
                        if (userCommand.needCreatePipe || userCommand.needConnectPipe) {
                            fd = open(remainingCommandCountData.latterFIFOFileName.c_str(), O_WRONLY);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        }
                    }
                    if (userCommand.userPipeCommand.isForReceiver && (!userCommand.userPipeCommand.happenError.isForReceiver)) {
                        userPipeFileName = FIFO_DIRECTORY_NAME + to_string(mediatorID) + "_to_" + to_string(receiverID);
                        fd = open(userPipeFileName.c_str(), O_WRONLY);
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }
                    if (!userCommand.userPipeCommand.isForReceiver) {
                        if ((!userCommand.needCreatePipe) && (!userCommand.needConnectPipe)) {
                            if (userCommand.fileName == "")
                                dup2(userSock, STDOUT_FILENO);
                            if (userCommand.fileName != "")
                                RedirectFile(userCommand.fileName);
                        }
                    }
                    if (userCommand.userPipeCommand.happenError.isForSender || userCommand.userPipeCommand.happenError.isForReceiver) {
                        int trashFileFD = open("/dev/null", O_RDWR);
                        if (userCommand.userPipeCommand.happenError.isForSender)
                            dup2(trashFileFD, STDIN_FILENO);
                        if (userCommand.userPipeCommand.happenError.isForReceiver)
                            dup2(trashFileFD, STDOUT_FILENO);
                        close(trashFileFD);
                    }
                    if (!userCommand.numberedPipeCommand.isSecondType)
                        dup2(userSock, STDERR_FILENO);
                    if (userCommand.numberedPipeCommand.isSecondType) {
                        fd = open(remainingCommandCountData.latterFIFOFileName.c_str(), O_WRONLY);
                        dup2(fd, STDERR_FILENO);
                        close(fd);
                    }
                    ExecuteCommand(userCommand.commandList[0]);
                }
                if (!userCommand.userPipeCommand.isForReceiver) {
                    if ((!userCommand.needCreatePipe) && (!userCommand.needConnectPipe))
                        waitpid(childProcessID, NULL, 0);
                }
                if (userCommand.userPipeCommand.happenError.isForSender || userCommand.userPipeCommand.happenError.isForReceiver)
                    waitpid(childProcessID, NULL, 0);
                if (!userCommand.userPipeCommand.isForReceiver) {
                    if (userCommand.needCreatePipe || userCommand.needConnectPipe)
                        AddRemainingCommandCountDataForRemainingCommandCountTable(userCommand, remainingCommandCountData);
                }
                if (userCommand.userPipeCommand.isForSender && (!userCommand.userPipeCommand.happenError.isForSender)) {
                    if ((!userCommand.needCreatePipe) && (!userCommand.needConnectPipe))
                        DeleteUserPipeForUserPipeTable(senderID, mediatorID);
                }
                if (userCommand.userPipeCommand.isForReceiver && (!userCommand.userPipeCommand.happenError.isForReceiver))
                    AddUserPipeForUserPipeTable(mediatorID, receiverID);                    
            }

            string formerFIFOFileName = "";
            string latterFIFOFileName = "";
            if (userCommand.isOrdinaryPipeCommand) {
                for (int k = 0; k < userCommand.commandList.size(); k++) {
                    if (k == 0) {
                        if (userCommand.needReceivePipe)
                            remainingCommandCountData.formerFIFOFileName = GetLatterFIFOFileNameForRemainingCommandCountTable(userCommand, 0);
                    }
                    if (k > 0)
                        formerFIFOFileName = latterFIFOFileName;
                    if (k < (userCommand.commandList.size() - 1)) {
                        pthread_mutex_lock(&fifoFileMutexLock);
                        fifoFileName = fifoFileSharedMemoryPointer->name;
                        fifoFileName += 1;
                        fifoFileSharedMemoryPointer->name = fifoFileName;
                        pthread_mutex_unlock(&fifoFileMutexLock);
                        
                        latterFIFOFileName = FIFO_DIRECTORY_NAME + to_string(fifoFileName);
                        mknod(latterFIFOFileName.c_str(), S_IFIFO | 0666, 0);
                    }
                    if (k == (userCommand.commandList.size() - 1)) {
                        if (!userCommand.userPipeCommand.isForReceiver) {
                            if (userCommand.needCreatePipe) {
                                pthread_mutex_lock(&fifoFileMutexLock);
                                fifoFileName = fifoFileSharedMemoryPointer->name;
                                fifoFileName += 1;
                                fifoFileSharedMemoryPointer->name = fifoFileName;
                                pthread_mutex_unlock(&fifoFileMutexLock);
                                
                                remainingCommandCountData.latterFIFOFileName = FIFO_DIRECTORY_NAME + to_string(fifoFileName);
                                mknod(remainingCommandCountData.latterFIFOFileName.c_str(), S_IFIFO | 0666, 0);
                            }
                        }
                        if (userCommand.needConnectPipe) {
                            pthread_mutex_lock(&fifoFileMutexLock);
                            fifoFileName = fifoFileSharedMemoryPointer->name;
                            fifoFileName += 1;
                            fifoFileSharedMemoryPointer->name = fifoFileName;
                            pthread_mutex_unlock(&fifoFileMutexLock);
                            
                            remainingCommandCountData.latterFIFOFileName = FIFO_DIRECTORY_NAME + to_string(fifoFileName);
                            mknod(remainingCommandCountData.latterFIFOFileName.c_str(), S_IFIFO | 0666, 0);
                        }
                    }
                    childProcessID = fork();
                    if (childProcessID == 0) {
                        if (k == 0) {
                            if (userCommand.userPipeCommand.isForSender && (!userCommand.userPipeCommand.happenError.isForSender)) {
                                if ((!userCommand.needReceivePipe) && (!userCommand.needConnectPipe)) {
                                    userPipeFileName = FIFO_DIRECTORY_NAME + to_string(senderID) + "_to_" + to_string(mediatorID);
                                    fd = open(userPipeFileName.c_str(), O_RDONLY);
                                    dup2(fd, STDIN_FILENO);
                                    close(fd);
                                }
                            }
                            if (userCommand.needReceivePipe) {
                                fd = open(remainingCommandCountData.formerFIFOFileName.c_str(), O_RDONLY);
                                dup2(fd, STDIN_FILENO);
                                close(fd);
                            }
                        }
                        if (k > 0) {
                            fd = open(formerFIFOFileName.c_str(), O_RDONLY);
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                        }
                        if (k < (userCommand.commandList.size() - 1)) {
                            fd = open(latterFIFOFileName.c_str(), O_WRONLY);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        }
                        if (k == (userCommand.commandList.size() - 1)) {
                            if (!userCommand.userPipeCommand.isForReceiver) {
                                if (userCommand.needCreatePipe || userCommand.needConnectPipe) {
                                    fd = open(remainingCommandCountData.latterFIFOFileName.c_str(), O_WRONLY);
                                    dup2(fd, STDOUT_FILENO);
                                    close(fd);
                                }
                            }
                            if (userCommand.userPipeCommand.isForReceiver && (!userCommand.userPipeCommand.happenError.isForReceiver)) {
                                userPipeFileName = FIFO_DIRECTORY_NAME + to_string(mediatorID) + "_to_" + to_string(receiverID);
                                fd = open(userPipeFileName.c_str(), O_WRONLY);
                                dup2(fd, STDOUT_FILENO);
                                close(fd);
                            }
                            if (!userCommand.userPipeCommand.isForReceiver) {
                                if ((!userCommand.needCreatePipe) && (!userCommand.needConnectPipe)) {
                                    if (userCommand.fileName == "")
                                        dup2(userSock, STDOUT_FILENO);
                                    if (userCommand.fileName != "")
                                        RedirectFile(userCommand.fileName);
                                }
                            }
                            if (userCommand.userPipeCommand.happenError.isForSender || userCommand.userPipeCommand.happenError.isForReceiver) {
                                int trashFileFD = open("/dev/null", O_RDWR);
                                if (userCommand.userPipeCommand.happenError.isForSender)
                                    dup2(trashFileFD, STDIN_FILENO);
                                if (userCommand.userPipeCommand.happenError.isForReceiver)
                                    dup2(trashFileFD, STDOUT_FILENO);
                                close(trashFileFD);
                            }
                        }
                        if (k < (userCommand.commandList.size() - 1)) {
                            if (!userCommand.numberedPipeCommand.isSecondType)
                                dup2(userSock, STDERR_FILENO);
                            if (userCommand.numberedPipeCommand.isSecondType) {
                                fd = open(latterFIFOFileName.c_str(), O_WRONLY);
                                dup2(fd, STDERR_FILENO);
                                close(fd);
                            }
                        }
                        if (k == (userCommand.commandList.size() - 1)) {
                            if (!userCommand.numberedPipeCommand.isSecondType)
                                dup2(userSock, STDERR_FILENO);
                            if (userCommand.numberedPipeCommand.isSecondType) {
                                fd = open(remainingCommandCountData.latterFIFOFileName.c_str(), O_WRONLY);
                                dup2(fd, STDERR_FILENO);
                                close(fd);
                            }
                        }
                        ExecuteCommand(userCommand.commandList[k]);
                    }
                }
                if (!userCommand.userPipeCommand.isForReceiver) {
                    if ((!userCommand.needCreatePipe) && (!userCommand.needConnectPipe))
                        waitpid(childProcessID, NULL, 0);
                }
                if (userCommand.userPipeCommand.happenError.isForSender || userCommand.userPipeCommand.happenError.isForReceiver)
                    waitpid(childProcessID, NULL, 0);
                if (!userCommand.userPipeCommand.isForReceiver) {
                    if (userCommand.needCreatePipe || userCommand.needConnectPipe)
                        AddRemainingCommandCountDataForRemainingCommandCountTable(userCommand, remainingCommandCountData);
                }
                if (userCommand.userPipeCommand.isForSender && (!userCommand.userPipeCommand.happenError.isForSender)) {
                    if ((!userCommand.needCreatePipe) && (!userCommand.needConnectPipe))
                        DeleteUserPipeForUserPipeTable(senderID, mediatorID);
                }
                if (userCommand.userPipeCommand.isForReceiver && (!userCommand.userPipeCommand.happenError.isForReceiver))
                    AddUserPipeForUserPipeTable(mediatorID, receiverID);
            }

            DeleteRemainingCommandCountDataListForRemainingCommandCountTable(userCommand);
        }
    }
}

int GetUserID() {
    int userID = -1;
    pthread_mutex_lock(&userTableMutexLock);
    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (!userTableSharedMemoryPointer[i].isActive) {
            userID = i + 1;
            break;
        }
    }
    pthread_mutex_unlock(&userTableMutexLock);
    return userID;
}

string GetUserPATHValue() {
    const char* userPATHValueC = getenv("PATH");
    string userPATHValueStr = userPATHValueC;
    return userPATHValueStr;
}

void ShowCommandLinePrompt() {
    string commandLinePromptMessage = "% ";
    write(userSock, commandLinePromptMessage.c_str(), commandLinePromptMessage.length());
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

void PrintEnv(string envVariable, int userID) {
    const char* envVariableValue = getenv(envVariable.c_str());
    ostringstream oss;
    oss << envVariableValue << endl;
    string envVariableValueMessage = oss.str();

    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, envVariableValueMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    if (userTableSharedMemoryPointer[userID - 1].isActive)
        kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
}

void SetEnv(string envVariable, string envVariableValue) {
    setenv(envVariable.c_str(), envVariableValue.c_str(), 1);
}

void ShowWelcomeMessage(int userID) {
    ostringstream oss;
    oss << "****************************************" << endl;
    oss << "** Welcome to the information server. **" << endl;
    oss << "****************************************" << endl;
    string welcomeMessage = oss.str();

    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, welcomeMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    if (userTableSharedMemoryPointer[userID - 1].isActive)
        kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
}

void BroadcastLoginMessage(int userID) {
    ostringstream oss;

    pthread_mutex_lock(&userTableMutexLock);
    if (userTableSharedMemoryPointer[userID - 1].isActive)
        oss << "*** User '" << userTableSharedMemoryPointer[userID - 1].nickname << "' entered from " << userTableSharedMemoryPointer[userID - 1].IP << ":" << userTableSharedMemoryPointer[userID - 1].port << ". ***" << endl;
    pthread_mutex_unlock(&userTableMutexLock);

    string loginMessage = oss.str();

    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, loginMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (userTableSharedMemoryPointer[i].isActive)
            kill(userTableSharedMemoryPointer[i].processID, SIGUSR1);
    }
}

void BroadcastLogoutMessage(int userID) {
    ostringstream oss;

    pthread_mutex_lock(&userTableMutexLock);
    if (userTableSharedMemoryPointer[userID - 1].isActive)
        oss << "*** User '" << userTableSharedMemoryPointer[userID - 1].nickname << "' left. ***" << endl;
    pthread_mutex_unlock(&userTableMutexLock);
    
    string logoutMessage = oss.str();

    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, logoutMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (userTableSharedMemoryPointer[i].isActive) {
            if (userID != userTableSharedMemoryPointer[i].ID)
                kill(userTableSharedMemoryPointer[i].processID, SIGUSR1);
        }
            
    }
}

void ExecuteWhoCommand(int userID) {
    ostringstream oss;
    oss << "<ID>\t<nickname>\t<IP:port>\t<indicate me>" << endl;

    pthread_mutex_lock(&userTableMutexLock);
    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (userTableSharedMemoryPointer[i].isActive) {
            oss << userTableSharedMemoryPointer[i].ID << "\t" << userTableSharedMemoryPointer[i].nickname << "\t" << userTableSharedMemoryPointer[i].IP << ":" << userTableSharedMemoryPointer[i].port;
            if (userID == userTableSharedMemoryPointer[i].ID)
                oss << "\t<-me";
            oss << endl;
        }
    }
    pthread_mutex_unlock(&userTableMutexLock);

    string whoMessage = oss.str();

    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, whoMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    if (userTableSharedMemoryPointer[userID - 1].isActive)
        kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
}

void ExecuteTellCommand(int senderID, int receiverID, vector<string> message) {    
    bool isHappenError = false;

    pthread_mutex_lock(&userTableMutexLock);
    if (!userTableSharedMemoryPointer[receiverID - 1].isActive)
        isHappenError = true;
    pthread_mutex_unlock(&userTableMutexLock);

    ostringstream oss;
    string tellMessage = "";
    if (isHappenError) {
        oss << "*** Error: user #" << receiverID << " does not exist yet. ***" << endl;
        tellMessage = oss.str();

        pthread_mutex_lock(&messageMutexLock);
        strcpy(messageSharedMemoryPointer->content, tellMessage.c_str());
        pthread_mutex_unlock(&messageMutexLock);

        if (userTableSharedMemoryPointer[senderID - 1].isActive)
            kill(userTableSharedMemoryPointer[senderID - 1].processID, SIGUSR1);
        return;
    }

    pthread_mutex_lock(&userTableMutexLock);
    if (userTableSharedMemoryPointer[senderID - 1].isActive)
        oss << "*** " << userTableSharedMemoryPointer[senderID - 1].nickname << " told you ***: ";
    pthread_mutex_unlock(&userTableMutexLock);

    for (int i = 2; i < message.size(); i++) {
        oss << message[i];
        if (i < (message.size() - 1))
            oss << " ";
    }
    oss << endl;
    tellMessage = oss.str();

    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, tellMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    if (userTableSharedMemoryPointer[receiverID - 1].isActive)
        kill(userTableSharedMemoryPointer[receiverID - 1].processID, SIGUSR1);
}

void ExecuteYellCommand(int userID, vector<string> message) {
    ostringstream oss;

    pthread_mutex_lock(&userTableMutexLock);
    if (userTableSharedMemoryPointer[userID - 1].isActive)
        oss << "*** " << userTableSharedMemoryPointer[userID - 1].nickname << " yelled ***: ";
    pthread_mutex_unlock(&userTableMutexLock);

    for (int i = 1; i < message.size(); i++) {
        oss << message[i];
        if (i < (message.size() - 1))
            oss << " ";
    }
    oss << endl;
    string yellMessage = oss.str();

    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, yellMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (userTableSharedMemoryPointer[i].isActive)
            kill(userTableSharedMemoryPointer[i].processID, SIGUSR1);
    }
}

void ExecuteNameCommand(int userID, string userNickname) {
    bool isHappenError = false;
    ostringstream oss;
    
    pthread_mutex_lock(&userTableMutexLock);
    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (userTableSharedMemoryPointer[i].isActive) {
            if (strcmp(userTableSharedMemoryPointer[i].nickname, userNickname.c_str()) == 0) {
                isHappenError = true;
                oss << "*** User '" << userTableSharedMemoryPointer[i].nickname << "' already exists. ***" << endl;
                break;
            }
        }
    }
    pthread_mutex_unlock(&userTableMutexLock);
    
    string nameMessage = "";
    if (isHappenError) {
        nameMessage = oss.str();

        pthread_mutex_lock(&messageMutexLock);
        strcpy(messageSharedMemoryPointer->content, nameMessage.c_str());
        pthread_mutex_unlock(&messageMutexLock);

        if (userTableSharedMemoryPointer[userID - 1].isActive)
            kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
        return;
    }

    UpdateUserNicknameForUserTable(userID, userNickname);

    pthread_mutex_lock(&userTableMutexLock);
    oss << "*** User from " << userTableSharedMemoryPointer[userID - 1].IP << ":" << userTableSharedMemoryPointer[userID - 1].port << " is named '" << userTableSharedMemoryPointer[userID - 1].nickname <<"'. ***" << endl;
    pthread_mutex_unlock(&userTableMutexLock);

    nameMessage = oss.str();

    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, nameMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (userTableSharedMemoryPointer[i].isActive)
            kill(userTableSharedMemoryPointer[i].processID, SIGUSR1);
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

string ShowUserErrorMessage(int errorUserID) {
    ostringstream oss;
    oss << "*** Error: user #" << errorUserID << " does not exist yet. ***" << endl;
    string userPipeErrorMessage = oss.str();
    return userPipeErrorMessage;
    /*
    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, userPipeErrorMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    if (userTableSharedMemoryPointer[userID - 1].isActive)
        kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
    */
}

void ShowUserPipeErrorMessageForSender(int senderID, int mediatorID, UserPipeMessage &userPipeMessage) {
    ostringstream oss;
    oss << "*** Error: the pipe #" << senderID << "->#" << mediatorID << " does not exist yet. ***" << endl;
    string userPipeErrorMessage = oss.str();

    userPipeMessage.userPipeMessageTypeForSender.isUserPipeErrorMessage = true;
    userPipeMessage.userPipeMessageContentForSender.userPipeErrorMessage = userPipeErrorMessage;
    /*
    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, userPipeErrorMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    if (userTableSharedMemoryPointer[mediatorID - 1].isActive)
        kill(userTableSharedMemoryPointer[mediatorID - 1].processID, SIGUSR1);
    */
}

void BroadcastUserPipeMessageForSender(int senderID, int mediatorID, vector<string> displayCommand, UserPipeMessage &userPipeMessage) {
    ostringstream oss;

    pthread_mutex_lock(&userTableMutexLock);
    oss << "*** " << userTableSharedMemoryPointer[mediatorID - 1].nickname << " (#" << userTableSharedMemoryPointer[mediatorID - 1].ID << ") just received from " << userTableSharedMemoryPointer[senderID - 1].nickname << " (#" << userTableSharedMemoryPointer[senderID - 1].ID << ") by '";
    pthread_mutex_unlock(&userTableMutexLock);
    
    for (int i = 0; i < displayCommand.size(); i++) {
        oss << displayCommand[i];
        if (i < (displayCommand.size() - 1))
            oss << " ";
    }
    oss << "' ***" << endl;
    userPipeMessage.userPipeSuccessMessage = oss.str();
    userPipeMessage.userPipeMessageTypeForSender.isUserPipeSuccessMessage = true;
    /*
    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, userPipeMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (userTableSharedMemoryPointer[i].isActive)
            kill(userTableSharedMemoryPointer[i].processID, SIGUSR1);
    }
    */
}

void ShowUserPipeErrorMessageForReceiver(int mediatorID, int receiverID, UserPipeMessage &userPipeMessage) {
    ostringstream oss;
    oss << "*** Error: the pipe #" << mediatorID << "->#" << receiverID << " already exists. ***" << endl;
    userPipeMessage.userPipeMessageContentForReceiver.userPipeErrorMessage = oss.str();
    userPipeMessage.userPipeMessageTypeForReceiver.isUserPipeErrorMessage = true;
    /*
    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, userPipeErrorMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    if (userTableSharedMemoryPointer[mediatorID - 1].isActive)
        kill(userTableSharedMemoryPointer[mediatorID - 1].processID, SIGUSR1);
    */
}

void BroadcastUserPipeMessageForReceiver(int mediatorID, int receiverID, vector<string> displayCommand, UserPipeMessage &userPipeMessage) {
    ostringstream oss;

    pthread_mutex_lock(&userTableMutexLock);
    oss << "*** " << userTableSharedMemoryPointer[mediatorID - 1].nickname << " (#" << userTableSharedMemoryPointer[mediatorID - 1].ID << ") just piped '";
    pthread_mutex_unlock(&userTableMutexLock);

    for (int i = 0; i < displayCommand.size(); i++) {
        oss << displayCommand[i];
        if (i < (displayCommand.size() - 1))
            oss << " ";
    }
    
    pthread_mutex_lock(&userTableMutexLock);
    oss << "' to " << userTableSharedMemoryPointer[receiverID - 1].nickname << " (#" << userTableSharedMemoryPointer[receiverID - 1].ID << ") ***" << endl;
    pthread_mutex_unlock(&userTableMutexLock);

    userPipeMessage.userPipeSuccessMessage += oss.str();
    userPipeMessage.userPipeMessageTypeForReceiver.isUserPipeSuccessMessage = true;
    /*
    pthread_mutex_lock(&messageMutexLock);
    strcpy(messageSharedMemoryPointer->content, userPipeMessage.c_str());
    pthread_mutex_unlock(&messageMutexLock);

    for (int i = 0; i < MAX_USER_COUNT; i++) {
        if (userTableSharedMemoryPointer[i].isActive)
            kill(userTableSharedMemoryPointer[i].processID, SIGUSR1);
    }
    */
}

void ShowUserPipeMessage(UserPipeMessage userPipeMessage, int userID) {
    if (userPipeMessage.userPipeMessageTypeForSender.isUserErrorMessage) {
        pthread_mutex_lock(&messageMutexLock);
        strcpy(messageSharedMemoryPointer->content, userPipeMessage.userPipeMessageContentForSender.userErrorMessage.c_str());
        pthread_mutex_unlock(&messageMutexLock);

        if (userTableSharedMemoryPointer[userID - 1].isActive)
            kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
    }

    if (userPipeMessage.userPipeMessageTypeForSender.isUserPipeErrorMessage) {
        pthread_mutex_lock(&messageMutexLock);
        strcpy(messageSharedMemoryPointer->content, userPipeMessage.userPipeMessageContentForSender.userPipeErrorMessage.c_str());
        pthread_mutex_unlock(&messageMutexLock);

        if (userTableSharedMemoryPointer[userID - 1].isActive)
            kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
    }

    if (userPipeMessage.userPipeMessageTypeForSender.isUserPipeSuccessMessage || userPipeMessage.userPipeMessageTypeForReceiver.isUserPipeSuccessMessage) {
        pthread_mutex_lock(&messageMutexLock);
        strcpy(messageSharedMemoryPointer->content, userPipeMessage.userPipeSuccessMessage.c_str());
        pthread_mutex_unlock(&messageMutexLock);

        for (int i = 0; i < MAX_USER_COUNT; i++) {
            if (userTableSharedMemoryPointer[i].isActive)
                kill(userTableSharedMemoryPointer[i].processID, SIGUSR1);
        }
    }

    if (userPipeMessage.userPipeMessageTypeForReceiver.isUserErrorMessage) {
        pthread_mutex_lock(&messageMutexLock);
        strcpy(messageSharedMemoryPointer->content, userPipeMessage.userPipeMessageContentForReceiver.userErrorMessage.c_str());
        pthread_mutex_unlock(&messageMutexLock);

        if (userTableSharedMemoryPointer[userID - 1].isActive)
            kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
    }

    if (userPipeMessage.userPipeMessageTypeForReceiver.isUserPipeErrorMessage) {
        pthread_mutex_lock(&messageMutexLock);
        strcpy(messageSharedMemoryPointer->content, userPipeMessage.userPipeMessageContentForReceiver.userPipeErrorMessage.c_str());
        pthread_mutex_unlock(&messageMutexLock);

        if (userTableSharedMemoryPointer[userID - 1].isActive)
            kill(userTableSharedMemoryPointer[userID - 1].processID, SIGUSR1);
    }
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