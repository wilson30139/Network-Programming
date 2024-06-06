#include <vector>
#include <iostream>
#include <boost/asio.hpp>
#include <fstream>

#define MAX_RECEIVE_BUFFER_SIZE 99999

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

io_context consoleCgiIoContext;

class RemoteServer {
    public:
        string hostname = "";
        string port = "";
        string file = "";
};
vector<RemoteServer> remoteServerList;

class RemoteServerSession : public enable_shared_from_this<RemoteServerSession> {
    public:
        RemoteServerSession(int remoteServerSessionId) : remoteServerSessionIdForRemoteServerSessionClass (remoteServerSessionId) {
            inputFile.open("./test_case/" + remoteServerList[remoteServerSessionId].file, ios::in);
        }
        
        void Start() {
            ConvertDomainNameToIpAddress();
        }

    private:
        int remoteServerSessionIdForRemoteServerSessionClass = -1;
        fstream inputFile;
        tcp::resolver remoteServerResolver{consoleCgiIoContext};
        tcp::socket consoleCgiSock{consoleCgiIoContext};
        char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE] = {0};
        string remoteServerMessage = "";
        string consolePageText = "";

        void ConvertDomainNameToIpAddress() {
            auto self(shared_from_this());
            tcp::resolver::query remoteServerQuery(remoteServerList[remoteServerSessionIdForRemoteServerSessionClass].hostname, remoteServerList[remoteServerSessionIdForRemoteServerSessionClass].port);
            remoteServerResolver.async_resolve(remoteServerQuery, [this, self](boost::system::error_code errorCode, tcp::resolver::iterator remoteServerResolverIterator) {
                if (!errorCode)
                    ConnectRemoteServer(remoteServerResolverIterator);
            });
        }

        void ConnectRemoteServer(tcp::resolver::iterator remoteServerResolverIterator) {
            auto self(shared_from_this());
            consoleCgiSock.async_connect(*remoteServerResolverIterator, [this, self, remoteServerResolverIterator](boost::system::error_code errorCode) {
                if (!errorCode)
                    ReadMessageFromRemoteServer();
            });
        }

        void ReadMessageFromRemoteServer() {
            auto self(shared_from_this());
            bzero(receiveBuffer, MAX_RECEIVE_BUFFER_SIZE);
            consoleCgiSock.async_read_some(buffer(receiveBuffer, MAX_RECEIVE_BUFFER_SIZE), [this, self](boost::system::error_code errorCode, size_t receiveBufferSize) {
                if (!errorCode) {
                    remoteServerMessage = "";
                    consolePageText = "";
                    remoteServerMessage = receiveBuffer;
                    ReplaceCharacterToHtml();
                    cout << "<script>document.getElementById('remote-server-session-" << remoteServerSessionIdForRemoteServerSessionClass << "').innerHTML += '" << consolePageText << "';</script>";
                    cout.flush();
                    if (remoteServerMessage.find("%") != string::npos)
                        WriteMessageToRemoteServer();
                    else
                        ReadMessageFromRemoteServer();
                }
            });
        }

        void ReplaceCharacterToHtml() {
            int remoteServerRequestLength = static_cast<int>(remoteServerMessage.length());
            for (int i = 0; i < remoteServerRequestLength; i++) {
                if (remoteServerMessage[i] == '<') {
                    consolePageText += "&lt;";
                    continue;
                }
                if (remoteServerMessage[i] == '>') {
                    consolePageText += "&gt;";
                    continue;
                }
                if (remoteServerMessage[i] == '&') {
                    consolePageText += "&amp;";
                    continue;
                }
                if (remoteServerMessage[i] == '\'') {
                    consolePageText += "&apos;";
                    continue;
                }
                if (remoteServerMessage[i] == '"') {
                    consolePageText += "&quot;";
                    continue;
                }
                if (i < (remoteServerRequestLength - 1)) {
                    if ((remoteServerMessage[i] == '\r') && (remoteServerMessage[i + 1] == '\n')) {
                        consolePageText += "&NewLine;";
                        i += 1;
                        continue;
                    }
                }
                if (remoteServerMessage[i] == '\r') {
                    consolePageText += "";
                    continue;
                }
                if (remoteServerMessage[i] == '\n') {
                    consolePageText += "&NewLine;";
                    continue;
                }
                consolePageText += remoteServerMessage[i];
            }
        }

        void WriteMessageToRemoteServer() {
            auto self(shared_from_this());
            remoteServerMessage = "";
            consolePageText = "";
            getline(inputFile, remoteServerMessage);
            remoteServerMessage += "\n";
            ReplaceCharacterToHtml();
            cout << "<script>document.getElementById('remote-server-session-" << remoteServerSessionIdForRemoteServerSessionClass << "').innerHTML += '<b>" << consolePageText << "</b>';</script>";
            async_write(consoleCgiSock, buffer(remoteServerMessage.c_str(), remoteServerMessage.length()), [this, self](boost::system::error_code errorCode, size_t /*length*/) {
                if (!errorCode)
                    ReadMessageFromRemoteServer();
            });
        }
};

