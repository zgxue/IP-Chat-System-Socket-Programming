#include "../include/Server.h"
#include "../include/global.h"
#include "../include/logger.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <cstring>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sstream>
#include <assert.h>
#include <ifaddrs.h>
#include <algorithm>
#include <iomanip>

using namespace std;

//test functions NEED deleted
void Server::testSortVector(){

    LoggedInListItemServer itemTemp = LoggedInListItemServer("euston", "34", 5701);
    loggedInList.push_back(itemTemp);
    itemTemp = LoggedInListItemServer("embankment", "35", 5000);
    loggedInList.push_back(itemTemp);
    itemTemp = LoggedInListItemServer("stones", "46", 4545);
    loggedInList.push_back(itemTemp);
    itemTemp = LoggedInListItemServer("highgate", "33", 5499);
    loggedInList.push_back(itemTemp);

    for (size_t i = 0; i < loggedInList.size(); i++) {
      LoggedInListItemServer t = loggedInList.at(i);
      cout <<i<< " : "<< t.hostname << " : " << t.ip_addr << " : "<< t.port_num <<endl;
    }

    sort(loggedInList.begin(), loggedInList.end());
    cout << "After Sorted:" << endl;
    for (size_t i = 0; i < loggedInList.size(); i++) {
      LoggedInListItemServer t = loggedInList.at(i);
      cout <<i<< " : "<< t.hostname << " : " << t.ip_addr << " : "<< t.port_num <<endl;
    }

    rmLoggedInList("35");
    cout << "After remove 35:" << endl;
    for (size_t i = 0; i < loggedInList.size(); i++) {
      LoggedInListItemServer t = loggedInList.at(i);
      cout <<i<< " : "<< t.hostname << " : " << t.ip_addr << " : "<< t.port_num <<endl;
    }
}

int Server::addLoggedInList(LoggedInListItemServer item){
  loggedInList.push_back(item);
  sort(loggedInList.begin(), loggedInList.end());
  return 1;
}
int Server::rmLoggedInList(string ip){
  //之后实现的时候需要注意，一个 ip 可能有多个端口的客户端，所以参数需要接受两个：ip 和 port
  for (size_t i = 0; i < loggedInList.size(); i++) {
    LoggedInListItemServer t = loggedInList.at(i);
    if (t.ip_addr == ip) {
      loggedInList.erase(loggedInList.begin() + i);
      return 1;
    }
  }
  return 0;
}

int Server::start(){
	string _portStr = selfPort;

	cout << "This is Server process with port : " << _portStr << endl;
  cout << "test begin***************************************"<<endl;
  testSortVector();
  cout << "test end*****************************************"<<endl;


	int port,
			server_socket,
			head_socket,
			selret,
			sock_index,
			fdaccept = 0,
			caddr_len;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	fd_set master_list;
	fd_set watch_list;

	//socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		std::cerr << "XueError: Cannot create socket" << '\n';
	}

	//fill up sockaddr_in struct
	port = atoi(_portStr.c_str());
	bzero(&server_addr,sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	//bind

	if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		std::cerr << "XueError: Bind failed" << '\n';
	}

	//listen
	if (listen(server_socket, BACKLOG) < 0) {
		std::cerr << "XueError: Unable to listen on port" << '\n';
	}

	/**************************************************************************/
	// Zero select FD sets
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);

	//Register the listening socket
	FD_SET(server_socket, &master_list);
	//Register STDIN
	FD_SET(STDIN, &master_list);

	head_socket = server_socket;

	while (TRUE) {
		memcpy(&watch_list, &master_list, sizeof(master_list));

		//select system call, BLOCK
		selret = select(head_socket+1, &watch_list, NULL, NULL, NULL);
		if (selret < 0) {
			std::cerr << "XueError: Select() failed." << '\n';
		}
		if (selret > 0) {
			//loop socket descriptors to check which one is ready
			for (sock_index=0; sock_index <= head_socket; sock_index++) {
				if (FD_ISSET(sock_index, &watch_list)) {
					//check if new command on STDIN
					if (sock_index == STDIN) {
						char *cmd = (char*) malloc(sizeof(char) * CMD_SIZE);
						memset(cmd, '\0', CMD_SIZE);
						if (fgets(cmd, CMD_SIZE-1, stdin) == NULL) {
							exit(-1);
						}
						std::cout << "I got command:" << cmd << endl;

						//todo: process PA1 commands here
						string tempCmd = string(cmd);
						parseCmd(tempCmd);

						free(cmd);
					}

					//check if new client is requesting connections, 新 client 连接
					else if (sock_index == server_socket) {
						caddr_len = sizeof(client_addr);
						fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&caddr_len);
						if (fdaccept < 0) {
							std::cerr << "XueError: Accept failed." << '\n';
						}
						std::cout << "Remote Host Connected." << '\n';

						//add to watched socket list
						FD_SET(fdaccept, &master_list);
						if (fdaccept > head_socket) {
							head_socket = fdaccept;
						}
					}

					//read from exiting Clients
					else{
						//initialize buffer to receive response
						char *buffer = (char*) malloc(sizeof(char) * BUFFER_SIZE);
						memset(buffer, '\0', BUFFER_SIZE);
						if (recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0) {
							close(sock_index);
							std::cout << "Remote Host's connection is terminated." << '\n';

							//remove from watched list
							FD_CLR(sock_index, &master_list);
						}
						else{
							//process incomming data from exiting clients
							std::cout << "Client sent me: " << buffer << endl;
							std::cout << "Echoing back to remote host ..." << endl;
							if (send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer)) {
								std::cout << "Done!" << endl;
							}
              //process request sent by existing sockets
              string requestStr = string(buffer);
              parseRequest(fdaccept, requestStr);

						}
						free(buffer);
					}
				}
			}
		}
	}
	return 1;
}

