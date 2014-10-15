/*
 *  File: boost_logger.cpp
 *  Author: rohitjoshi
 *  Created on: Sep 6, 2009
 *  Copyright (c) 2009 . See LICENSE for details.
 */
#include <iostream>
#include "boost_logger.hpp"
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_logger.hpp>
//#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/init/to_console.hpp>
#include <boost/log/utility/init/to_file.hpp>
#include <boost/log/utility/init/from_stream.hpp>
#include <boost/log/utility/init/common_attributes.hpp>

#include "log.hpp"



//namespace
namespace arawat {
namespace util {


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)
#define TID pthread_self()
#define PID getpid()


BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(boostlogger, boost::log::sources::severity_logger_mt< >)

//BOOST_LOG_DECLARE_LOGGER_TYPE(boostlogger, boost::log::sources::severity_logger_mt< >)
//BOOST_LOG_DECLARE_GLOBAL_LOGGER(boostlogger, boost::log::sources::severity_logger_mt< >)

boost_logger::boost_logger():init_(false){
   std::cout << "boost_logger():init_:" << init_ << std::endl;
}
boost_logger::~boost_logger(){
	std::cout << "~boost_logger()" << std::endl;

}
/**
 * intialize logger.
 * @param filepath
 * @return
 */
bool boost_logger::init(const std::string& filepath) {
	std::cout << "boost_logger::init():" << filepath << std::endl;
	//if initialized, return
	if (init_)
		return init_;
	//read the logging ini and initialize the logger...
	std::cout << "Opening file " << filepath << std::endl;
	std::ifstream file(filepath.c_str());
	if (!file.is_open()) {
		std::cerr << "Could not open log configuration file: " << filepath
				<< std::endl;
		boost::log::init_log_to_console(std::cout);
	} else {
		//boost::log::init_log_to_console(std::cout);
		std::cout << "Initializing logger from stream:" << filepath << std::endl;
		boost::log::init_from_stream(file);
		//boost::log::init_log_to_file("/tmp/http.log");

	}
	boost::log::add_common_attributes();
	init_ = true;
    std::cout << "severity level:" << boost::log::sources::aux::get_severity_level() << std::endl;
	std::cout << "boost_logger initialized successfully.." << std::endl;

    std::cout << "severity level:" << boost::log::sources::aux::get_severity_level() << std::endl;
	return init_;
}
/**
 * write log
 * @param logtype
 * @param file
 * @param line
 * @param func
 * @param buf
 */
void boost_logger::write_log(const unsigned int logtype,
		const char* file, unsigned int line, const char* func, const char * buf,
		...) {

	 std::string msg;
	 va_list args;
	 va_start(args, buf);
	 msg = log::format_arg_list(buf, args);
	 va_end(args);
     std::string func_inout = func;
     func_inout += "|";

     if(logtype == log::LOG_TRACE_IN)
    	 func_inout += "-->IN|";
     else if(logtype == log::LOG_TRACE_OUT)
    	 func_inout += "<--OUT|";

     const char* filename = file;
     const char* p = strrchr(file, '/');
     if(p) {
    	 filename = ++p;
     }


     severity_level level = warn;

     switch(logtype) {
     case log::LOG_TRACE_IN:
    	 level = trace;
    	 msg = func_inout + msg;
    	 break;
     case log::LOG_TRACE_OUT:
    	 level = trace;
    	 msg = func_inout + msg;
    	 break;
     case log::LOG_TRACE:
    	 msg = func_inout + msg;
    	 level = trace;
    	 break;
     case log::LOG_INFO:
    	 level = info;
    	 break;
     case log::LOG_EVENT:
    	 level = event;
    	 break;
     case log::LOG_WARNING:
    	 level = warn;
    	 break;
     case log::LOG_ERROR:
    	 level = error;
    	 break;
     default:
    	 break;
     }


     BOOST_LOG_SEV(boostlogger::get(),level)  <<  boost::format("%5d|%8X|") %PID % (TID%10000000000)
    		 << filename << ":" << std::dec << line << "|" << msg;
}
}//namespace util
} //ns arawatin
