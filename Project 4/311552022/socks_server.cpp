#include <iostream>
#include <boost/asio.hpp>
#include <sys/wait.h>
#include <fstream>
#include <vector>

#define MAX_SOCKS4_REQUEST_SIZE 99
#define MAX_SOCKS4_REPLY_SIZE 8
#define MAX_MESSAGE_SIZE 99999

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::address;

io_context socksServerIoContext;

class FirewallRule {
    public:
        string operation = "";
        string commandCode = "";
        vector<string> ipAddress;
};

class ConnectionBetweenClientAndDestinationServer : public enable_shared_from_this<ConnectionBetweenClientAndDestinationServer> {
    public:
        ConnectionBetweenClientAndDestinationServer(tcp::socket clientSockForSocksServerClass) : clientSock(move(clientSockForSocksServerClass)) {

        }

        void Start() {
            ReceiveSocks4RequestFromClient();
        }

    private:
        tcp::socket clientSock;

        char socks4Request[MAX_SOCKS4_REQUEST_SIZE] = {0};
        
        // 1. Receive SOCKS4_REQUEST from the SOCKS client
        void ReceiveSocks4RequestFromClient() {
            bzero(socks4Request, MAX_SOCKS4_REQUEST_SIZE);
            auto self(shared_from_this());
            clientSock.async_read_some(buffer(socks4Request, MAX_SOCKS4_REQUEST_SIZE), [this, self](boost::system::error_code errorCode, size_t socks4RequestSize) {
                if (!errorCode)
                    GetConnectionResult(socks4RequestSize);
            });
        }

        string sourceIpAddress = "";
        string sourcePort = "";
        string destinationIpAddress = "";
        string destinationPort = "";
        string commandCode = "";
        string destinationDomainName = "";
        
        // 2. Get the destination IP and port from SOCKS4_REQUEST
        void GetConnectionResult(size_t socks4RequestSize) {
            sourceIpAddress = "";
            sourcePort = "";
            destinationIpAddress = "";
            destinationPort = "";
            commandCode = "";
            destinationDomainName = "";

            tcp::endpoint clientEndpoint = clientSock.remote_endpoint();
            sourceIpAddress = clientEndpoint.address().to_string();
            sourcePort = to_string(clientEndpoint.port());
            destinationIpAddress = to_string(static_cast<int>(static_cast<unsigned char>(socks4Request[4]))) + "." + to_string(static_cast<int>(static_cast<unsigned char>(socks4Request[5]))) + "." + to_string(static_cast<int>(static_cast<unsigned char>(socks4Request[6]))) + "." + to_string(static_cast<int>(static_cast<unsigned char>(socks4Request[7])));
            destinationPort = to_string(static_cast<int>(static_cast<unsigned char>(socks4Request[2])) * 256 + static_cast<int>(static_cast<unsigned char>(socks4Request[3])));
            commandCode = to_string(static_cast<int>(static_cast<unsigned char>(socks4Request[1])));
            if (commandCode == "1")
                commandCode = "CONNECT";
            if (commandCode == "2")
                commandCode = "BIND";
            if (destinationIpAddress == "0.0.0.1") {
                for (size_t i = 10; i < socks4RequestSize; i++) {
                    if (socks4Request[i] == 0x00)
                        break;
                    destinationDomainName += socks4Request[i];
                }
            }

            if (destinationDomainName.empty())
                CheckFirewall();
            else
                ConvertDomainNameToIpAddressForDestinationServer();
        }

        tcp::resolver destinationServerResolver{socksServerIoContext};

        void ConvertDomainNameToIpAddressForDestinationServer() {
            tcp::resolver::query destinationServerQuery(destinationDomainName, destinationPort);
            auto self(shared_from_this());
            destinationServerResolver.async_resolve(destinationServerQuery, [this, self](boost::system::error_code errorCode, tcp::resolver::iterator destinationServerResolverIterator) {
                if (!errorCode) {
                    destinationIpAddress = destinationServerResolverIterator->endpoint().address().to_string();
                    CheckFirewall();
                }
            });
        }

        string replyCondition = "";
        
