#ifndef SERVER_H_
#define SERVER_H_

#include <string>
#include <vector>
#include "../include/LoggedInListItemServer.h"

using namespace std;


class Server{
private:
	string selfPort;
	string selfIP;
	string selfHostName;

	// todo: 需要一个结构来维护logged in 的 client List数据
  std::vector<LoggedInListItemServer> loggedInList;

public:
	Server(string _port)
		:selfPort(_port),
		selfIP(onIP()),
		selfHostName(getMyHostName())
		{}

	int start();
	int parseCmd(string cmd); //Cmd from stdin
  int parseRequest(int fdaccept, string requestStr);  //request from sockets
	string getMyHostName();
  int sendMsgtoSocket(int _socket, string msg);
  string recvMsgfromSocket(int _socket);

  //deal with LoggedInList
  void testSortVector();
  int addLoggedInList(LoggedInListItemServer item);
  int rmLoggedInList(string ip);
  int printLoggedInList();

	//both
	string onAUTHOR();
 	string onIP();
	string onPORT();
	string onLIST();
	//shell
	string onSTATISTICS();
	string onBLOCKED(string _clientIP);



};

#endif
