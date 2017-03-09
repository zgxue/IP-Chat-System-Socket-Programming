#include "../include/Server.h"
#include "../include/global.h"
#include "../include/logger.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
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

    printLoggedInList();

    sort(loggedInList.begin(), loggedInList.end());
    cout << "After Sorted:>>" << endl;
    printLoggedInList();

    rmLoggedInList("35");
    cout << "After remove 35:>>" << endl;
    printLoggedInList();

}

int Server::addLoggedInList(LoggedInListItemServer item){
  //need to deal with the client which is not new but logged-out
  // return 1 : add
  // return 2 : update

  int listSize = loggedInList.size();
  for (int i = 0; i < listSize; i++) {
    if (loggedInList.at(i).hostname == item.hostname) {
      loggedInList.at(i).status = "logged-in";
      return 2;
    }
  }

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
int Server::addBlockList(string blockingIP, string blockedIP){
    size_t i = 0;
    for (i = 0; i < loggedInList.size(); i++) {
        LoggedInListItemServer lilis = loggedInList.at(i);
        if (lilis.ip_addr == blockingIP) {
            vector<string> blist = lilis.blockedList;
            int j = 0;
            for (j = 0; j < blist.size(); ++j) {
                if(blist.at(j) == blockedIP){
                    break;
                }
            }
            if (j == blist.size()){ //没有再次 block，添加新的
                loggedInList.at(i).blockedList.push_back(blockedIP);
            }
            break;
        }
    }
    if (i == loggedInList.size()){
        return -1; //means that blockingIP is not existed in loggedInList
    }
    return 1;
}
int Server::rmBlockList(string unblockingIP, string unblockedIP){
    size_t i = 0;
    for (i = 0; i < loggedInList.size(); i++) {
        LoggedInListItemServer lilis = loggedInList.at(i);
        if (lilis.ip_addr == unblockingIP) {
            vector<string> blist = lilis.blockedList;
            int j = 0;
            for (j = 0; j < blist.size(); ++j) {
                if(blist.at(j) == unblockedIP){
                    loggedInList.at(i).blockedList.erase(loggedInList.at(i).blockedList.begin() + i);
                    break;
                }
            }
            break;
        }
    }
    if (i == loggedInList.size()){
        return -1; //means that blockingIP is not existed in loggedInList
    }
    return 1;
}
int Server::isClientBeenBlocked(string blockedBy, string beBlocked){
    // -1 means no such blockedBy IP in the loggedInList
    // 0 means not be blocked
    // 1 means be blocked
    int ret = -100;
    int i = 0;
    for (i = 0; i < loggedInList.size(); ++i) {
        if(loggedInList.at(i).ip_addr == blockedBy){
            int j = 0;
            for (j = 0; j < loggedInList.at(i).blockedList.size(); ++j) {
                if(loggedInList.at(i).blockedList.at(j) == beBlocked){
                    return 1;
                }
            }
            return 0;
        }
    }
    return -1;
}
int Server::isClientLoggedIn(string clientIP){
    // -1 means clientIP doesn't exist
    // 0 means logged-out
    // 1 means logged-in
    int ret = -100;
    int i =0;
    for (i = 0; i < loggedInList.size(); ++i) {
        if(loggedInList.at(i).ip_addr == clientIP){
            if(loggedInList.at(i).status == "logged-in"){
                ret = 1;
            }else if(loggedInList.at(i).status == "logged-out"){
                ret = 0;
            }
            break;
        }
    }
    if(i == loggedInList.size()){
        ret = -1;
    }

    return ret;
}
int Server::findIndexOfIpInLIST(string clientIP){
    // -1 means no such IP exists in LIST
    // >= 0 the index
    int ret = -1;
    for (int i = 0; i < loggedInList.size(); ++i) {
        if(loggedInList.at(i).ip_addr == clientIP){
            ret = i;
            break;
        }
    }
    return ret;

}
string Server::findClientIPfromSocket(int clientSocket){
    struct sockaddr_in c;
    socklen_t cLen = sizeof(c);
    getpeername(clientSocket, (struct sockaddr*) &c, &cLen);
    string result =  inet_ntoa(c.sin_addr);
    return result;
}
void Server::errorLogPrint(string cmder){
    cse4589_print_and_log("[%s:ERROR]\n",cmder.c_str());
    cse4589_print_and_log("[%s:END]\n", cmder.c_str());
}
string Server::getHeaderOfString(string str){

    int ind = str.find_first_of(" ");
    if (str.length() > 0 && ind == string::npos) {
        return str;
    }
    if(ind >= 0){
        string Header = str.substr(0, ind);
        return Header;
    }
    return "";
}
string Server::getRestAfterRMHeader(string str){
    int ind = str.find_first_of(" ");
    if (ind >= 0 && str.length() > ind+1 ){
        string buffstr = str.substr(ind + 1);
        return buffstr;
    }
    return "";
}
int Server::getNumOfSegmentsOfString(string str){
    if (str == ""){
        return 0;
    }
    return count(str.begin(), str.end(), ' ') + 1;
}





int Server::start(){
	string _portStr = selfPort;

	cout << "This is Server process with port : " << _portStr << endl;
  cout << "test begin***************************************"<<endl;
  //testSortVector();
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
//						std::cout << "I got command:" << cmd << endl;

						//todo: process PA1 commands here
						string tempCmd = string(cmd);
                        if (tempCmd.length() != 0 && (tempCmd[tempCmd.length()-1] == '\n')) { //去掉回车
                            tempCmd = tempCmd.substr(0, tempCmd.length()-1);
                        }
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
                            //process request sent by existing sockets
                            string requestStr = string(buffer);
//                            parseRequest(fdaccept, requestStr);
                            parseRequest(sock_index, requestStr);

						}
						free(buffer);
					}
				}
			}
		}
	}
	return 1;
}