vector<string> SplitQueryString(string queryString) {
    vector<string> parameterListForQueryString;
    string parameterForQueryString = "";
    int start = 0;
    int end = 0;

    int queryStringLength = static_cast<int>(queryString.length());
    for (int i = 0; i < queryStringLength; i++) {
        if ((queryString[i] == '&') || (i == (queryStringLength - 1))) {
            parameterForQueryString = "";
            if (queryString[i] == '&')
                end = i - 1;
            if (i == (queryStringLength - 1))
                end = i;
            for (int j = start; j <= end; j++) {
                if (queryString[j] == '=') {
                    for (int k = (j + 1); k <= end; k++)
                        parameterForQueryString += queryString[k];
                    break;
                }
            }
            start = i + 1;
            parameterListForQueryString.push_back(parameterForQueryString);
        }
    }

    return parameterListForQueryString;
}

void ParseQueryString() {
    remoteServerList.clear();
    string queryString = getenv("QUERY_STRING");
    vector<string> parameterListForQueryString = SplitQueryString(queryString);

    int parameterListForQueryStringLength = static_cast<int>(parameterListForQueryString.size());
    int index = 0;
    while (true) {
        if (index >= parameterListForQueryStringLength)
            break;
        if ((parameterListForQueryString[index] != "") && (parameterListForQueryString[index + 1] != "") && (parameterListForQueryString[index + 2] != "")) {
            RemoteServer remoteServer;
            remoteServer.hostname = parameterListForQueryString[index];
            remoteServer.port = parameterListForQueryString[index + 1];
            remoteServer.file = parameterListForQueryString[index + 2];
            remoteServerList.push_back(remoteServer);
        }
        index += 3;
    }
}

void ShowConsolePage() {
    cout << "Content-type: text/html\r\n\r\n";
    string consolePageText = R""""(
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8" />
                <title>NP Project 3 Sample Console</title>
                <link
                    rel="stylesheet"
                    href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
                    integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
                    crossorigin="anonymous"
                />
                <link
                    href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
                    rel="stylesheet"
                />
                <link
                    rel="icon"
                    type="image/png"
                    href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
                />
                <style>
                    * {
                        font-family: 'Source Code Pro', monospace;
                        font-size: 1rem !important;
                    }
                    body {
                        background-color: #212529;
                    }
                    pre {
                        color: #cccccc;
                    }
                    b {
                        color: #01b468;
                    }
                </style>
            </head>
            <body>
                <table class="table table-dark table-bordered">
                    <thead>
                        <tr id="table-head">
                        </tr>
                    </thead>
                    <tbody>
                        <tr id="table-body">
                        </tr>
                    </tbody>
                </table>
            </body>
        </html> 
    )"""";
    cout << consolePageText;

    int remoteServerListLength = static_cast<int>(remoteServerList.size());
    for (int i = 0; i < remoteServerListLength; i++) {
        cout << "<script>document.getElementById('table-head').innerHTML += '<th scope=\"col\">" << remoteServerList[i].hostname << ":" << remoteServerList[i].port << "</th>';</script>";
        cout << "<script>document.getElementById('table-body').innerHTML += '<td><pre id=\\'remote-server-session-" << i << "\\' class=\"mb-0\"></pre></td>';</script>";
    }
}

int main() {
    ParseQueryString();
    ShowConsolePage();
    int remoteServerListLength = static_cast<int>(remoteServerList.size());
    for (int i = 0; i < remoteServerListLength; i++)
        make_shared<RemoteServerSession>(i)->Start();
    consoleCgiIoContext.run();
    return 0;
}