        // 3. Check the firewall (socks.conf), and send SOCKS4_REPLY to the SOCKS client if rejected
        void CheckFirewall() {
            replyCondition = "";

            ifstream firewallFile("socks.conf");

            vector<FirewallRule> firewallRuleList;
            string singleLineInput = "";
            string word = "";
            int spaceCount = 2;
            int start = 0;
            int end = 0;
            int startForIpAddress = start;
            int endForIpAddress = start;
            string number = "";

            while (getline(firewallFile, singleLineInput)) {
                FirewallRule firewallRule;
                spaceCount = 2;
                start = 0;
                end = 0;
                for (size_t i = 0; i < singleLineInput.length(); i++) {
                    if ((singleLineInput[i] == ' ') || (i == (singleLineInput.length() - 1))) {
                        word = "";
                        if (singleLineInput[i] == ' ')
                            end = i - 1;
                        else if (i == (singleLineInput.length() - 1))
                            end = i;
                        if ((spaceCount == 2) || (spaceCount == 1)) {
                            for (int j = start; j <= end; j++)
                                word += singleLineInput[j];
                            if (spaceCount == 2)
                                firewallRule.operation = word;
                            else if (spaceCount == 1)
                                firewallRule.commandCode = word;
                        } else if (spaceCount == 0) {
                            startForIpAddress = start;
                            endForIpAddress = start;
                            number = "";
                            for (int j = start; j <= end; j++) {
                                if ((singleLineInput[j] == '.') || (j == end)) {
                                    number = "";
                                    if (singleLineInput[j] == '.')
                                        endForIpAddress = j - 1;
                                    else if (j == end)
                                        endForIpAddress = j;
                                    for (int k = startForIpAddress; k <= endForIpAddress; k++)
                                        number += singleLineInput[k];
                                    startForIpAddress = j + 1;
                                    firewallRule.ipAddress.push_back(number);
                                }
                            }
                        }
                        start = i + 1;
                        spaceCount -= 1;
                    }
                }
                firewallRuleList.push_back(firewallRule);
            }

            vector<string> ipAddress;
            startForIpAddress = 0;
            endForIpAddress = 0;
            number = "";
            for (size_t i = 0; i < destinationIpAddress.length(); i++) {
                if ((destinationIpAddress[i] == '.') || (i == (destinationIpAddress.length() - 1))) {
                    number = "";
                    if (destinationIpAddress[i] == '.')
                        endForIpAddress = i - 1;
                    else if (i == (destinationIpAddress.length() - 1))
                        endForIpAddress = i;
                    for (int j = startForIpAddress; j <= endForIpAddress; j++)
                        number += destinationIpAddress[j];
                    startForIpAddress = i + 1;
                    ipAddress.push_back(number);
                }
            }

            bool isHandle = false;

            for (size_t i = 0; i < firewallRuleList.size(); i++) {
                if (firewallRuleList[i].operation == "permit") {
                    if (((firewallRuleList[i].commandCode == "c") && (commandCode == "CONNECT")) || ((firewallRuleList[i].commandCode == "b") && (commandCode == "BIND"))) {
                        for (size_t j = 0; j < firewallRuleList[i].ipAddress.size(); j++) {
                            if (firewallRuleList[i].ipAddress[j] == "*") {
                                if (j < (firewallRuleList[i].ipAddress.size() - 1))
                                    continue;
                                else if (j == (firewallRuleList[i].ipAddress.size() - 1)) {
                                    isHandle = true;
                                    replyCondition = "Accept";
                                    CheckCommandCode();
                                    break;
                                }
                            }
                            if (firewallRuleList[i].ipAddress[j] != ipAddress[j])
                                break;
                            if (j == (firewallRuleList[i].ipAddress.size() - 1)) {
                                isHandle = true;
                                replyCondition = "Accept";
                                CheckCommandCode();
                            }
                        }
                    }
                }
                if (firewallRuleList[i].operation == "deny") {
                    if (((firewallRuleList[i].commandCode == "c") && (commandCode == "CONNECT")) || ((firewallRuleList[i].commandCode == "b") && (commandCode == "BIND"))) {
                        for (size_t j = 0; j < firewallRuleList[i].ipAddress.size(); j++) {
                            if (firewallRuleList[i].ipAddress[j] == "*") {
                                if (j < (firewallRuleList[i].ipAddress.size() - 1))
                                    continue;
                                else if (j == (firewallRuleList[i].ipAddress.size() - 1)) {
                                    isHandle = true;
                                    replyCondition = "Reject";
                                    SendSocks4ReplyToClient();
                                    break;
                                }
                            }
                            if (firewallRuleList[i].ipAddress[j] != ipAddress[j])
                                break;
                            if (j == (firewallRuleList[i].ipAddress.size() - 1)) {
                                isHandle = true;
                                replyCondition = "Reject";
                                SendSocks4ReplyToClient();
                            }
                        }
                    }
                }
                if (isHandle)
                    break;
                if (i == (firewallRuleList.size() - 1)) {
                    replyCondition = "Reject";
                    SendSocks4ReplyToClient();
                }
            }
        }