int Server::connect_to_host(string server_ip, int server_port){
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

int Server::parseCmd(string cmd){
    //Process cmds
    string cmder = getHeaderOfString(cmd);
    string restcmd = getRestAfterRMHeader(cmd);
    int nToken = getNumOfSegmentsOfString(cmd);  //至少有的空格数目 + 1

	if (cmder == "AUTHOR") {
        if(nToken != 1){ errorLogPrint(cmder);return 0; }
		onAUTHOR();
	}
	else if (cmder == "IP") {
        if(nToken != 1){ errorLogPrint(cmder);return 0; }
        onIP();
	}
	else if (cmder == "PORT") {
        if(nToken != 1){ errorLogPrint(cmder);return 0; }
		onPORT();
	}

	else if (cmder == "LIST") {
        if(nToken != 1){ errorLogPrint(cmder);return 0; }
        onLIST();
    }
	else if (cmder == "STATISTICS") {
        if(nToken != 1){ errorLogPrint(cmder);return 0; }
        onSTATISTICS();
    }
	else if (cmder == "BLOCKED") {
        if(nToken != 2){ errorLogPrint(cmder);return 0; }
        string ip = getHeaderOfString(restcmd);
        onBLOCKED(ip);
    }
	else{
        std::cerr << "[XueError:]"<<"NO such commander!"<< cmder<< std::endl;
        errorLogPrint(cmder);
        return 0;
    }
    return 1;
}

int Server::parseRequest(int fdaccept, string requestStr){

    string rqstCmder = getHeaderOfString(requestStr);
    string restcmd = getRestAfterRMHeader(requestStr);
    int nTokens = getNumOfSegmentsOfString(requestStr);  //至少有的空格数目 + 1

    //Process requestStr
    //***********************************************************************************
    if (rqstCmder == "LOGIN") {
        string firstPara = getHeaderOfString(restcmd);
        string firstStr = getRestAfterRMHeader(restcmd);
        string secondPara = getHeaderOfString(firstStr);
        string secondStr = getRestAfterRMHeader(firstStr);
        string thirdPara = getHeaderOfString(secondStr);

        LoggedInListItemServer item = LoggedInListItemServer(firstPara, secondPara, atoi(thirdPara.c_str()));
        //需要查询一下是否已经有对应的项，re-login会出现的情况
        item.status = "logged-in";
        item.socketNumber = fdaccept;

        addLoggedInList(item);
        //printLoggedInList();

        vector<LoggedInListItemClient> tlist = getListForClient();


        string strsend;
        for (int i = 0; i < tlist.size(); ++i) {
            stringstream ss;
            ss << tlist.at(i).port_num;
            string strport_num = ss.str();

            strsend = strsend
                      + " " + tlist.at(i).hostname
                      + " " + tlist.at(i).ip_addr
                      + " " + strport_num;
        }
        sendMsgtoSocket(fdaccept,strsend);
        cout << "just sent : "<<strsend<<endl;

        cout << "just receive: " << recvMsgfromSocket(fdaccept) <<endl; // == "ACK");


        string strRcv;

        int ind = findIndexOfIpInLIST(findClientIPfromSocket(fdaccept));
        if(ind >= 0){
            for (int j = 0; j < loggedInList.at(ind).bufferdMessages.size(); ++j) {
                string header = "MSG";
                string t = header + loggedInList.at(ind).bufferdMessages.at(j).first
                        +" " + loggedInList.at(ind).bufferdMessages.at(j).second;
                sendMsgtoSocket(fdaccept, t);
                recvMsgfromSocket(fdaccept);  //wait "ACK"

                cse4589_print_and_log("[RELAYED:SUCCESS]\n");
                cse4589_print_and_log("msg from:%s, to:%s\n",
                                      loggedInList.at(ind).bufferdMessages.at(j).first.c_str(),
                                      secondPara.c_str());
                cse4589_print_and_log("[msg]:%s\n",loggedInList.at(ind).bufferdMessages.at(j).second.c_str());
                cse4589_print_and_log("[RELAYED:END]\n");
            }
            sendMsgtoSocket(fdaccept,"END");

            //完成后清空
            loggedInList.at(ind).bufferdMessages.clear();
        }
        //printLoggedInList();

      //***********************************************************************************
    }else if(rqstCmder == "LOGOUT"){

        //set list of current client to LOGOUT
        string clntIP = getHeaderOfString(restcmd);
        for (size_t i = 0; i < loggedInList.size(); i++) {
            if(loggedInList.at(i).ip_addr == clntIP) {
                loggedInList.at(i).status = "logged-out";
                loggedInList.at(i).socketNumber = -1;
                break;
            }
        }
        //printLoggedInList();

        //***********************************************************************************
    }else if(rqstCmder == "REFRESH"){
        vector<LoggedInListItemClient> tlist = getListForClient();
        string strsend = "";

        for (int i = 0; i < tlist.size(); ++i) {
            stringstream ss;
            ss << tlist.at(i).port_num;
            string strport_num = ss.str();

            strsend = strsend
                      + " " + tlist.at(i).hostname
                      + " " + tlist.at(i).ip_addr
                      + " " + strport_num;
        }
        sendMsgtoSocket(fdaccept,strsend);
        recvMsgfromSocket(fdaccept); // == "ACK");

        //***********************************************************************************
    }else if(rqstCmder == "SEND"){
        string toClientIP = getHeaderOfString(restcmd);
        string msgSend = getRestAfterRMHeader(restcmd);
        inSEND(fdaccept, toClientIP, msgSend);
/*
 * cout << "[Server::parseRequest] : Request is SEND, Processing..." << endl;
        string toClientIP = getHeaderOfString(restcmd);
        string strMsg = getRestAfterRMHeader(restcmd);


        int toClientPort = 0;
        string fromClientIP;

        int listSize = loggedInList.size();
        int toClientSocket = -100;
        int indexOftoClientInList = -1;
        int indexOffromClientInList = -1;

        //getpeerip, get fromClientIP
        struct sockaddr_in c;
        socklen_t cLen = sizeof(c);
        getpeername(fdaccept, (struct sockaddr*) &c, &cLen);
        printf("[Send] fromClient: %s\n", inet_ntoa(c.sin_addr));
        fromClientIP =  string(inet_ntoa(c.sin_addr));


        //modify the fromClient's msg_send
        for (int i = 0; i < listSize; i++) {
            if (loggedInList.at(i).ip_addr == fromClientIP) {
                indexOffromClientInList = i;
            }
        }

        loggedInList.at(indexOffromClientInList).num_msg_sent += 1;


        //find socketNumber from list
        for (int i = 0; i < listSize; i++) {
            if (loggedInList.at(i).ip_addr == toClientIP) {
                toClientSocket = loggedInList.at(i).socketNumber;
                toClientPort = loggedInList.at(i).port_num;
                indexOftoClientInList = i;
                break;
            }
        }

        //根据 socket 来决定 action
        if(toClientSocket > 0){
            if (isClientBeenBlocked(toClientIP, fromClientIP) == 0){

                //assert(toClientPort != 0);
                int newSocketToClient = connect_to_host(toClientIP, toClientPort);
                sendMsgtoSocket(newSocketToClient, fromClientIP + " " + strMsg);
                loggedInList.at(indexOftoClientInList).num_msg_rcv += 1;
                close(newSocketToClient);

                cse4589_print_and_log("[RELAYED:SUCCESS]\n");
                cse4589_print_and_log("msg from:%s, to:%s\n", fromClientIP.c_str(), toClientIP.c_str());
                cse4589_print_and_log("[msg]:%s\n",strMsg.c_str());
                cse4589_print_and_log("[RELAYED:END]\n");
            }

        }
        if(toClientSocket == -1){

            cout << "Here should buffer this strMsg for toClientIP, and store it in the loggedInList" <<endl;
//            assert(indexOftoClientInList != -1);
            pair<string, string> tbuf = make_pair(fromClientIP, strMsg);
            loggedInList.at(indexOftoClientInList).bufferdMessages.push_back(tbuf);

        }

        printLoggedInList();
*/

        //**************************************************************************************************
    }else if(rqstCmder == "BLOCK"){

//        cout << "start parsing request and compare it to BLOCK, nTokens is(should be 2): " << nTokens<< endl;
//        assert(nTokens == 2);

        string blockingIP;
        string blockedIP;

        //get blocking IP an blockedIP
        struct sockaddr_in c;
        socklen_t cLen = sizeof(c);
        getpeername(fdaccept, (struct sockaddr*) &c, &cLen);
        printf("[Send] fromClient: %s\n", inet_ntoa(c.sin_addr));
        blockingIP =  inet_ntoa(c.sin_addr);
        blockedIP = getHeaderOfString(restcmd);
        addBlockList(blockingIP, blockedIP);

        //**************************************************************************************************
    }else if(rqstCmder == "UNBLOCK"){
//        cout << "start parsing request and compare it to UNBLOCK, nTokens is(should be 2): " << nTokens<< endl;
//        assert(nTokens == 2);

        string unblockingIP;
        string unblockedIP;

        //get blocking IP an blockedIP
        struct sockaddr_in c;
        socklen_t cLen = sizeof(c);
        getpeername(fdaccept, (struct sockaddr*) &c, &cLen);
//        printf("[Send] fromClient: %s\n", inet_ntoa(c.sin_addr));

        unblockingIP =  inet_ntoa(c.sin_addr);
        unblockedIP = getHeaderOfString(restcmd);
        rmBlockList(unblockingIP, unblockedIP);

        //**************************************************************************************************
    }else if (rqstCmder == "BROADCAST"){
        string strBroadcast = restcmd;

        inBROADCAST(fdaccept,strBroadcast);

        //**************************************************************************************************
    } else if(rqstCmder == "EXIT"){
//        assert(nTokens == 1);
        inEXIT(fdaccept);
    }
    else{
        cout << "[Server::parseRequest] : Request is not LOGIN or LOGOUT or REFRESH or SEND or BLOCK or Broadcast or exit" << endl;
    }
    return 1;

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
        << " : " <<setw(5)<< t.socketNumber
        <<endl;
  }
}

