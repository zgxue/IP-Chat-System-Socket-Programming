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
#include <algorithm>



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
    //testSortVector();
    cout << "myIP: "<<selfIP<< endl << "myPort:"<< selfPort;

//    onLOGIN("128.205.36.46", "30000");  //一开始是这个导致author 通过不，应为 client 登陆会出问题，server 的 port 是 grader 随机指定的。
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
//                            std::cout << "[select]Remote sent me: " << buffer << endl;
                            string msgStr = string(buffer);

                            cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
                            cse4589_print_and_log("msg from:%s\n", getHeaderOfString(msgStr).c_str());
                            cse4589_print_and_log("[msg]:%s\n",getRestAfterRMHeader(msgStr).c_str());
                            cse4589_print_and_log("[RECEIVED:END]\n");

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
string Client::getMyIP(){
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
bool Client::isIPinLoggedInList(string ip){
    vector<LoggedInListItemClient> cacheList = loggedInList;
    for (int i = 0; i < cacheList.size(); ++i) {
        if(cacheList.at(i).ip_addr == ip){
            return true;
        }
    }
    return false;
}
bool Client::isIPinBlockedList(string ip){
    vector<string> cacheList = Gblockedlist;
    for (int i = 0; i < cacheList.size(); ++i) {
        if(cacheList.at(i) == ip){
            return true;
        }
    }
    return false;
}
bool Client::isValidIP(string ip){
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
    if(result == 0){
        return false;
    }
    return true;

/*
    vector<int> tokens;
    char str[20];
    for(int i=0;i<20;i++){
        str[i]=ip.c_str()[i];
    }
    char *pch;
    pch = strtok(str, ".");
    while (pch != NULL){
        tokens.push_back(atoi(pch));
        pch = strtok(NULL, ".");
    }
    if (tokens.size() != 4){
        return false;
    }
    for (int j = 0; j < tokens.size(); ++j) {
        ;
    }
*/
}
bool Client::isValidPort(string port){
    int result = atoi(port.c_str());

    if (result >= 1 && result <= 65535){
        return true;
    }
    return false;
}

int Client::sendMsgtoSocket(int _socket, string msg){
	if (send(_socket, msg.c_str(), msg.length(), 0) == msg.length()) {
        return 1;
    }
	return 0;
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
void Client::errorLogPrint(string cmder){
    cse4589_print_and_log("[%s:ERROR]\n",cmder.c_str());
    cse4589_print_and_log("[%s:END]\n", cmder.c_str());
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
string Client::getHeaderOfString(string str){

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
string Client::getRestAfterRMHeader(string str){
    int ind = str.find_first_of(" ");
    if (ind >= 0 && str.length() > ind+1 ){
        string buffstr = str.substr(ind + 1);
        return buffstr;
    }
    return "";
}
int Client::getNumOfSegmentsOfString(string str){
    if (str == ""){
        return 0;
    }
    return count(str.begin(), str.end(), ' ') + 1;
}


int Client::parseCmd(string cmd){

	//Process cmds
	string cmder = getHeaderOfString(cmd);
    string restcmd = getRestAfterRMHeader(cmd);
	int nToken = getNumOfSegmentsOfString(cmd);  //至少有的空格数目 + 1

//    printf("[parseCmd]%s\n", cmd.c_str());
//    printf("[parseCmd]%s, %s, %d\n", cmder.c_str(), restcmd.c_str(), nToken);

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
        if(!logStatus || nToken != 1){ errorLogPrint(cmder);return 0; }
        onLIST();
    }
	else if (cmder == "LOGIN") {
        if(nToken != 3){ errorLogPrint(cmder);return 0; }

        string ip = getHeaderOfString(restcmd);
        string rest = getRestAfterRMHeader(restcmd);
        string port = getHeaderOfString(rest);

        onLOGIN(ip, port);
    }
    else if (cmder == "REFRESH") {
        if(!logStatus || nToken != 1){ errorLogPrint(cmder);return 0; }
        onREFRESH();
    }
    else if (cmder == "SEND") {
        if(!logStatus || nToken < 3){ errorLogPrint(cmder);return 0; }

        string ip = getHeaderOfString(restcmd);
        string tmsg = getRestAfterRMHeader(restcmd);

//        printf("[parseCmd] after Send: ip=%s, tmsg=%s\n",ip.c_str(), tmsg.c_str());

        onSEND(ip, tmsg);
    }
    else if (cmder == "BLOCK") {
        if(!logStatus || nToken != 2){ errorLogPrint(cmder);return 0; }
        string ip = getHeaderOfString(restcmd);
        onBLOCK(ip);
    }
    else if (cmder == "UNBLOCK") {
        if(!logStatus || nToken != 2){ errorLogPrint(cmder);return 0; }
        string ip = getHeaderOfString(restcmd);
        onUNBLOCK(ip);
    }
    else if (cmder == "LOGOUT") {
        if(!logStatus || nToken != 1){ errorLogPrint(cmder);return 0; }
		onLOGOUT();
	}
	else if (cmder == "EXIT") {
        if(nToken != 1){ errorLogPrint(cmder);return 0; }
        onEXIT();
    }
    else if(cmder == "BROADCAST"){
        if(!logStatus || nToken < 2){ errorLogPrint(cmder);return 0; }
        onBROADCAST(restcmd);
    }
	else{
        std::cerr << "XueError: "<< cmder <<" | NO such commander!" << std::endl;
        errorLogPrint(cmder);
        return 0;
    }
  return 1;
}

string Client::onAUTHOR(){
    cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
    cse4589_print_and_log("I, zhenggan, have read and understood the course academic integrity policy.\n");
    cse4589_print_and_log("[AUTHOR:END]\n");
    return "zhenggan";
}
string Client::onIP(){
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
string Client::onPORT(){
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

string Client::onLIST(){

    vector<LoggedInListItemClient> resultList = loggedInList;

    cse4589_print_and_log("[LIST:SUCCESS]\n");
    for (int j = 0; j < resultList.size(); ++j) {
        LoggedInListItemClient t = resultList.at(j);
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", j+1, t.hostname.c_str(), t.ip_addr.c_str(), t.port_num);
    }
    cse4589_print_and_log("[LIST:END]\n");

    return "list";
}

string Client::onLOGIN(string _serverIP, string _serverPort){
    //identify themselves to the server
    //get the list of other-logged in clients
    //get bufferd messages
    /**
    * 1, 发送自己的信息给 server处理，2， 接收 server 发来的信息并处理。
    */
    if(!isValidIP(_serverIP) || !isValidPort(_serverPort)){
        errorLogPrint("LOGIN");
        return "exception";
    }

    serverSocket = connect_to_host(_serverIP, atoi(_serverPort.c_str()));
    server_ip = _serverIP;
    server_port = _serverPort;

    string request = string("LOGIN") +" "+ selfHostName +" "+ selfIP +" "+ selfPort;
    sendMsgtoSocket(serverSocket, request);


    //接收返回的list
    string t = recvMsgfromSocket(serverSocket);
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
//        cout << "I got buffered message: " << tmp_msg << endl;

        string msgfrom = getHeaderOfString(tmp_msg);
        string buffstr = getRestAfterRMHeader(tmp_msg);

        cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
        cse4589_print_and_log("msg from:%s\n",msgfrom.c_str());
        cse4589_print_and_log("[msg]:%s\n",buffstr.c_str());
        cse4589_print_and_log("[RECEIVED:END]\n");

        sendMsgtoSocket(serverSocket, "ACK");
    }
    logStatus = true;

    cse4589_print_and_log("[LOGIN:SUCCESS]\n");
    cse4589_print_and_log("[LOGIN:END]\n");

    cout << "Finish Loggin Step, you can check \"LIST\"." <<endl;
	return "loggedin"; //can be ignored
}

string Client::onREFRESH(){
    string request = string("REFRESH");
    sendMsgtoSocket(serverSocket, request);

    string t = recvMsgfromSocket(serverSocket);
    vector<string> tokens = splitString(t);
    vector<LoggedInListItemClient> vlist;

    assert(tokens.size()%3 == 0);

    for (int j = 0; j < tokens.size(); j=j+3) {
        LoggedInListItemClient temp_item = LoggedInListItemClient(tokens.at(j+0),tokens.at(j+1),atoi(tokens.at(j+2).c_str()));
        vlist.push_back(temp_item);
    }
    loggedInList = vlist;

    sendMsgtoSocket(serverSocket, "ACK");

    cout << "Finish Refresh Step, you can check \"LIST\". The new loggedInlist has size: " <<vlist.size()<< endl;
    return "refresh";
}

string Client::onSEND(string _clientIP, string _msg){
    if(!isValidIP(_clientIP) || !isIPinLoggedInList(_clientIP)){
//        cout << "isValidIP:"<<isValidIP(_clientIP) <<endl;
//        cout << "isIPinLoggedInList:"<<isIPinLoggedInList(_clientIP)<<endl;
        errorLogPrint("SEND");
        return "exception";
    }

    string request = string("SEND");
    request = request + " " + _clientIP + " " + _msg;
    sendMsgtoSocket(serverSocket, request);

    cse4589_print_and_log("[SEND:SUCCESS]\n");
    cse4589_print_and_log("[SEND:END]\n");

    return "send";
}

string Client::onBLOCK(string _clientIP){
    cout << "I'm in BLOCK, and processing with serverSocket: "<<serverSocket << endl;

    //exceptions, 1, 是否 clietnip 在本地的 loggedinlist 中 2, 是否已经 blocked 了
    if(!isValidIP(_clientIP)){
        errorLogPrint("BLOCK");
        return "exception";
    }

    bool hasBeenBlocked = isIPinBlockedList(_clientIP);
    bool notInLocalLoggedInList = !isIPinLoggedInList(_clientIP);

    if (hasBeenBlocked || notInLocalLoggedInList){
        errorLogPrint("BLOCK");
        return "exception";
    }

    string request = string("BLOCK");
    request = request + " " + _clientIP;
    sendMsgtoSocket(serverSocket, request);
    Gblockedlist.push_back(_clientIP);
    cse4589_print_and_log("[BLOCK:SUCCESS]\n");
    cse4589_print_and_log("[BLOCK:END]\n");

    return "block";
}

string Client::onUNBLOCK(string _clientIP){
    cout << "I'm in UNBLOCK, and processing with serverSocket: "<<serverSocket << endl;

    if(!isValidIP(_clientIP) || !isIPinBlockedList(_clientIP) || !isIPinLoggedInList(_clientIP)){
        errorLogPrint("UNBLOCK");
        return "exception";
    }

//    assert(serverSocket >= 0);
    string request = string("UNBLOCK");
    request = request + " " + _clientIP;
    sendMsgtoSocket(serverSocket, request);

    return "unblock";
}

string Client::onLOGOUT(){
	cout << "I'm in onLOGOUT, and processing with serverSocket: "<<serverSocket << endl;
//    assert(serverSocket >= 0);
    string request = string("LOGOUT");
    request = request + " " + selfIP;
    sendMsgtoSocket(serverSocket, request);
    logStatus = false;

    cse4589_print_and_log("[LOGOUT:SUCCESS]\n");
    cse4589_print_and_log("[LOGOUT:END]\n");
	return "logout";
}
string Client::onEXIT(){
    cout << "I'm in onEXIT, and processing with serverSocket: "<<serverSocket << endl;
//    assert(serverSocket >= 0);
    string request = string("EXIT");
    sendMsgtoSocket(serverSocket, request);
    cse4589_print_and_log("[EXIT:SUCCESS]\n");
    cse4589_print_and_log("[EXIT:END]\n");
    close(serverSocket);
    exit(-1);
    return "exit";
}

string Client::onBROADCAST(string strBroadcast){
    cout << "I'm in BROADCAST, and processing with serverSocket: "<<serverSocket << endl;
//    assert(serverSocket >= 0);
    string request = string("BROADCAST");
    request = request + " " + strBroadcast;
    sendMsgtoSocket(serverSocket, request);
    cse4589_print_and_log("[BROADCAST:SUCCESS]\n");
    cse4589_print_and_log("[BROADCAST:END]\n");

    return "broadcast";
}
