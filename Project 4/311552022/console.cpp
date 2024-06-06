#include <vector>
#include <iostream>
#include <boost/asio.hpp>
#include <fstream>

#define MAX_SOCKS4_HEADER_SIZE 99
#define MAX_BUFFER_SIZE 99999

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

string socksServerHostname = "";
string socksServerPort = "";

class RemoteServerSession : public enable_shared_from_this<RemoteServerSession> {
    public:
        RemoteServerSession(int remoteServerSessionIdForExternal) : remoteServerSessionId (remoteServerSessionIdForExternal) {
            inputFile.open("./test_case/" + remoteServerList[remoteServerSessionIdForExternal].file, ios::in);
        }
        
        void Start() {
            ConvertDomainNameToIpAddress();
        }

    private:
        int remoteServerSessionId = -1;
        fstream inputFile;
        tcp::resolver socksServerResolver{consoleCgiIoContext};
        tcp::socket consoleCgiSock{consoleCgiIoContext};
        unsigned char socks4Request[MAX_SOCKS4_HEADER_SIZE] = {0};
        unsigned char socks4Reply[MAX_SOCKS4_HEADER_SIZE] = {0};
        char socksServerReceiveBuffer[MAX_BUFFER_SIZE] = {0};
        string socksServerMessage = "";
        string consolePageText = "";

        void ConvertDomainNameToIpAddress() {
            auto self(shared_from_this());
            tcp::resolver::query socksServerQuery(socksServerHostname, socksServerPort);
            socksServerResolver.async_resolve(socksServerQuery, [this, self](boost::system::error_code errorCode, tcp::resolver::iterator socksServerResolverIterator) {
                if (!errorCode)
                    ConnectSocksServer(socksServerResolverIterator);
            });
        }

        void ConnectSocksServer(tcp::resolver::iterator socksServerResolverIterator) {
            auto self(shared_from_this());
            consoleCgiSock.async_connect(*socksServerResolverIterator, [this, self, socksServerResolverIterator](boost::system::error_code errorCode) {
                if (!errorCode)
                    SendSocks4RequestToSocksServer();
            });
        }

        void SendSocks4RequestToSocksServer() {
            bzero(socks4Request, MAX_SOCKS4_HEADER_SIZE);
            // VN
            socks4Request[0] = 0x04;
            // CD
            socks4Request[1] = 0x01; // Connect 
            // DSTPORT
            socks4Request[2] = stoi(remoteServerList[remoteServerSessionId].port) / 256;
            socks4Request[3] = stoi(remoteServerList[remoteServerSessionId].port) % 256;
            // DSTIP
            socks4Request[4] = 0x00;
            socks4Request[5] = 0x00;
            socks4Request[6] = 0x00;
            socks4Request[7] = 0x01;
            // USERID
            socks4Request[8] = 0x13; // Random
            // NULL
            socks4Request[9] = 0x00;
            // DOMAIN NAME
            int hostnameLength = static_cast<int>(remoteServerList[remoteServerSessionId].hostname.length());
            for (int i = 0; i < hostnameLength; i++)
                socks4Request[10 + i] = remoteServerList[remoteServerSessionId].hostname[i];
            // NULL
            socks4Request[10 + hostnameLength] = 0x00;

            auto self(shared_from_this());
            consoleCgiSock.async_send(buffer(socks4Request, sizeof(unsigned char) * MAX_SOCKS4_HEADER_SIZE), [this, self](boost::system::error_code errorCode, size_t length) {
                if (!errorCode)
                    ReceiveSocksReplyFromSocksServer();
            });
        }

        void ReceiveSocksReplyFromSocksServer() {
            bzero(socks4Reply, MAX_SOCKS4_HEADER_SIZE);
            auto self(shared_from_this());
            consoleCgiSock.async_receive(buffer(socks4Reply, sizeof(unsigned char) * MAX_SOCKS4_HEADER_SIZE), [this, self](boost::system::error_code errorCode, size_t length) {
                if (!errorCode) {
                    if (((unsigned int) socks4Reply[1]) == 90)
                        ReceiveMessageFromSocksServer();
                }
            });
        }

        void ReceiveMessageFromSocksServer() {
            bzero(socksServerReceiveBuffer, MAX_BUFFER_SIZE);
            auto self(shared_from_this());
            consoleCgiSock.async_read_some(buffer(socksServerReceiveBuffer, MAX_BUFFER_SIZE), [this, self](boost::system::error_code errorCode, size_t socksServerReceiveBufferSize) {
                if (!errorCode) {
                    socksServerMessage = "";
                    consolePageText = "";
                    socksServerMessage = socksServerReceiveBuffer;
                    ReplaceCharacterToHtml();
                    cout << "<script>document.getElementById('remote-server-session-" << remoteServerSessionId << "').innerHTML += '" << consolePageText << "';</script>";
                    cout.flush();
                    if (socksServerMessage.find("%") != string::npos)
                        SendMessageToSocksServer();
                    else
                        ReceiveMessageFromSocksServer();
                }
            });
        }

        void ReplaceCharacterToHtml() {
            int socksServerRequestLength = static_cast<int>(socksServerMessage.length());
            for (int i = 0; i < socksServerRequestLength; i++) {
                if (socksServerMessage[i] == '<') {
                    consolePageText += "&lt;";
                    continue;
                }
                if (socksServerMessage[i] == '>') {
                    consolePageText += "&gt;";
                    continue;
                }
                if (socksServerMessage[i] == '&') {
                    consolePageText += "&amp;";
                    continue;
                }
                if (socksServerMessage[i] == '\'') {
                    consolePageText += "&apos;";
                    continue;
                }
                if (socksServerMessage[i] == '"') {
                    consolePageText += "&quot;";
                    continue;
                }
                if (i < (socksServerRequestLength - 1)) {
                    if ((socksServerMessage[i] == '\r') && (socksServerMessage[i + 1] == '\n')) {
                        consolePageText += "&NewLine;";
                        i += 1;
                        continue;
                    }
                }
                if (socksServerMessage[i] == '\r') {
                    consolePageText += "";
                    continue;
                }
                if (socksServerMessage[i] == '\n') {
                    consolePageText += "&NewLine;";
                    continue;
                }
                consolePageText += socksServerMessage[i];
            }
        }

        void SendMessageToSocksServer() {
            auto self(shared_from_this());
            socksServerMessage = "";
            consolePageText = "";
            getline(inputFile, socksServerMessage);
            socksServerMessage += "\n";
            ReplaceCharacterToHtml();
            cout << "<script>document.getElementById('remote-server-session-" << remoteServerSessionId << "').innerHTML += '<b>" << consolePageText << "</b>';</script>";
            async_write(consoleCgiSock, buffer(socksServerMessage.c_str(), socksServerMessage.length()), [this, self](boost::system::error_code errorCode, size_t /*length*/) {
                if (!errorCode)
                    ReceiveMessageFromSocksServer();
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
    socksServerHostname = "";
    socksServerPort = "";

    string queryString = getenv("QUERY_STRING");
    vector<string> parameterListForQueryString = SplitQueryString(queryString);

    int index = 0;
    while (true) {
        if (index == 15) {
            socksServerHostname = parameterListForQueryString[index];
            socksServerPort = parameterListForQueryString[index + 1];
            break;
        }
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