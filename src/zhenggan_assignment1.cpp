/**
 * @zhenggan_assignment1
 * @author  Zhenggang Xue <zhenggan@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
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


#include "../include/global.h"
#include "../include/logger.h"
#include "../include/Client.h"
#include "../include/Server.h"


using namespace std;

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

int main(int argc, char **argv){
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
  fclose(fopen(LOGFILE, "w"));

	/*Start Here*/

	if (argc != 3) {
		std::cerr << "XueError:The Input Parameters are not in correct syntax.." << '\n';
		exit(-1);
	}

	string indicatorStr = argv[1];
	string portStr = argv[2];

	cout <<"Server/Clients?: "<< indicatorStr << " : "<<portStr << endl;

	if (indicatorStr == "s") {
		Server myServer = Server(portStr);
		myServer.start();
	}else if (indicatorStr =="c") {
		Client myClient = Client(portStr);
//		myClient.start();
		myClient.startNew();
	}else{
		std::cerr << "XueError:First parameter should be 'c' or 's..'" << '\n';
	}

	return 0;
}
