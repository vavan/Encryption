/*
 * config.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <log4cpp/Category.hh>
#include "addr.h"

/*
 * Logging system
 */
//Access to the log
#define LOG log4cpp::Category::getRoot()


//Initialization of the log
void init_log(std::string log_file_name);

/*
 * Major configuration class
 */
class Config {
	static Config* instance;
public:

	Addr server;
	Addr client;

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
