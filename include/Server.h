#ifndef SERVER_H_
#define SERVER_H_

#include <string>
#include <vector>
#include "../include/LoggedInListItemServer.h"
#include "../include/LoggedInListItemClient.h"

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
		selfIP(getMyIP()),
		selfHostName(getMyHostName())
		{}

	int start();
    int connect_to_host(string server_ip, int server_port);//返回成功连接的remoteServer 的 fdsocket
    int parseCmd(string cmd); //Cmd from stdin
	int parseRequest(int fdaccept, string requestStr);  //request from sockets
	string getMyHostName();
    string getMyIP();
	int sendMsgtoSocket(int _socket, string msg);
	string recvMsgfromSocket(int _socket);
    int sendMsgtoSocketWithLength(int _socket);


    //deal with LoggedInList
	void testSortVector();
	int addLoggedInList(LoggedInListItemServer item);
	int rmLoggedInList(string ip);
	int printLoggedInList();
	vector<LoggedInListItemClient> getListForClient();

    int addBlockList(string blockingIP, string blockedIP);
    int rmBlockList(string unblockingIP, string unblockedIP);
    int isClientBeenBlocked(string blockedBy, string beBlocked);
    int isClientLoggedIn(string clientIP);
    int findIndexOfIpInLIST(string clientIP);
    string findClientIPfromSocket(int clientSocket);
    void errorLogPrint(string cmder);



    //both
	string onAUTHOR();
 	string onIP();
	string onPORT();
	string onLIST();
	//shell
	string onSTATISTICS();
	string onBLOCKED(string _clientIP);

    string inBROADCAST(int fromClientSocket, string msgBroadcast);
    string inSEND(int fromClientSocket, string toClientIP, string msgSend);
    string inEXIT(int fromClientSocket);

    string getHeaderOfString(string str);
    string getRestAfterRMHeader(string str);
    int getNumOfSegmentsOfString(string str);  //at least
    string to3charInt(int number);




};

#endif