vector<LoggedInListItemClient> Server::getListForClient(){
    vector<LoggedInListItemClient> resultList;

    for (size_t i = 0; i < loggedInList.size(); i++) {
        if(loggedInList.at(i).status != "logged-in"){
            continue;
        }

        LoggedInListItemServer ts = loggedInList.at(i);
        LoggedInListItemClient tc = LoggedInListItemClient(ts.hostname, ts.ip_addr, ts.port_num);
        resultList.push_back(tc);
    }
    return resultList;
}


string Server::getMyHostName(){
	char *msg = (char*)malloc(MSG_SIZE);
	gethostname(msg, MSG_SIZE);
  string ret = string(msg);
  free(msg);
	return ret;
}
string Server::getMyIP(){
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



int Server::sendMsgtoSocket(int _socket, string msg){
	if (send(_socket, msg.c_str(), msg.length(), 0) == msg.length()) {
		return 1;
	}
	return 0;
}
string Server::recvMsgfromSocket(int _socket){
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

string Server::onAUTHOR(){
	string myUBIT = "zhenggan";
    cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
    cse4589_print_and_log("I, zhenggan, have read and understood the course academic integrity policy.\n");
    cse4589_print_and_log("[AUTHOR:END]\n");
	return myUBIT;
}
string Server::onIP(){

    if(selfIP != ""){
        cse4589_print_and_log("[IP:SUCCESS]\n");
        cse4589_print_and_log("IP:%s\n",selfIP.c_str());
        cse4589_print_and_log("[IP:END]\n");
    }else{
        cse4589_print_and_log("[IP:ERROR]\n");
        cse4589_print_and_log("[IP:END]\n");
    }
    return selfIP;
}
string Server::onPORT(){
    if(selfPort != ""){
        cse4589_print_and_log("[PORT:SUCCESS]\n");
        cse4589_print_and_log("PORT:%s\n",selfPort.c_str());
        cse4589_print_and_log("[PORT:END]\n");
    }else{
        cse4589_print_and_log("[PORT:ERROR]\n");
        cse4589_print_and_log("[PORT:END]\n");
    }
    return selfPort;
}

string Server::onLIST(){

    vector<LoggedInListItemClient> resultList = getListForClient();

    cse4589_print_and_log("[LIST:SUCCESS]\n");
    for (int j = 0; j < resultList.size(); ++j) {
        LoggedInListItemClient t = resultList.at(j);
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", j+1, t.hostname.c_str(), t.ip_addr.c_str(), t.port_num);
    }
    cse4589_print_and_log("[LIST:END]\n");

    return "list";
}
string Server::onSTATISTICS(){

    vector<LoggedInListItemServer> resultList = loggedInList;
    cse4589_print_and_log("[STATISTICS:SUCCESS]\n");
    for (int j = 0; j < resultList.size(); ++j) {
        LoggedInListItemServer t = resultList.at(j);
        cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", j+1, t.hostname.c_str(), t.num_msg_sent, t.num_msg_rcv, t.status.c_str());
    }

    cse4589_print_and_log("[STATISTICS:END]\n");
    return "statistics";
}
string Server::onBLOCKED(string _clientIP){

    vector<string> blockedlist;
    int i = 0;
    for (i = 0; i < loggedInList.size(); ++i) {
        LoggedInListItemServer lllis = loggedInList.at(i);
        if(lllis.ip_addr == _clientIP){
            blockedlist = lllis.blockedList;
            break;
        }
    }

    if(i == loggedInList.size()){
        cerr << "[blocked] there is no match record given the clientIP." <<endl;
        errorLogPrint("BLOCKED");
        return "NoMatch";
    }

    //针对blockedlist 找到对应的 index， 排序后输出正确的顺序
    vector<int> indexSet;
    for (int j = 0; j < blockedlist.size(); ++j) {
        int ind = findIndexOfIpInLIST(blockedlist.at(j));
        if (ind >= 0){  //这里有预防性拉黑的策略，可以在一个 ip 还没有 loggin 的时候就在 blocklist 存储。但proj1要求的是返回 list 中有的。满足它。
            indexSet.push_back(ind);
        }
    }
    sort(indexSet.begin(), indexSet.end());

    cse4589_print_and_log("[BLOCKED:SUCCESS]\n");
//    for (int j = 0; j < blockedlist.size(); ++j) {
//        cout << blockedlist.at(j) << endl;
//    }
    for (int k = 0; k < indexSet.size(); ++k) {
        LoggedInListItemServer t = loggedInList.at(indexSet.at(k));
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", k+1, t.hostname.c_str(), t.ip_addr.c_str(), t.port_num);
    }
    cse4589_print_and_log("[BLOCKED:END]\n");

    return "blocked";
}

string Server::inSEND(int fromClientSocket, string _toClientIP, string msgSend){
    string fromClientIP;
    string iterIP;

    //get fromClientIP
    fromClientIP = findClientIPfromSocket(fromClientSocket);

    //给 fromClient 的 msg sent number +1
    int indexFrom = findIndexOfIpInLIST(fromClientIP);

    assert(indexFrom >= 0);
    loggedInList.at(indexFrom).num_msg_sent += 1;

    //对 loggedInList 里面每一个client 遍历处理, 不包括自己
    int i = 0;
    for (i = 0; i < loggedInList.size(); ++i) {
        iterIP = loggedInList.at(i).ip_addr;

        //找到目标 ip
        if(iterIP == _toClientIP){
            if(loggedInList.at(i).status == "logged-in"){
                if (isClientBeenBlocked(_toClientIP, fromClientIP) == 0){
                    int newSocketToClient = connect_to_host(_toClientIP, loggedInList.at(i).port_num);
                    string header = "MSG";
                    string strSend = header + " " + fromClientIP + " " + msgSend;
                    sendMsgtoSocket(newSocketToClient, strSend);
                    loggedInList.at(i).num_msg_rcv += 1;
                    close(newSocketToClient);

                    cse4589_print_and_log("[RELAYED:SUCCESS]\n");
                    cse4589_print_and_log("msg from:%s, to:%s\n", fromClientIP.c_str(), _toClientIP.c_str());
                    cse4589_print_and_log("[msg]:%s\n",msgSend.c_str());
                    cse4589_print_and_log("[RELAYED:END]\n");
                }else{
                    cout << "[inSend] the client has been blocked!";  //ignored if blocked
                }

            }else{ //if logged-out, buffered the messages
                pair<string, string> tbuf = make_pair(fromClientIP, msgSend);
                loggedInList.at(i).bufferdMessages.push_back(tbuf);
            }
            break;
        }
    }
    if (i == loggedInList.size()){
        return "toClientExited";
    }

    return "broadcast";

}

string Server::inBROADCAST(int fromClientSocket, string msgBroadcast){

    string fromClientIP;
    string toClientIP;

    //get fromClientIP
    fromClientIP = findClientIPfromSocket(fromClientSocket);
    cout << "[inBROADCAST] fromClientIP is:"<<fromClientIP <<"fromClientSocket:"<<fromClientSocket<<endl;

    //给 fromClient 的 msg sent number +1
    int indexFrom = findIndexOfIpInLIST(fromClientIP);
    cout << "[inBROADCAST] index of fromClientIP is:"<<indexFrom<<endl;


    assert(indexFrom >= 0);
    loggedInList.at(indexFrom).num_msg_sent += 1;


    //对 loggedInList 里面每一个client 遍历处理, 不包括自己
    for (int i = 0; i < loggedInList.size(); ++i) {
        toClientIP = loggedInList.at(i).ip_addr;

        //跳过自己
        if(toClientIP == fromClientIP){
            continue;
        }

        if(loggedInList.at(i).status == "logged-in"){
            if (isClientBeenBlocked(toClientIP, fromClientIP) == 0){
                int newSocketToClient = connect_to_host(toClientIP, loggedInList.at(i).port_num);
                string header = "MSG";
                string strSend = header + " " + fromClientIP + " " + msgBroadcast;
                sendMsgtoSocket(newSocketToClient, strSend);
                loggedInList.at(i).num_msg_rcv += 1;
                close(newSocketToClient);
            }else{
                cout << "[broadcast] the client has been blocked!";  //ignored if blocked
            }

        }else{ //if logged-out, buffered the messages
            pair<string, string> tbuf = make_pair(fromClientIP, msgBroadcast);
            loggedInList.at(i).bufferdMessages.push_back(tbuf);
        }
    }
    cse4589_print_and_log("[RELAYED:SUCCESS]\n");
    cse4589_print_and_log("msg from:%s, to:255.255.255.255\n", fromClientIP.c_str());
    cse4589_print_and_log("[msg]:%s\n",msgBroadcast.c_str());
    cse4589_print_and_log("[RELAYED:END]\n");

    return "broadcast";
}

string Server::inEXIT(int fromClientSocket){
    string fromClientIP = findClientIPfromSocket(fromClientSocket);
    int index = findIndexOfIpInLIST(fromClientIP);
    if(index >= 0){
        loggedInList.erase(loggedInList.begin()+index);
    }
    return "exit";
}
