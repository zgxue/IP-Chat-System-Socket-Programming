#include "../include/Client.h"
#include "../include/global.h"
#include "../include/logger.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sstream>
#include <assert.h>
#include <ifaddrs.h>



using namespace std;


int Client::start(){
	string _portStr = selfPort;
	cout << "This is Client process with port : " << _portStr << endl;
	//现在一个机器的上运行，127.0.0.1， 两个程序，分配不同监听端口
	//todo: 需要先读取处理 cmd，出了 IP, LIST, etc。connect 前的第一件事应该是 LOGIN

	//serverSocket = connect_to_host(server_ip, atoi(server_port.c_str()));

  string msg;
	while (TRUE) {
		std::cout << "[Client@Xue589]$" << std::endl;
		// char *msg = (char*) malloc(sizeof(char) * MSG_SIZE);
		// memset(msg, '\0', MSG_SIZE);
		// if (fgets(msg, MSG_SIZE-1, stdin) == NULL) {
		// 	exit(-1);
		// }
		// printf("I got: %s(size:%d chars)\n", msg, strlen(msg));
		// std::cout << "SENDing it to the remote server..." << '\n';
		// if (send(server, msg, strlen(msg), 0) == strlen(msg)) {
		// 	std::cout << "Done" << std::endl;
		// }

		getline(cin, msg);

    cout << "Start parseCmd ..."<<endl;
		parseCmd(msg);
    cout << "Sucessfully finish parseCmd ..."<<endl;

		// sendMsgtoSocket(serverSocket, msg);
		// string recvedMsg = recvMsgfromSocket(serverSocket);
		// std::cout << "Recved msg is: " << recvedMsg.c_str() << '\n';

	}
	return 1;
}
int Client::connect_to_host(string server_ip, int server_port){
	int fdsocket;
	struct sockaddr_in remoteServerAddr;
	fdsocket = socket(AF_INET, SOCK_STREAM, 0);
	if (fdsocket < 0) {
		std::cerr << "XueError: Failed to create socket.[Client::connect_to_host]" << std::endl;
	}
	bzero(&remoteServerAddr, sizeof(remoteServerAddr));
	remoteServerAddr.sin_family = AF_INET;
	inet_pton(AF_INET, server_ip.c_str(), &remoteServerAddr.sin_addr);
	remoteServerAddr.sin_port = htons(server_port);

	if (connect(fdsocket, (struct sockaddr *)&remoteServerAddr, (socklen_t)sizeof(remoteServerAddr))<0) {
		std::cerr << "XueError: Conect failed.[Client::connect_to_host]" << std::endl;
	}
	return fdsocket;
}

