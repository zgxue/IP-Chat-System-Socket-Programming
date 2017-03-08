#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include <vector>
#include "../include/LoggedInListItemClient.h"

using namespace std;


class Client{
private:
	string selfPort;
	string selfIP;
	string server_ip;
	string server_port;
	string selfHostName;

  int serverSocket;

  // todo: 需要一个结构来维护logged in 的 client List数据,REFRESER 会从 server 获取新的
  // login 和 refresher 都会请求最近的 list。
  std::vector<LoggedInListItemClient> loggedInList;


public:
	Client(string _port)
		: selfPort(_port),
		selfIP(onIP()),
		server_ip(""),
		server_port(""),
		selfHostName(getMyHostName()),
        serverSocket(-1)
		{}

	int start();
    int startNew();
	int connect_to_host(string server_ip, int server_port);//返回成功连接的remoteServer 的 fdsocket
	int parseCmd(string cmd);
	int sendMsgtoSocket(int _socket, string msg);
	string recvMsgfromSocket(int _socket);
    string recvMsgfromSocketWithLoop(int _socket);

	string getMyHostName();

	string onAUTHOR();
	string onIP();
	string onPORT();
	string onLIST();

	//client
	string onLOGIN(string _serverIP, string _serverPort);
	string onREFRESH();
	string onSEND(string _clientIP, string _msg);
	string onBLOCK(string _clientIP);
	string onUNBLOCK(string _clientIP);
	string onLOGOUT();
	string onEXIT();
    string onBROADCAST(string strBroadcast);
	// int onSENDFILE(string _clientIP, string _filePath);

	void testSortVector();
    vector<string> splitString(string str);

};


#endif
