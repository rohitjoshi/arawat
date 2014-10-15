//============================================================================
// Name        : smpp_test.cpp
// Author      : Rohit Joshi
// Version     :
// Copyright   : Copy Rights Joshi Inc.
// Description : smpp test
//============================================================================

#include <iostream>
#include <boost/signal.hpp>
#include <smpp_conn.hpp>
#include <smpp_config.hpp>
#include <smpp_message.hpp>
#include <conn_mgr.hpp>
#include <boost_logger.hpp>
#include <log.hpp>

using namespace arawat::protocol;
using namespace arawat::util;

unsigned request_per_threads = 10;
bool deliver_sm = false;
void write_log(const unsigned int logtype,
		const char* file, unsigned int line, const char* func, const char * buf,
		...) {


	 std::string msg;
	 va_list args;
	 va_start(args, buf);
	 msg = log::format_arg_list(buf, args);
	 va_end(args);
     std::string inout;
     if(logtype == log::LOG_TRACE_IN)
    	 inout = "-->IN|";
     else if(logtype == log::LOG_TRACE_OUT)
    	 inout = "<--OUT|";

	 std::cout << logtype << "|" << file << ":" << line << "|" << func << "|"
			   << inout
			   << msg << std::endl;

}
namespace {
const std::string str_msg = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
}
class smpp_test {
public:
	typedef conn_mgr<smpp_conn, smpp_config> smpp_conn_mgr_t;
	typedef std::vector<smpp_config>  config_list_t;
//	typedef boost::signal<bool(const proto_data_interface&)> signal_msg_type;
//	typedef signal_msg_type::slot_type observer_msg_type;

	smpp_test(config_list_t& configs):received_(0),sent_(0){
		LOG_IN("");

		result_t r= conn_mgr_.init(configs, boost::bind(&smpp_test::msg_receiver, this, _1, _2));
		    if(r.get<0>()) {
		    	LOG_INFO("Successfully initialized.");
		    	conn_mgr_.start();

		    }else {
		    	LOG_ERROR("Failed to initialize [%s]", r.get<1>().c_str());
		    	//return 0;
		    }
		    LOG_OUT("");
	}
	~smpp_test() {
		LOG_IN("");
		LOG_OUT("");
	}

	void send() {
		//send_message("localhost:9000", "8045551000", "8045551001", "this is a test msg");
		 boost::thread t1(boost::bind(&smpp_test::send_message, this, "localhost:9000", "1000000000", "8045551001", str_msg));
		 boost::thread t2(boost::bind(&smpp_test::send_message, this, "localhost:9000_1", "2000000000", "8045551002", str_msg));
		 boost::thread t3(boost::bind(&smpp_test::send_message, this, "localhost:9000_2", "3000000000", "8045551003", str_msg));
		 boost::thread t4(boost::bind(&smpp_test::send_message, this, "localhost:9000_3", "4000000000", "8045551004", str_msg));
		 boost::thread t5(boost::bind(&smpp_test::send_message, this, "localhost:9000_4", "5000000000", "8045551004", str_msg));

		 int i = 500;
	     unsigned total = 0;
	     unsigned expected_count = request_per_threads * 5;
	     if(deliver_sm) {
	    	 expected_count *= 2;
	     }
	     while(i--  && total < expected_count) {
	    	 sleep(1);
	    	 {
	    	 	 boost::mutex::scoped_lock guard(mt_);
	    	 	 total = received_;
	    	 }
	    	 LOG_WARN("Received Messages:[%d]", total);
	     }
	     LOG_WARN("Joining threads T1...");
	     if (t1.joinable()) {
			t1.join();
		}
	     LOG_WARN("Joining threads T2...");
		if (t2.joinable()) {
			t2.join();
		}
		 LOG_WARN("Joining threads T3...");
		if (t3.joinable()) {
			t3.join();
		}
		 LOG_WARN("Joining threads T4...");
		if (t4.joinable()) {
			t4.join();
		}
		LOG_WARN("Joining threads T5...");
		if (t5.joinable()) {
			t5.join();
		}
		 LOG_WARN("Joining threads done...");
	     conn_mgr_.stop();
	     LOG_INFO("stopped succesfully");
	}

