#ifndef LOGGEDINLISTITEMCLIENT_H_
#define LOGGEDINLISTITEMCLIENT_H_

//This is the Client version of LoggedInList

#include <string>
#include <vector>
using namespace std;

class LoggedInListItemClient{
public:
  LoggedInListItemClient(string _hostname, string _ip, int _port)
    :hostname(_hostname), ip_addr(_ip), port_num(_port){}

  string hostname;
  string ip_addr;
  int port_num;

  bool operator < (const LoggedInListItemClient &m)const{
        return port_num < m.port_num;
  }
};

#endif
