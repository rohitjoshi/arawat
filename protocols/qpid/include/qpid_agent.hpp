/*
 * qpid_receiver.hpp
 *
 *  Created on: Apr 1, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_QPID_RECEIVER_HPP_
#define JOSHI_QPID_RECEIVER_HPP_

#include <boost/signal.hpp>
#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Message.h>
#include <qpid/messaging/Receiver.h>
#include <qpid/messaging/Sender.h>
#include <qpid/messaging/Session.h>
#include <qpid/client/Message.h>
#include <qpid/client/MessageListener.h>
#include <qpid/client/Connection.h>
#include <qpid/client/SubscriptionManager.h>

#include <conn_interface.hpp>
#include <proto_data_interface.hpp>
#include <msgpack_util.hpp>

using namespace qpid::messaging;

namespace arawat {
namespace protocol {

class qpid_agent: boost::noncopyable,
		public conn_interface,
		public qpid::client::MessageListener {
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
	explicit qpid_agent(const std::string& qname, const std::string rkey,
			const std::string& host, int port, direction dir) :
		conn_interface(arawat::util::uuid::gen_uid_str(),
				arawat::util::msg::PROTO_QPID), qname_(qname), routing_key_(
				rkey), host_(host), port_(port), dir_(dir) {
		LOG_IN("");
		LOG_OUT("");
	}

	/**
	 * destructor
	 * @return
	 */
	~qpid_agent() {
		LOG_IN("");
		ses_.close();
		conn_.close();
		LOG_OUT("");
	}

	void add_receiver_handler(const observer_msg_receiver_type& observer) {
		LOG_IN("observer[%p]", &observer);
		updated_receiver_msg.connect(observer);
		LOG_OUT("");
	}

	/**
	 * init
	 */
	result_t init() {
		LOG_IN("");
		try {
			conn_.open(host_, port_);
			//conn_.SetOption("reconnect", true);

		} catch (qpid::ConnectionException& ex) {
			LOG_ERROR("Exception:[%s], Failed to connect host[%s], Port[%d]",
					ex.what(), host_.c_str(), port_);
			LOG_RET_RESULT(result_t(false, "failed"));
		}
		ses_ = conn_.newSession();


		running_ = false;

		LOG_INFO("Initialized connection successfully..");
		result_t r(true, "success");
		LOG_RET_RESULT(r);
	}

	/**
	 * Declare a queue for receiver
	 */
	void declare_queue() {
		LOG_INFO("Declaring a queue[%s]", qname_.c_str());
		ses_.queueDeclare(qpid::client::arg::queue = qname_);

		LOG_INFO("Declaring a exchangeBinding[amq.direct], routingKey[%s], qname[%s]", routing_key_.c_str(), qname_.c_str());
		ses_.exchangeBind(qpid::client::arg::exchange="amq.direct", qpid::client::arg::queue=qname_,
						qpid::client::arg::bindingKey=routing_key_);

	}
	/**
	 * Add the response queue for sent message
	 * @param name
	 */
	void add_response_queue(const std::string& name) {


	}
	/**
	 * Start
	 */
	void start() {
		LOG_IN("");
		//declare_queue();
		running_ = true;
		LOG_OUT("");
	}
	/**
	 * start
	 */
	void stop(bool force = false) {
		LOG_IN("force[%d]", force);
		running_ = false;
		LOG_OUT("");

	}

	/**
	 * stop subscription
	 * @return
	 */
	result_t stop_subscription() {
		LOG_IN("");
		try {
			p_subs_ptr_->stop();
			running_ = false;
		} catch (const qpid::Exception& e) {
			LOG_RET("failed", false);

		}
		result_t r(true, "success");
		LOG_RET_RESULT(r);

	}

	/***
	 * start subscription
	 * @return
	 */
	result_t start_subscription() {
		LOG_IN("");
		declare_queue();
		p_subs_ptr_.reset(new qpid::client::SubscriptionManager(ses_));
		try {
			LOG_INFO("Subscribing to queue[%s]", qname_.c_str());
			p_subs_ptr_->subscribe((qpid::client::MessageListener&) *this,
					qname_);
			p_subs_ptr_->run();
			running_ = true;
		} catch (const qpid::Exception& e) {
			LOG_ERROR("Exception[%s], Failed to subscribe to qname[%s]",
					e.what(), qname_.c_str());
			result_t r(false, "failed");
			LOG_RET_RESULT(r);
		}
		result_t r(true, "success");
		LOG_RET_RESULT(r);
	}

	/**
	 * Send message to the queue
	 */
	result_t send_message(const property_map_t& props, const std::string& data,
			const std::string& routing_key = "") {
		LOG_IN("props.size[%d], data[%s], routing_key[%s]", props.size(),
				data.c_str(), routing_key.c_str());
		if (!running()) {
			return result_t(false,
					"Queue is not started. Not accepting messages");
		}
		qpid::client::Message qmsg;
		if (!routing_key.empty()) {
			qmsg.getDeliveryProperties().setRoutingKey(routing_key);
		} else {
			qmsg.getDeliveryProperties().setRoutingKey(routing_key_);
		}
		property_map_t::const_iterator it = props.begin();
		for (; it != props.end(); ++it) {
			//qmsg()[it->first] = it->second;
		}
		qmsg.setData(data);
		qmsg.getMessageProperties().setCorrelationId(id_);
		try {
			ses_.messageTransfer(qpid::client::arg::content = qmsg,
					qpid::client::arg::destination = qname_);
		} catch (const qpid::Exception& e) {
			LOG_ERROR("Exception[%s], Failed to send message to qname[%s]",
					e.what(), qname_.c_str());
			result_t r(false, "failed");
			LOG_RET_RESULT(r);
		}
		return result_t(true, "successfully sent message");
	}

	/**
	 * Send message to the queue
	 * @param message
	 * @return
	 */
	template<typename T>
	result_t send_message(const T& message) {
		LOG_IN("");
		if (!running()) {
			return result_t(false,
					"Queue is not started. Not accepting messages");
		}

		msgpack::sbuffer sbuf;
		util::msgpack_util<T>::encode(message, sbuf);

		qpid::client::Message qmsg(std::string(sbuf.data(), sbuf.size()),
				routing_key_);
		LOG_TRACE("Sending message size[%d]", sbuf.size());
		qmsg.getDeliveryProperties().setRoutingKey(routing_key_);
		qmsg.getMessageProperties().setCorrelationId(id_);


		try {
			ses_.messageTransfer(qpid::client::arg::content = qmsg,
					qpid::client::arg::destination = "amq.direct");
		} catch (const qpid::Exception& e) {
			LOG_ERROR("Exception[%s], Failed to send message to qname[%s]",
					e.what(), qname_.c_str());
			result_t r(false, "failed");
			LOG_RET_RESULT(r);
		}

		return result_t(true, "successfully sent message");
	}

	result_t send_message(const proto_data_interface& message) {
		return result_t(true, "successfully sent message");
	}

	/**
	 * Send Message to the queue
	 * @param message
	 * @return
	 */
	result_t send_message(const std::string& message) {
		LOG_IN("message[%s]", message.c_str());
		if (!running()) {
			return result_t(false,
					"Queue is not started. Not accepting messages");
		}

		qpid::client::Message qmsg(message, routing_key_);
		//qmsg.getDeliveryProperties().setRoutingKey(routing_key_);
		qmsg.getMessageProperties().setCorrelationId(id_);

		try {
			ses_.messageTransfer(qpid::client::arg::content = qmsg,
					qpid::client::arg::destination = "amq.direct");
		} catch (const qpid::Exception& e) {
			LOG_ERROR("Exception[%s], Failed to send message to qname[%s]",
					e.what(), qname_.c_str());
			result_t r(false, "failed");
			LOG_RET_RESULT(r);
		}
		return result_t(true, "successfully sent message");
	}

	/**
	 * received
	 * @param msg
	 */
	virtual void received(qpid::client::Message& msg) {
		LOG_IN("");
		updated_receiver_msg(msg);
		LOG_OUT("");
	}

	//	/**
	//	 * type
	//	 * @return
	//	 */
	//	std::string type() {
	//		if(dir_ == QUEUE_SENDER) {
	//			return STR_QUEUE_SENDER;
	//		}
	//		return STR_QUEUE_RECEIVER;
	//	}


private:
	std::string qname_;
	std::string routing_key_;
	std::string host_;
	int port_;
	direction dir_;
	qpid::client::Connection conn_;
	qpid::client::Session ses_;
	boost::shared_ptr<qpid::client::SubscriptionManager> p_subs_ptr_;
	signal_msg_receiver_type updated_receiver_msg;

};

}
}

#endif /* JOSHI_QPID_QUEUE_RECEIVER_HPP_ */
