/*
 * config.cpp
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"

#include "config.h"

Config* Config::instance = NULL;

void init_log(std::string log_file)
{
	log4cpp::Appender *appender1 = new log4cpp::OstreamAppender("console", &std::cout);
	appender1->setLayout(new log4cpp::BasicLayout());

//	log4cpp::Appender *appender = new log4cpp::FileAppender("default", log_file);
//	log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
//	std::stringstream spid; spid << getpid();
//	layout->setConversionPattern("[%p] %m%n");
//	appender->setLayout(layout);

	log4cpp::Category& root = log4cpp::Category::getRoot();
//	root.removeAllAppenders();
	root.setPriority(log4cpp::Priority::DEBUG);
	root.addAppender(appender1);
}

