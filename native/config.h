/*
 * config.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdlib.h>
#include <string>
#include "socket.h"
#include <log4cpp/Category.hh>

/*
 * Logging system
 */
//Access to the log
#define LOG log4cpp::Category::getRoot()

//Initialization of the log
void init_log();

/*
 * Major configuration class
 */
class Config {
	static Config* instance;
	Config() : inbound_secure(false), outbound_secure(false){};
public:

	Addr inbound;
	Addr outbound;
	bool inbound_secure;
	bool outbound_secure;
	std::string key_file;
	std::string crt_file;

	static void init() {
		Config::instance = new Config();
	}
	static void done() {
		delete Config::instance;
	}
	static Config& get() {
		return *Config::instance;
	}
};




#endif /* CONFIG_H_ */
