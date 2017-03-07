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
#include <unistd.h>
#include <iomanip>


using namespace std;

void Client::testSortVector(){
    LoggedInListItemClient itemTemp = LoggedInListItemClient("euston.onlyone.edu", "34.1.1.1", 3456);
    loggedInList.push_back(itemTemp);
    for (int i = 0; i < loggedInList.size(); ++i) {
        cout << setw(2) <<i+1
             << " : " <<setw(30)<< loggedInList.at(i).hostname
             << " : " <<setw(15)<< loggedInList.at(i).ip_addr
             << " : " <<setw(5)<< loggedInList.at(i).port_num
             <<endl;
    }

}

int Client::start(){
    string _portStr = selfPort;
    cout << "This is Client process with port : " << _portStr << endl;
    cout << "test begin***************************************"<<endl;
    testSortVector();
    onLOGIN("128.205.36.46", "30000");
    cout << "test end*****************************************"<<endl;



    string msgIn;
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

        cout << "Please input commander: $";
        getline(cin, msgIn);

        //onLOGIN("128.205.36.46", "30000");
        parseCmd(msgIn);

    }
    return 1;
}

int Client::startNew(){
    string _portStr = selfPort;
    cout << "This is Client process with port : " << _portStr << endl;
    cout << "test begin***************************************"<<endl;
    testSortVector();
    onLOGIN("128.205.36.46", "30000");
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
                            std::cout << "[select]Remote sent me: " << buffer << endl;
                            string msgStr = string(buffer);

                        }
                        free(buffer);
                    }
                }
            }
        }
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
	string ret = string(msg);
	free(msg);
	return ret;
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
	}
	free(buffer);
  return ret;
}

string Client::recvMsgfromSocketWithLoop(int _socket){
    //initialize buffer to receive response
    char *buffer = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);
    string ret = "";
    int nCount;
    while( (nCount = recv(_socket, buffer, BUFFER_SIZE, 0)) > 0 ){
        ret = ret + string(buffer);
    }
    free(buffer);
    return ret;
}



vector<string> Client::splitString(string str){
    stringstream ss(str);
    vector<string> tokens;
    string tkn;
    while (ss >> tkn) {
        tokens.push_back(tkn);
    }
    return tokens;
}

int Client::parseCmd(string cmd){
	/**stringstream ss(cmd);
	string token[ARGN_MAX];
	int nToken = 0;
	while (ss >> token[nToken] && nToken < ARGN_MAX) {
		nToken++;
	}
	if (nToken == 0) {
		exit(-1);
	}*/

	stringstream ss(cmd);
	vector<string> tokens;
	string tkn;
	while (ss >> tkn) {
		tokens.push_back(tkn);
	}
	if (tokens.size() == 0) {
		exit(-1);
	}


	//Process cmds
	string cmder = tokens.at(0);
	int nToken = tokens.size();

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
        onLOGIN(tokens.at(1), tokens.at(2));
        cout << "Finish onLOGIN execution..."<<endl;
    }
    else if (cmder == "REFRESH") {
        assert(nToken == 1);
        onREFRESH();
        cout << "Finish onREFRESH execution..."<<endl;
    }
    else if (cmder == "SEND") {
        assert(nToken >= 3);
        string tmsg = tokens.at(2);
        for (int i = 3; i < nToken; ++i) {
            tmsg = tmsg + " " + tokens.at(i);
        }
        onSEND(tokens.at(1), tmsg);
    }
    else if (cmder == "BLOCK") {assert(nToken == 2); onBLOCK(tokens.at(1));}
    else if (cmder == "UNBLOCK") {assert(nToken == 2); onUNBLOCK(tokens.at(1));}
    else if (cmder == "LOGOUT") {
		cout << "Compare if cmder is LOGOUT..."<<endl;
		assert(nToken == 1);
		onLOGOUT();
		cout << "Finish LOGOUT execution..."<<endl;
	}
	else if (cmder == "EXIT") {assert(nToken == 1); onEXIT();}
	else{std::cerr << "XueError: "<< cmder <<" | NO such commander!" << std::endl;}
  return 1;
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

string Client::onLIST(){
    vector<LoggedInListItemClient> resultList = loggedInList;

    printf("[LIST:SUCCESS]\n");
    for (int j = 0; j < resultList.size(); ++j) {
        LoggedInListItemClient t = resultList.at(j);
        cout << setw(2) <<j+1
             << " : " <<setw(30)<< t.hostname
             << " : " <<setw(15)<< t.ip_addr
             << " : " <<setw(5)<< t.port_num
             <<endl;
    }
    printf("[LIST:END]\n");
    return "list";
}

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

    string t = recvMsgfromSocket(serverSocket);
    cout <<"Recv msg is : "<< t <<endl;


    //接收返回的list
    t = recvMsgfromSocket(serverSocket);
    vector<string> tokens = splitString(t);
    vector<LoggedInListItemClient> vlist;

    assert(tokens.size()%3 == 0);

    for (int j = 0; j < tokens.size(); j=j+3) {
        LoggedInListItemClient temp_item = LoggedInListItemClient(tokens.at(j+0),tokens.at(j+1),atoi(tokens.at(j+2).c_str()));
        vlist.push_back(temp_item);
    }
    loggedInList = vlist;
    sendMsgtoSocket(serverSocket, "ACK");


    //接收返回的buffered messages
    string tmp_msg;
    while ((tmp_msg = recvMsgfromSocket(serverSocket)) != "END"){
        cout << "I got buffered message: " << tmp_msg << endl;
        sendMsgtoSocket(serverSocket, "ACK");
    }

    cout << "Finish Loggin Step, you can check \"LIST\"." <<endl;
	return "loggedin"; //can be ignored
}

string Client::onREFRESH(){
    string request = string("REFRESH");
    sendMsgtoSocket(serverSocket, request);

    string t = recvMsgfromSocket(serverSocket);
    cout <<"Recv msg is : "<< t <<endl;

    t = recvMsgfromSocket(serverSocket);
    vector<string> tokens = splitString(t);
    vector<LoggedInListItemClient> vlist;

    assert(tokens.size()%3 == 0);

    for (int j = 0; j < tokens.size(); j=j+3) {
        LoggedInListItemClient temp_item = LoggedInListItemClient(tokens.at(j+0),tokens.at(j+1),atoi(tokens.at(j+2).c_str()));
        vlist.push_back(temp_item);
    }
    loggedInList = vlist;

    cout << "Finish Refresh Step, you can check \"LIST\"." << endl;
    return "refresh";
}

string Client::onSEND(string _clientIP, string _msg){
    string request = string("SEND");

    request = request + " " + _clientIP + " " + _msg;
    //request = "SEND 128.205.36.35 sendmessagetest if you can see this in client machine, SEND does well.";

    sendMsgtoSocket(serverSocket, request);

    //this is for "echo"
    string t = recvMsgfromSocket(serverSocket);
    cout <<"Recv msg is : "<< t <<endl;

    return "send";

}

string Client::onBLOCK(string _clientIP){}

string Client::onUNBLOCK(string _clientIP){}

string Client::onLOGOUT(){
	cout << "I'm in onLOGOUT, and processing with serverSocket: "<<serverSocket << endl;
    assert(serverSocket >= 0);
    string request = string("LOGOUT");
    request = request + " " + selfIP;

    sendMsgtoSocket(serverSocket, request);

	string t = recvMsgfromSocket(serverSocket);
	cout <<"Recv msg is : "<< t <<endl;

	return "logout";  //segmentation fault solved. add this return statement.
}
string Client::onEXIT(){}
