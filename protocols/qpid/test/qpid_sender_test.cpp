/*
 * qpid_sender._test.cpp
 *
 *  Created on: Sep 3, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */
#include <iostream>
#include <boost/signal.hpp>
#include <conn_mgr.hpp>
#include <boost_logger.hpp>
#include <qpid_agent.hpp>
#include <smpp.hpp>
#include <smpp_message.hpp>
#include <log.hpp>

using namespace arawat::protocol;
using namespace arawat::util;


void msg_receiver(qpid::client::Message& msg) {
  LOG_INFO("Message received:[%s]", msg.getData().c_str());
}
int main(int argc, char** argv) {
	arawat::util::log::init(arawat::util::boost_logger::write_log);
	arawat::util::boost_logger::instance().init(
			"/home/rjoshi/arawat_dev/config/boost-log.ini");

	if (argc < 2) {
		std::cerr
				<< "Usage smpp_test [loglevel] \n\te.g smpp_test INFO, request_per_thread, is_deliver_sm"
				<< std::endl;
		return (EXIT_SUCCESS);
	}
	if (argc > 1) {
		char* log_level = argv[1];
		if (strcasecmp(log_level, "INFO") == 0) {
			std::cout << "INFO logging is enabled..." << std::endl;
			arawat::util::log::enable_info();
			LOG_INFO("Info logging is enabled");
		} else if (strcasecmp(log_level, "TRACE") == 0) {
			std::cout << "TRACE logging is enabled..." << std::endl;
			arawat::util::log::enable_trace();
			LOG_TRACE("Trace logging is enabled");
		} else if (strcasecmp(log_level, "WARN") == 0) {
			std::cout << "WARN logging is enabled..." << std::endl;
			arawat::util::log::enable_warn();
			LOG_WARN("Warn logging is enabled");
		}
	}
	//
	//	 if(argc > 2) {
	//		 std::string thds = argv[2];
	//		 LOG_INFO("Requests[%s]", thds.c_str());
	//		// request_per_threads = boost::lexical_cast<unsigned>(thds);
	//	 }
	//	 if(argc > 3) {
	//
	//		 std::string dsm = argv[3];
	//		 LOG_INFO("deliver_sm[%s]", dsm.c_str());
	//		 if (strcasecmp(argv[3], "true") == 0) {
	//		//   deliver_sm = true;
	//		 }
	//	 }
	//typedef qpid_agent<smpp_message_req> qpid_smpp_sender;

	qpid_agent qsender("qpid_test_queue; {create: always}", "route_abc", "localhost", 5672,
			qpid_agent::QUEUE_SENDER);

	LOG_TRACE("Adding handler..");
	qsender.add_receiver_handler(boost::bind(&msg_receiver, _1));
	LOG_TRACE("Initializing..");
	result_t r = qsender.init();
	if(r.get<0>() == false) {
		LOG_ERROR("Failed to initialize..");
		return EXIT_SUCCESS;
	}
	LOG_TRACE("Starting..");
	qsender.start();

	smpp_message_req msg(Smpp::CommandId::SubmitSm);
	msg.from_.address_ = "1000000000";
	smpp_address addr;
	addr.address_ = "8045551001";
	msg.to_list_.push_back(addr);
	msg.msg_ = "test message";
	LOG_TRACE("Sending message..");

	qsender.send_message<smpp_message_req>(msg);

	qsender.stop();

	LOG_INFO("Done with test.");
	std::cout << "Done with test" << std::endl;

	return EXIT_SUCCESS;

}
