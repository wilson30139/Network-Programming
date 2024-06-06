#include <boost/asio.hpp>
#include <fstream>
#include <vector>

#define MAX_RECEIVE_BUFFER_SIZE 99999

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

io_context cgiServerIoContext;
string clientQueryString = "";

class RemoteServer {
    public:
        string hostname = "";
        string port = "";
        string file = "";
};
vector<RemoteServer> remoteServerList;

class RemoteServerSession : public enable_shared_from_this<RemoteServerSession> {
    public:
        RemoteServerSession(int remoteServerSessionIdForClientSessionClass, shared_ptr<tcp::socket> _share_ptr) : remoteServerSessionIdForRemoteServerSessionClass(remoteServerSessionIdForClientSessionClass), share_ptr(_share_ptr) {
            inputFile.open("./test_case/" + remoteServerList[remoteServerSessionIdForClientSessionClass].file, ios::in);
        }

        void Start() {
            ConvertDomainNameToIpAddress();
        }

    private:
        int remoteServerSessionIdForRemoteServerSessionClass = -1;
        shared_ptr<tcp::socket> share_ptr;
        fstream inputFile;
        tcp::resolver remoteServerResolver{cgiServerIoContext};
        tcp::socket cgiServerSock{cgiServerIoContext};
        char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE] = {0};
        string remoteServerMessage = "";
        string consolePageText = "";
        string message = "";

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
            cgiServerSock.async_connect(*remoteServerResolverIterator, [this, self, remoteServerResolverIterator](boost::system::error_code errorCode) {
                if (!errorCode)
                    ReadMessageFromRemoteServer();
            });
        }

        void ReadMessageFromRemoteServer() {
            auto self(shared_from_this());
            memset(receiveBuffer, '\0', MAX_RECEIVE_BUFFER_SIZE);
            cgiServerSock.async_read_some(buffer(receiveBuffer, MAX_RECEIVE_BUFFER_SIZE), [this, self](boost::system::error_code errorCode, size_t receiveBufferSize) {
                if (!errorCode) {
                    remoteServerMessage = "";
                    consolePageText = "";
                    remoteServerMessage = receiveBuffer;
                    ReplaceCharacterToHtml();
                    message = R""""(
                        <script>document.getElementById('remote-server-session-)"""" + to_string(remoteServerSessionIdForRemoteServerSessionClass) + R""""(').innerHTML += ')"""" + consolePageText + R""""(';</script>
                    )"""";
                    WriteMessageToClient();
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

        void WriteMessageToClient() {
            auto self(shared_from_this());
            async_write(*share_ptr, buffer(message.c_str(), message.length()), [this, self](boost::system::error_code ec, size_t /*length*/) {

            });
        }

        void WriteMessageToRemoteServer() {
            auto self(shared_from_this());
            remoteServerMessage = "";
            consolePageText = "";
            getline(inputFile, remoteServerMessage);
            remoteServerMessage += "\n";
            ReplaceCharacterToHtml();
            message = R""""(
                <script>document.getElementById('remote-server-session-)"""" + to_string(remoteServerSessionIdForRemoteServerSessionClass) + R""""(').innerHTML += '<b>)"""" + consolePageText + R""""(</b>';</script>
            )"""";
            WriteMessageToClient();
            async_write(cgiServerSock, buffer(remoteServerMessage.c_str(), remoteServerMessage.length()), [this, self](boost::system::error_code errorCode, size_t /*length*/) {
                if (!errorCode)
                    ReadMessageFromRemoteServer();
            });
        }
};

class ClientSession : public enable_shared_from_this<ClientSession> {
    public:
        ClientSession(tcp::socket clientSockForCgiServerClass) : clientSockForClientSessionClass(move(clientSockForCgiServerClass)) {

        }

        void Start() {
            ReadMessageFromClient();
        }
    
    private:
        tcp::socket clientSockForClientSessionClass;
        char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE] = {0};
        string clientMessage = "";
        int clientMessageLength = -1;
        int spaceCount = 1;
        int start = 0;
        int end = 0;
        string fileName = "";
        string text = "";
        int clientQueryStringLength = -1;
        string parameterForClientQueryString = "";
        vector<string> parameterListForClientQueryString;
        int parameterListForClientQueryStringLength = -1;
        int remoteServerListLength = -1;

        void ReadMessageFromClient() {
            auto self(shared_from_this());
            clientSockForClientSessionClass.async_read_some(buffer(receiveBuffer, MAX_RECEIVE_BUFFER_SIZE), [this, self](boost::system::error_code errorCode, size_t receiveBufferSize) {
                if (!errorCode) {
                    clientMessage = receiveBuffer;

                    if (receiveBufferSize <= INT_MAX)
                        clientMessageLength = static_cast<int>(receiveBufferSize);

                    ParseClientMessage();

                    if (fileName == "panel.cgi")
                        BuildPanelPage();                        
                    if (fileName == "console.cgi") {
                        ParseClientQueryString();
                        BuildConsolePage();
                        shared_ptr<tcp::socket> ptrr(&clientSockForClientSessionClass);
                        for (int i = 0; i < remoteServerListLength; i++)
                            make_shared<RemoteServerSession>(i, ptrr)->Start();
                        cgiServerIoContext.run();
                    }
                }
            });
        }

        void ParseClientMessage() {
            clientQueryString = "";
            for (int i = 0; i < clientMessageLength; i++) {
                if (spaceCount == 1) {
                    if (clientMessage[i] == ' ') {
                        start = i + 2;
                        spaceCount -= 1;
                        continue;
                    }
                }
                if (spaceCount == 0) {
                    if (clientMessage[i] == ' ') {
                        end = i - 1;
                        for (int j = start; j <= end; j++) {
                            if (clientMessage[j] == '?') {
                                for (int k = (j + 1); k <= end; k++)
                                    clientQueryString += clientMessage[k];
                                break;
                            }
                            fileName += clientMessage[j];
                        }
                        break;
                    }
                }
            }
        }

        void BuildPanelPage() {
            string hostMenuText = "";
            for (int i = 1; i <= 12; i++) {
                text = R""""(
                    <option value="nplinux)"""" + to_string(i) + R""""(.cs.nycu.edu.tw">nplinux)"""" + to_string(i) + R""""(</option>
                )"""";
                hostMenuText += text;
            }

            string testCaseText = "";
            for (int i = 1; i <= 5; i++) {
                text = R""""(
                    <option value="t)"""" + to_string(i) + R""""(.txt">t)"""" + to_string(i) + R""""(.txt</option>
                )"""";
                testCaseText += text;
            }

            string panelPageText = "";
            text = "HTTP/1.1 200 OK\r\n";
            panelPageText += text;

            text = "Content-type: text/html\r\n\r\n";
            panelPageText += text;

            text = R""""(
                <!DOCTYPE html>
                <html lang="en">
                    <head>
                        <title>NP Project 3 Panel</title>
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
                            href="https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png"
                        />
                        <style>
                            * {
                                font-family: 'Source Code Pro', monospace;
                            }
                        </style>
                    </head>
                    <body class="bg-secondary pt-5">
            )"""";
            panelPageText += text;

            text = R""""(
                        <form action="console.cgi" method="GET">
                            <table class="table mx-auto bg-light" style="width: inherit">
                                <thead class="thead-dark">
                                    <tr>
                                        <th scope="col">#</th>
                                        <th scope="col">Host</th>
                                        <th scope="col">Port</th>
                                        <th scope="col">Input File</th>
                                    </tr>
                                </thead>
                                <tbody>
            )"""";
            panelPageText += text;
            
            for (int i = 0; i < 5; i++) {
                text = R""""(
                                    <tr>
                                        <th scope="row" class="align-middle">Session )"""" + to_string(i + 1) + R""""(</th>
                                        <td>
                                            <div class="input-group">
                                                <select name="h)"""" + to_string(i) + R""""(" class="custom-select">
                                                    <option></option>)"""" + hostMenuText + R""""(
                                                </select>
                                                <div class="input-group-append">
                                                    <span class="input-group-text">.cs.nycu.edu.tw</span>
                                                </div>
                                            </div>
                                        </td>
                                        <td>
                                            <input name="p)"""" + to_string(i) + R""""(" type="text" class="form-control" size="5" />
                                        </td>
                                        <td>
                                            <select name="f)"""" + to_string(i) + R""""(" class="custom-select">
                                                <option></option>
                                                )"""" + testCaseText + R""""(
                                            </select>
                                        </td>
                                    </tr>
                )"""";
                panelPageText += text;
            }

            text = R""""(
                                    <tr>
                                        <td colspan="3"></td>
                                        <td>
                                            <button type="submit" class="btn btn-info btn-block">Run</button>
                                        </td>
                                    </tr>
                                </tbody>
                            </table>
                        </form>
                    </body>
                </html>
            )"""";
            panelPageText += text;

            WriteMessageToClient(panelPageText);
        }

        void WriteMessageToClient(string message) {
            auto self(shared_from_this());
            async_write(clientSockForClientSessionClass, buffer(message.c_str(), message.length()), [this, self](boost::system::error_code ec, size_t /*length*/) {

            });
        }

        void SplitQueryString() {
            start = 0;
            end = 0;
            clientQueryStringLength = static_cast<int>(clientQueryString.length());
            for (int i = 0; i < clientQueryStringLength; i++) {
                if ((clientQueryString[i] == '&') || (i == (clientQueryStringLength - 1))) {
                    parameterForClientQueryString = "";
                    if (clientQueryString[i] == '&')
                        end = i - 1;
                    if (i == (clientQueryStringLength - 1))
                        end = i;
                    for (int j = start; j <= end; j++) {
                        if (clientQueryString[j] == '=') {
                            for (int k = (j + 1); k <= end; k++)
                                parameterForClientQueryString += clientQueryString[k];
                            break;
                        }
                    }
                    start = i + 1;
                    parameterListForClientQueryString.push_back(parameterForClientQueryString);
                }
            }
        }

        void ParseClientQueryString() {
            remoteServerList.clear();
            SplitQueryString();

            parameterListForClientQueryStringLength = static_cast<int>(parameterListForClientQueryString.size());

            int index = 0;
            while (true) {
                if (index >= parameterListForClientQueryStringLength)
                    break;
                if ((parameterListForClientQueryString[index] != "") && (parameterListForClientQueryString[index + 1] != "") && (parameterListForClientQueryString[index + 2] != "")) {
                    RemoteServer remoteServer;
                    remoteServer.hostname = parameterListForClientQueryString[index];
                    remoteServer.port = parameterListForClientQueryString[index + 1];
                    remoteServer.file = parameterListForClientQueryString[index + 2];
                    remoteServerList.push_back(remoteServer);
                }
                index += 3;
            }
        }

        void BuildConsolePage() {
            string consolePageText = "";
            text = "HTTP/1.1 200 OK\r\n";
            consolePageText += text;

            text = "Content-type: text/html\r\n\r\n";
            consolePageText += text;
            
            text = R""""(
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
            consolePageText += text;

            remoteServerListLength = static_cast<int>(remoteServerList.size());
            for (int i = 0; i < remoteServerListLength; i++) {
                text = R""""(
                    <script>document.getElementById('table-head').innerHTML += '<th scope="col">)"""" + remoteServerList[i].hostname + R""""(:)"""" + remoteServerList[i].port + R""""(</th>';</script>
                    <script>document.getElementById('table-body').innerHTML += '<td><pre id="remote-server-session-)"""" + to_string(i) + R""""(" class="mb-0"></pre></td>';</script>
                )"""";
                consolePageText += text;
            }

            WriteMessageToClient(consolePageText);
        }
};

class CgiServer {
    public:
        CgiServer(int cgiServerListenPort) : cgiServerAcceptor(cgiServerIoContext, tcp::endpoint(tcp::v4(), cgiServerListenPort)) {
            AcceptClientConnection();
        }

    private:
        tcp::acceptor cgiServerAcceptor;

        void AcceptClientConnection() {
            cgiServerAcceptor.async_accept([this](boost::system::error_code errorCode, tcp::socket clientSockForCgiServerClass) {
                if (!errorCode)
                    make_shared<ClientSession>(move(clientSockForCgiServerClass))->Start();
                AcceptClientConnection();
            });
        }
};

int main(int argc, char* argv[]) {
    CgiServer cgiServer(atoi(argv[1]));
    cgiServerIoContext.run();
    return 0;
}