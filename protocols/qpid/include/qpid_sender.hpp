/*
 * qpid_sender.hpp
 *
 *  Created on: Apr 1, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_QPID_SENDER_HPP_
#define JOSHI_QPID_SENDER_HPP_

#include <boost/signal.hpp>
#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Message.h>
#include <qpid/messaging/Receiver.h>
#include <qpid/messaging/Sender.h>
#include <qpid/messaging/Session.h>
#include <qpid/client/Message.h>

using namespace qpid::messaging;

namespace arawat {
namespace protocol {

template<typename T>
class qpid_queue:  public conn_interface  {
public:
	typedef std::map<std::string, std::string> property_map_t;
	typedef boost::signal<void(qpid::client::Message& msg)>
			signal_msg_receiver_type;
	typedef signal_msg_receiver_type::slot_type observer_msg_receiver_type;

	/**
	 * types
	 */
	enum direction {
		QUEUE_SENDER, //!< QUEUE_SENDER
		QUEUE_RECEIVER
	//!< QUEUE_RECEIVER
	};
	/**
	 * constructor
	 * @param qname
	 * @param rkey
	 * @param host
	 * @param port
	 * @param t
	 * @return
	 */
	explicit qpid_queue(const std::string& url, const std::string& conn_options,
			const std::string& qname, const std::string& resp_qname,
			) : conn_interface(arawat::util::uuid::gen_uid_str(), arawat::util::msg::PROTO_QPID)
		qname_(qname), resp_qname_(resp_qname),conn_(url, conn_options){
	}

	/**
	 * destructor
	 * @return
	 */
	~qpid_queue() {
		ses_.close();
		conn_.close();
	}

	/**
	 * init
	 */
	result_t init() {
		try {
		  conn_.open();
		  ses_ = conn_.createSession();
		  sender_ = ses_.createSender(qname_);
		}catch (qpid::Exception& e) {
			return result_t(false, e.what());
		}
		return result_t(true, "");
	}


	bool start() {
		running_ = true;
	}

	/**
	 * start
	 */
	void stop() {
		running_ = false;
	}

	/**
	 * Send message to the queue
	 * @param message
	 * @return
	 */
	result_t send_message(const proto_data_interface& message) {
		if(!running) {
			 return  result_t(false, "Queue is not started. Not accepting messages");
		}

		msgpack::sbuffer sbuf;
		util::msgpack_util<T>::encode(message,sbuf);

		qpid::client::Message message(subf.data(), sbuf.size());
		message.getDeliveryProperties().setRoutingKey(routing_key_);
		message.setCorrelationId(id_);

		ses_.messageTransfer(qpid::client::arg::content = message,
				qpid::client::arg::destination = qname_);
	}

	/**
	 * Send Message to the queue
	 * @param message
	 * @return
	 */
	result_t send_message(const std::string& message) {
		if(!running) {
			 return  result_t(false, "Queue is not started. Not accepting messages");
		}


		qpid::client::Message message(message, message.length());
		message.getDeliveryProperties().setRoutingKey(routing_key_);
		message.setCorrelationId(id_);

		ses_.messageTransfer(qpid::client::arg::content = message,
				qpid::client::arg::destination = qname_);
	}

private:
	std::string qname_;
	std::string resp_qname_;
	qpid::messaging::Connection conn_;
	qpid::messaging::Session ses_;
	qpid::messaging::Sender sender_;

};

}
}

#endif /* JOSHI_QPID_QUEUE_SENDER_HPP_ */
