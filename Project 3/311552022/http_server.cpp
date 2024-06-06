#include <sys/wait.h>
#include <boost/asio.hpp>
#include <iostream>

#define MAX_RECEIVE_BUFFER_SIZE 99999

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

io_context httpServerIoContext;

class ClientEnvVariable {
    public:
        string requestMethod = "";
        string requestUri = "";
        string queryString = "";
        string serverProtocol = "";
        string httpHost = "";
        string serverAddr = "";
        string serverPort = "";
        string remoteAddr = "";
        string remotePort = "";
};

class ClientSession : public enable_shared_from_this<ClientSession> {
    public:
        ClientSession(tcp::socket clientSockForHttpServerClass) : clientSockForClientSessionClass(move(clientSockForHttpServerClass)) {

        }

        void Start() {
            ReadMessageFromClient();
        }

    private:
        tcp::socket clientSockForClientSessionClass;
        char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE] = {0};
        string clientMessage = "";
        int clientMessageLength = -1;
        pid_t childProcessID;
        int clientSock = -1;
        int spaceCount = 3;
        int crCount = 1;
        int start = 0;
        int end = 0;
        ClientEnvVariable clientEnvVariable;
        int requestUriLength = -1;
        string exeFileName = "";

        void ReadMessageFromClient() {
            auto self(shared_from_this());
            clientSockForClientSessionClass.async_read_some(buffer(receiveBuffer, MAX_RECEIVE_BUFFER_SIZE), [this, self](boost::system::error_code errorCode, size_t receiveBufferSize) {
                if (!errorCode) {
                    clientMessage = receiveBuffer;

                    if (receiveBufferSize <= INT_MAX)
                        clientMessageLength = static_cast<int>(receiveBufferSize);

                    childProcessID = fork();
                    if (childProcessID == 0) {                        
                        ParseClientMessage();
                        SetEnv();
                        clientSock = clientSockForClientSessionClass.native_handle();
                        dup2(clientSock, STDIN_FILENO);
                        dup2(clientSock, STDOUT_FILENO);
                        dup2(clientSock, STDERR_FILENO);
                        clientSockForClientSessionClass.close();
                        WriteMessageToClient();
                    }
                    clientSockForClientSessionClass.close();
                }
            });
        }

        void ParseClientMessage() {
            for (int i = 0; i < clientMessageLength; i++) {
                if (crCount == 1) {
                    if ((spaceCount == 3) || (spaceCount == 2)) {
                        if (clientMessage[i] == ' ') {
                            end = i - 1;
                            for (int j = start; j <= end; j++) {
                                if (spaceCount == 3)
                                    clientEnvVariable.requestMethod += clientMessage[j];
                                if (spaceCount == 2)
                                    clientEnvVariable.requestUri += clientMessage[j];
                            }
                            start = i + 1;
                            spaceCount -= 1;
                            continue;
                        }
                    }
                    if (spaceCount == 1) {
                        if (clientMessage[i] == '\r') {
                            end = i - 1;
                            for (int j = start; j <= end; j++)
                                clientEnvVariable.serverProtocol += clientMessage[j];
                            crCount -= 1;
                            continue;
                        }
                    }
                }
                if (crCount == 0) {
                    if (spaceCount == 1) {
                        if (clientMessage[i] == ' ') {
                            start = i + 1;
                            spaceCount -= 1;
                            continue;
                        }
                    }
                    if (spaceCount == 0) {
                        if (clientMessage[i] == '\r') {
                            end = i - 1;
                            for (int j = start; j <= end; j++)
                                clientEnvVariable.httpHost += clientMessage[j];
                            break;
                        }
                    }
                }
            }
            requestUriLength = static_cast<int>(clientEnvVariable.requestUri.length());
            for (int i = 0; i < requestUriLength; i++) {
                if (clientEnvVariable.requestUri[i] == '?') {
                    for (int j = (i + 1); j < requestUriLength; j++)
                        clientEnvVariable.queryString += clientEnvVariable.requestUri[j];
                    break;
                }
            }
            clientEnvVariable.serverAddr = clientSockForClientSessionClass.local_endpoint().address().to_string();
            clientEnvVariable.serverPort = to_string(clientSockForClientSessionClass.local_endpoint().port());
            clientEnvVariable.remoteAddr = clientSockForClientSessionClass.remote_endpoint().address().to_string();
            clientEnvVariable.remotePort = to_string(clientSockForClientSessionClass.remote_endpoint().port());
        }

        void SetEnv() {
            setenv("REQUEST_METHOD", clientEnvVariable.requestMethod.c_str(), 1);
            setenv("REQUEST_URI", clientEnvVariable.requestUri.c_str(), 1);
            setenv("QUERY_STRING", clientEnvVariable.queryString.c_str(), 1);
            setenv("SERVER_PROTOCOL", clientEnvVariable.serverProtocol.c_str(), 1);
            setenv("HTTP_HOST", clientEnvVariable.httpHost.c_str(), 1);
            setenv("SERVER_ADDR", clientEnvVariable.serverAddr.c_str(), 1);
            setenv("SERVER_PORT", clientEnvVariable.serverPort.c_str(), 1);
            setenv("REMOTE_ADDR", clientEnvVariable.remoteAddr.c_str(), 1);
            setenv("REMOTE_PORT", clientEnvVariable.remotePort.c_str(), 1);
        }

        void WriteMessageToClient() {
            cout << "HTTP/1.1 200 OK\r\n";
            cout.flush();
            for (int i = 0; i < requestUriLength; i++) {
                if (clientEnvVariable.requestUri[i] == '?') {
                    for (int j = 0; j < i; j++)
                        exeFileName += clientEnvVariable.requestUri[j];
                    break;
                }
                if (i == (requestUriLength - 1))
                    exeFileName += clientEnvVariable.requestUri;
            }
            exeFileName = "." + exeFileName;
            execlp(exeFileName.c_str(), exeFileName.c_str(), NULL);
        }
};

class HttpServer {
    public:
        HttpServer(int httpServerListenPort) : httpServerAcceptor(httpServerIoContext, tcp::endpoint(tcp::v4(), httpServerListenPort)) {
            AcceptClientConnection();
        }

    private:
        tcp::acceptor httpServerAcceptor;

        void AcceptClientConnection() {
            httpServerAcceptor.async_accept([this](boost::system::error_code errorCode, tcp::socket clientSockForHttpServerClass) {
                if (!errorCode)
                    make_shared<ClientSession>(move(clientSockForHttpServerClass))->Start();
                AcceptClientConnection();
            });
        }
};

void HandleSignal(int signal) {
    if (signal == SIGCHLD) {
        while (true) {
            if (waitpid(-1, NULL, WNOHANG) <= 0)
                break;
        }
        return;
    }
}

int main(int argc, char* argv[]) {
    signal(SIGCHLD, HandleSignal);
    HttpServer httpServer(atoi(argv[1]));
    httpServerIoContext.run();
    return 0;
}