	void send_message(const std::string& id, const std::string& from, const std::string& to, const std::string& body) {
		LOG_IN("id[%s], fron[%s], to[%s], body[%s]", id.c_str(), from.c_str(), to.c_str(), body.c_str());
		 smpp_message_req msg(Smpp::CommandId::SubmitSm);
		 msg.from_.address_ = from;
         smpp_address addr;
         addr.address_ = to;
		 msg.to_list_.push_back(addr);
		 msg.msg_ = body;

		 long nfrom = boost::lexical_cast<long>(msg.from_.address_);
		 boost::shared_ptr<smpp_conn> c = conn_mgr_.get_conn_by_id(id);
		 unsigned i = 0;
		 while(i < request_per_threads) {
		   if(c.get()) {
			   {
				 //  boost::mutex::scoped_lock guard(mt_sent_);
			    //   std::cout << "sender:" <<  sent_++ << std::endl;
		       }
			   long f = nfrom + i++;
			   msg.from_.address_ = boost::lexical_cast<std::string>(f);
			   LOG_INFO("Sending message to %s", msg.from_.address_.c_str());
			   c->send_message(msg);
			   usleep(1);
		   }else {
			 LOG_ERROR("Failed to get connection for id[%s], i[%d]", id.c_str(), i);
		   }
		 }
	}
	/**
	 * msg receiver
	 * @param msg
	 * @return
	 */
	bool msg_receiver(const proto_data_interface& msg, const std::string& conn_id) {
	   LOG_IN("");
	   unsigned l = 0;
		{
			boost::mutex::scoped_lock guard(mt_);
			l = ++received_;


		}
		LOG_INFO("receiver[%d]", l);
	   if(msg.protocol() == msg::PROTO_SMPP) {
		   if(msg.type() == msg::TYPE_REQUEST) {
			   const smpp_message_req& request = (const smpp_message_req&) msg;
			   LOG_INFO("Smpp Msg  Received: conn_id[%s], seqnum[%d],received[%d].",
					   conn_id.c_str(), request.seqnum_, l);

		   }else {
			  // std::cout << "received:" <<  l << std::endl;
			   const smpp_message_resp& resp = (const smpp_message_resp&) msg;
			   LOG_INFO("Smpp Msg Response Received: conn_id[%s], seqnum[%d],received[%d].",
					   conn_id.c_str(), resp.seqnum_, l);
		   }
	   }else {
		   LOG_INFO("Unknown msg type");
	   }
	   LOG_RET("success", true);
	}
private:
//	observer_msg_type update_msg_type_;
//	conn_mgr<smpp_conn, smpp_config> conn_mgr_;
	//smpp_conn conn_;
	smpp_conn_mgr_t conn_mgr_;
	boost::mutex mt_;
	unsigned received_;
	boost::mutex mt_sent_;
	unsigned sent_;
};

int main(int argc, char** argv) {
	arawat::util::log::init(arawat::util::boost_logger::write_log);
	arawat::util::boost_logger::instance().init("/home/rjoshi/arawat_dev/config/boost-log.ini");

	if (argc < 2) {
	    std::cerr << "Usage smpp_test [loglevel] \n\te.g smpp_test INFO, request_per_thread, is_deliver_sm" << std::endl;
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

	 if(argc > 2) {
		 std::string thds = argv[2];
		 LOG_INFO("Requests[%s]", thds.c_str());
		 request_per_threads = boost::lexical_cast<unsigned>(thds);
	 }
	 if(argc > 3) {

		 std::string dsm = argv[3];
		 LOG_INFO("deliver_sm[%s]", dsm.c_str());
		 if (strcasecmp(argv[3], "true") == 0) {
		   deliver_sm = true;
		 }
	 }
	smpp_config config, config1, config2, config3, config4;
    config.host_ = "localhost";
    config.port_ = "9000";
    config.bindtype_ = smpp_config::BIND_TXRX;
    config.id_ = config.host_ + ":" + config.port_;
    config1 = config;
    config1.id_ += "_1";
    config2 = config;
    config2.id_ += "_2";

    config3 = config;
    config3.id_ += "_3";

    config4 = config;
    config4.id_ += "_4";

    smpp_test::config_list_t configs;
    configs.push_back(config);
    configs.push_back(config1);
    configs.push_back(config2);
    configs.push_back(config3);
    configs.push_back(config4);

    smpp_test t(configs);


    t.send();

   LOG_INFO("Done with test.");
   std::cout << "Done with test" << std::endl;
    return EXIT_SUCCESS;
    std::cout << "Done with test" << std::endl;
}
