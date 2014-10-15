/*
 * conn_interface.hpp
 *
 *  Created on: Mar 5, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_CONN_INTERFACE_HPP_
#define JOSHI_CONN_INTERFACE_HPP_
namespace arawat {
namespace protocol {

/**
 * connection interface
 */
class conn_interface {
public:
	/**
	 * constructor
	 * @param id
	 * @return
	 */
	explicit conn_interface(const std::string& id,
			util::msg::e_msg_protocol proto) :
		id_(id), protocol_(proto), running_(false) {
	}
	/**
	 * destructor
	 * @return
	 */
	virtual ~conn_interface() {
	}
	/**
	 * initialize
	 * @return
	 */
	virtual result_t init() = 0;
	/**
	 * send message
	 * @param message
	 * @return
	 */
	virtual result_t send_message(const proto_data_interface& message) = 0;


	/**
	 * start
	 */
	virtual void start() = 0;
	/**
	 * stop the connection
	 */
	virtual void stop(bool bforce) = 0;

	/**
	 * is running
	 * @return
	 */
	bool running() {
		return running_;
	}
	/**
	 * id
	 * @return
	 */
	const std::string id() const {
		return id_;
	}
	/**
	 * type
	 * @return
	 */
	arawat::util::msg::e_msg_protocol protocol() {
		return protocol_;
	}
protected:
	std::string id_;

	arawat::util::msg::e_msg_protocol protocol_;

	bool running_;
};
}
}

#endif /* JOSHI_CONN_INTERFACE_HPP_ */
