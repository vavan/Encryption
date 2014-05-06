/*
 * config.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include "socket.h"

class Config {
	static Config* instance;
public:

	Addr server;
	Addr client;

	static void init() {
		Config::instance = new Config();
	}
	static void done() {
		free(Config::instance);
	}
	static Config& get() {
		return *Config::instance;
	}
};




#endif /* CONFIG_H_ */