int Server::parseCmd(string cmd){
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
	else if (cmder == "STATISTICS") {assert(nToken == 1);onSTATISTICS();}
	else if (cmder == "BLOCKED") {assert(nToken == 2); onBLOCKED(token[1]);}
	else{std::cerr << "XueError: "<< cmder <<" | NO such commander!" << std::endl;}
}

int Server::parseRequest(int fdaccept, string requestStr){
  stringstream ss(requestStr);
  vector<string> tokens;
  string tkn;
  while (ss >> tkn) {
    tokens.push_back(tkn);
  }
  if (tokens.size() == 0) {
    exit(-1);
  }

  //Process requestStr
  if (tokens[0] == "LOGIN") {
    cout << "[Server::parseRequest] : Request is LOGIN, Processing..." << endl;
    //add to list
    assert(tokens.size() == 4);

    LoggedInListItemServer item = LoggedInListItemServer(tokens[1], tokens[2], atoi(tokens[3].c_str()));
    //需要查询一下是否已经有对应的项，re-login会出现的情况
    item.status = "logged-in";
    addLoggedInList(item);
    printLoggedInList();

    // sendMsgtoSocket(fdaccept, "Already Logged in!");
    //todo: send list and buffered messages
    //todo string 开始以 list 行数， buffered messages 条数。 以 | 分割， getline 可以识别



  }else if(tokens[0] == "LOGOUT"){
    assert(tokens.size() == 1);

    struct sockaddr_in *m_address;
    memset(&m_address, '\0', sizeof(m_address));
    int nAddrLen = sizeof(m_address);
    if(getpeername(fdaccept, (struct sockaddr*)&m_address, (socklen_t*)&nAddrLen) != 0)
    {
      printf("Get IP address by socket failed!n");
      return -1;
    }
    string clntIP = string(inet_ntoa(m_address->sin_addr));
    int iport = ntohs(m_address->sin_port);
    stringstream ss;
    ss << iport;
    string clntPort = ss.str();


    //set list of current client to LOGOUT
    for (size_t i = 0; i < loggedInList.size(); i++) {
      if (loggedInList.at(i).ip_addr == clntIP) {
        loggedInList.at(i).status = "logged-out";
      }
    }
    printLoggedInList();

  }else{
    cout << "[Server::parseRequest] : Request is not LOGIN or LOGOUT" << endl;
  }

}

int Server::printLoggedInList(){
  for (size_t i = 0; i < loggedInList.size(); i++) {
    LoggedInListItemServer t = loggedInList.at(i);
    cout << setw(2) <<i
        << " : " <<setw(30)<< t.hostname
        << " : " <<setw(15)<< t.ip_addr
        << " : " <<setw(5)<< t.port_num
        << " : " <<setw(3)<< t.num_msg_sent
        << " : " <<setw(3)<< t.num_msg_rcv
        << " : " <<setw(3)<< t.status
        <<endl;
  }
}

string Server::getMyHostName(){
	char *msg = (char*)malloc(MSG_SIZE);
	gethostname(msg, MSG_SIZE);
	return string(msg);
}
int Server::sendMsgtoSocket(int _socket, string msg){
	std::cout << "SENDing it to the remote server...";
	if (send(_socket, msg.c_str(), msg.length(), 0) == msg.length()) {
		std::cout << "Done!" << std::endl;
	}
	return 1;
}
string Server::recvMsgfromSocket(int _socket){
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

string Server::onAUTHOR(){
	string myUBIT = "zhenggan";
	return myUBIT;
	// printf("I, %s, have read and understood the course academic integrity policy.\n", myUBIT.c_str());
}
string Server::onIP(){
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
string Server::onPORT(){
	return selfPort;
}

string Server::onLIST(){}
string Server::onSTATISTICS(){}
string Server::onBLOCKED(string _clientIP){}