        // 4. Check CD value and choose one of the operations
        void CheckCommandCode() {
            if (commandCode == "CONNECT")
                ConnectDestinationServer();
            if (commandCode == "BIND")
                BindAndListenPortForSocksServer();
        }

        tcp::socket destinationServerSock{socksServerIoContext};
        
        // 4.1. Connect to the destination (CONNECT)
        void ConnectDestinationServer() {
            tcp::endpoint destinationServerEndpoint(address::from_string(destinationIpAddress), static_cast<unsigned short>(stoi(destinationPort)));
            auto self(shared_from_this());
            destinationServerSock.async_connect(destinationServerEndpoint, [this, self](boost::system::error_code errorCode) {
                if (!errorCode)
                    SendSocks4ReplyToClient();
            });
        }

        string socksServerBindAndListenPort = "";
        tcp::acceptor socksServerAcceptor{socksServerIoContext};

        // 4.1. Bind and listen a port (BIND)
        void BindAndListenPortForSocksServer() {
            socksServerBindAndListenPort = "";
            tcp::endpoint socksServerEndpoint(tcp::v4(), 0);
            socksServerAcceptor.open(socksServerEndpoint.protocol());
            socksServerAcceptor.bind(socksServerEndpoint);
            socksServerAcceptor.listen();
            socksServerBindAndListenPort = to_string(socksServerAcceptor.local_endpoint().port());
            SendSocks4ReplyToClient();
            AcceptDestinationServerConnection();
        }

        char socks4Reply[MAX_SOCKS4_REPLY_SIZE] = {0};

        // 4.2. Send SOCKS4_REPLY to the SOCKS client (CONNECT)
        // 4.2. Send SOCKS4_REPLY to SOCKS client to tell which port to connect (BIND)
        void SendSocks4ReplyToClient() {
            bzero(socks4Reply, MAX_SOCKS4_REPLY_SIZE);
            // VN
            socks4Reply[0] = 0;
            // CD
            socks4Reply[1] = 90; // Accept
            // DSTPORT
            if (commandCode == "CONNECT") {
                socks4Reply[2] = 0;
                socks4Reply[3] = 0;
            }
            if ((commandCode == "BIND") && (replyCondition == "Accept")) {
                socks4Reply[2] = stoi(socksServerBindAndListenPort) / 256;
                socks4Reply[3] = stoi(socksServerBindAndListenPort) % 256;
            }
            // DSTIP
            socks4Reply[4] = 0;
            socks4Reply[5] = 0;
            socks4Reply[6] = 0;
            socks4Reply[7] = 0;

            auto self(shared_from_this());
            clientSock.async_send(buffer(socks4Reply, MAX_SOCKS4_REPLY_SIZE), [this, self](boost::system::error_code errorCode, size_t length) {
                if (!errorCode) {
                    ShowConnectionResult();
                    if (replyCondition == "Accept") {
                        if (commandCode == "CONNECT")
                            StartRelayTraffic();
                    } else if (replyCondition == "Reject")
                        clientSock.close();
                }
            });
        }        

        // 4.4. Accept connection from destination and send another SOCKS4_REPLY to SOCKS client (BIND)
        void AcceptDestinationServerConnection() {
            auto self(shared_from_this());
            socksServerAcceptor.async_accept([this, self](boost::system::error_code errorCode, tcp::socket newDestinationServerSock) {
                if (!errorCode) {
                    destinationServerSock = move(newDestinationServerSock);
                    SendSocks4ReplyToClient();
                    StartRelayTraffic();
                }
            });
        }