string Client::getMyHostName(){
	char *msg = (char*)malloc(MSG_SIZE);
	gethostname(msg, MSG_SIZE);
	return string(msg);
}
int Client::sendMsgtoSocket(int _socket, string msg){
	std::cout << "SENDing it to the remote server...";
	if (send(_socket, msg.c_str(), msg.length(), 0) == msg.length()) {
		std::cout << "Done!" << std::endl;
	}
	return 1;
}
string Client::recvMsgfromSocket(int _socket){
	//initialize buffer to receive response
	char *buffer = (char*) malloc(sizeof(char) * BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);
	string ret = "";
	if (recv(_socket, buffer, BUFFER_SIZE, 0) >= 0) {
    ret = string(buffer);
    free(buffer);
		return ret;
	}
  return ret;
}
int Client::parseCmd(string cmd){
	stringstream ss(cmd);
	string token[ARGN_MAX];
	int nToken = 0;
	while (ss >> token[nToken] && nToken < ARGN_MAX) {
		nToken++;
	}
	if (nToken == 0) {
		exit(-1);
	}

	//Process cmds
	string cmder = token[0];

	if (cmder == "AUTHOR") {
		assert(nToken == 1);
		string myUBIT = onAUTHOR();
		assert(myUBIT != "");
		printf("[%s:SUCCESS]\n",cmder.c_str());
		printf("I, %s, have read and understood the course academic integrity policy.\n", myUBIT.c_str());
		printf("[%s:END]\n", cmder.c_str());
	}
	else if (cmder == "IP") {
		assert(nToken == 1);
		string ip_addr = onIP();
		if(ip_addr != ""){
			printf("[%s:SUCCESS]\n",cmder.c_str());
			printf("IP:%s\n",ip_addr.c_str());
			printf("[%s:END]\n", cmder.c_str());
		}else{
			printf("[%s:ERROR]\n",cmder.c_str());
			printf("[%s:END]\n", cmder.c_str());
		}
	}
	else if (cmder == "PORT") {
		assert(nToken == 1);
		string port_num = onPORT();
		if(port_num != ""){
			printf("[%s:SUCCESS]\n",cmder.c_str());
			printf("IP:%s\n",port_num.c_str());
			printf("[%s:END]\n", cmder.c_str());
		}else{
			printf("[%s:ERROR]\n",cmder.c_str());
			printf("[%s:END]\n", cmder.c_str());
		}
	}

	else if (cmder == "LIST") {assert(nToken == 1); onLIST();}
	else if (cmder == "LOGIN") {
    cout << "Compare if cmder is LOGIN..."<<endl;
    assert(nToken == 3);
    onLOGIN(token[1], token[2]);
    cout << "Finish onLOGIN execution..."<<endl;
  }
	else if (cmder == "REFRESH") {assert(nToken == 1); onREFRESH();}
	else if (cmder == "SEND") {assert(nToken == 3); onSEND(token[1], token[2]);}
	else if (cmder == "BLOCK") {assert(nToken == 2); onBLOCK(token[1]);}
	else if (cmder == "UNBLOCK") {assert(nToken == 2); onUNBLOCK(token[1]);}
	else if (cmder == "LOGOUT") {assert(nToken == 1); onLOGOUT();}
	else if (cmder == "EXIT") {assert(nToken == 1); onEXIT();}
	else{std::cerr << "XueError: "<< cmder <<" | NO such commander!" << std::endl;}
  return 1;
}

string getMyHostName(){
	char *msg = (char*)malloc(MSG_SIZE);
	gethostname(msg, MSG_SIZE);
	return string(msg);
}

string Client::onAUTHOR(){
	string myUBIT = "zhenggan";
	return myUBIT;
	// printf("I, %s, have read and understood the course academic integrity policy.\n", myUBIT.c_str());
}
string Client::onIP(){
	struct ifaddrs *ifaddr, *ifa;
  struct sockaddr_in *sa;
  char *ip_addr;

  if(getifaddrs (&ifaddr) == -1){
		std::cerr << "XueError: getifaddrs == -1 [Client:onIP]" << std::endl;
		return "";
	}

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL){
			continue;
		}
  	if (ifa->ifa_addr->sa_family==AF_INET && strncmp(ifa->ifa_name,"eth0",BUFFER_SIZE)==0) {
    	sa = (struct sockaddr_in *) ifa->ifa_addr;
      ip_addr = inet_ntoa(sa->sin_addr);
    }
  }

    freeifaddrs(ifaddr);
    return string(ip_addr);
}
string Client::onPORT(){
	return selfPort;
}

string Client::onLIST(){}
string Client::onLOGIN(string _serverIP, string _serverPort){
  //identify themselves to the server
  //get the list of other-logged in clients
  //get bufferd messages
  /**
  * 1, 发送自己的信息给 server处理，2， 接收 server 发来的信息并处理。
  */
  serverSocket = connect_to_host(_serverIP, atoi(_serverPort.c_str()));
  server_ip = _serverIP;
  server_port = _serverPort;

  string request = string("LOGIN") +" "+ selfHostName +" "+ selfIP +" "+ selfPort;

  sendMsgtoSocket(serverSocket, request);

  // string t = recvMsgfromSocket(serverSocket);
  // cout <<"Recv msg is : "<< t <<endl;
  cout << "Done the request Sending! (if double free or corruption appears again?)"<<endl;

}
string Client::onREFRESH(){}
string Client::onSEND(string _clientIP, string _msg){}
string Client::onBLOCK(string _clientIP){}
string Client::onUNBLOCK(string _clientIP){}
string Client::onLOGOUT(){
  assert(serverSocket >= 0);

  string request = string("LOGOUT");

  sendMsgtoSocket(serverSocket, request);
  string t = recvMsgfromSocket(serverSocket);
  cout <<"Recv msg is : "<< t <<endl;
}
string Client::onEXIT(){}
