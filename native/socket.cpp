/*
 * socket.cpp
 *
 *  Created on: May 7, 2014
 *      Author: vova
 */
/*
 * socket.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */


#include <sstream>
#include "socket.h"
#include "config.h"


std::string Addr::str() {
	std::stringstream os;
	os << ip << ":" << port;
	return os.str();
}

Socket::Socket(Addr addr) : addr(addr) {}

Socket::~Socket() {}