        void ShowConnectionResult() {
            cout << "<S_IP>: " << sourceIpAddress << endl;
            cout << "<S_PORT>: " << sourcePort << endl;
            cout << "<D_IP>: " << destinationIpAddress << endl;
            cout << "<D_PORT>: " << destinationPort << endl;
            cout << "<Command>: " << commandCode << endl;
            cout << "<Reply>: " << replyCondition << endl;
        }

        // 4.3. Start relaying traffic on both directions (CONNECT)
        // 4.5. Start relaying traffic on both directions (BIND)
        void StartRelayTraffic() {
            ReceiveMessageFromClient();
            ReceiveMessageFromDestinationServer();
        }

        char giveToDestinationServerMessage[MAX_MESSAGE_SIZE] = {0};
        
        void ReceiveMessageFromClient() {
            bzero(giveToDestinationServerMessage, MAX_MESSAGE_SIZE);
            auto self(shared_from_this());
            clientSock.async_read_some(buffer(giveToDestinationServerMessage, MAX_MESSAGE_SIZE), [this, self](boost::system::error_code errorCode, size_t receiveMessageLength) {
                if (!errorCode)
                    SendMessageToDestinationServer(receiveMessageLength);
                if (errorCode) {
                    if (errorCode == error::eof) {
                        destinationServerSock.close();
                        clientSock.close();
                    }
                }
            });
        }        

        void SendMessageToDestinationServer(size_t receiveMessageLength) {
            auto self(shared_from_this());
            destinationServerSock.async_send(buffer(giveToDestinationServerMessage, receiveMessageLength), [this, self](boost::system::error_code errorCode, size_t sendMessageLength) {
                if (!errorCode)
                    ReceiveMessageFromClient();
            });
        }

        char giveToClientMessage[MAX_MESSAGE_SIZE] = {0};

        void ReceiveMessageFromDestinationServer() {
            bzero(giveToClientMessage, MAX_MESSAGE_SIZE);
            auto self(shared_from_this());
            destinationServerSock.async_read_some(buffer(giveToClientMessage, MAX_MESSAGE_SIZE), [this, self](boost::system::error_code errorCode, size_t receiveMessageLength) {
                if (!errorCode)
                    SendMessageToClient(receiveMessageLength);
                if (errorCode) {
                    if (errorCode == error::eof) {
                        destinationServerSock.close();
                        clientSock.close();
                    }
                }
            });
        }
        
        void SendMessageToClient(size_t receiveMessageLength) {
            auto self(shared_from_this());
            clientSock.async_send(buffer(giveToClientMessage, receiveMessageLength), [this, self](boost::system::error_code errorCode, size_t sendMessageLength) {
                if (!errorCode)
                    ReceiveMessageFromDestinationServer();
            });
        }
};

class SocksServer {
    public:
        SocksServer(int socksServerBindAndListenPort) : socksServerAcceptor(socksServerIoContext, tcp::endpoint(tcp::v4(), socksServerBindAndListenPort)) {
            AcceptClientConnection();
        }

    private:
        tcp::acceptor socksServerAcceptor;
        
        pid_t childProcessID;

        void AcceptClientConnection() {
            socksServerAcceptor.async_accept([this](boost::system::error_code errorCode, tcp::socket clientSock) {
                if (!errorCode) {
                    socksServerIoContext.notify_fork(io_context::fork_prepare);
                    childProcessID = fork();
                    if (childProcessID == 0) {
                        socksServerIoContext.notify_fork(io_context::fork_child);
                        make_shared<ConnectionBetweenClientAndDestinationServer>(move(clientSock))->Start();
                    }
                    if (childProcessID > 0) {
                        socksServerIoContext.notify_fork(io_context::fork_parent);
                        clientSock.close();
                        AcceptClientConnection();
                    }
                }
            });
        }        
};

void HandleSignal(int signal) {
    if (signal == SIGCHLD) {
        while (true) {
            if (waitpid(-1, NULL, WNOHANG) <= 0)
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    signal(SIGCHLD, HandleSignal);
    SocksServer socksServer(atoi(argv[1]));
    socksServerIoContext.run();
    return 0;
}