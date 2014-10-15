/*
 * smpp_conn.hpp
 *
 *  Created on: Dec 27, 2010
 *      Author: rarawat
 *  Arawat Inc Reserved 2010
 */

#ifndef ARWT_SMPPCONN_HPP_
#define ARWT_SMPPCONN_HPP_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/detail/atomic_count.hpp>
#include <smpp.hpp> //smppcxx include
#include <client_connection.hpp>
#include <proto_data_interface.hpp>
#include "conn_interface.hpp"
#include "smpp_config.hpp"
#include "smpp_message.hpp"

using namespace Smpp;
/*
 * namespace arawat
 */

namespace arawat {
/**
 * namespace protocol
 */

namespace protocol {

//forward declaration
struct smpp_config;
/**
 * Creating smpp class which provides the sending and receving smpp messages asynchronously.
 */
class smpp_conn : public conn_interface {

public:
	typedef boost::shared_ptr<boost::thread> thread_ptr_t;
	typedef std::list<smpp_message_req> smpp_messages_t;
	/**
	 * call back function for data and connection id
	 */
	typedef boost::signal<bool(const proto_data_interface&, const std::string&)> signal_msg_type;
	typedef signal_msg_type::slot_type observer_msg_type;

	/**
	 * Constructor
	 */
	explicit smpp_conn(const smpp_config& config, const observer_msg_type& observer);
	/**
	 * Destructor
	 */
	virtual ~smpp_conn();
	/**
	 * Initialize the connection
	 * @param conf
	 * @return
	 */
	result_t init();


	 std::string id() const {
		return id_;
	}
	std::string type() const{
		  return config_.bindtype();
	}
	/**
	 * send message
	 * @param data
	 * @return
	 */
	result_t send_message(const proto_data_interface& message);

	/**
	 * start the threads
	 */
	void start();

	/**
	 * stop and exit..
	 */
	void stop(bool force=false);

	/**
	 * receive message
	 * @param data
	 * @return
	 */
	//static result_t receive(const proto_data_interface& data) = 0;

protected:

	/**
	 * Send bind
	 * @return
	 */
	result_t send_bind();

	/**
	 * send unbind
	 * @return
	 */
	result_t send_unbind();
	/**
	 * send enquire link
	 * @return
	 */
	result_t send_enquire_link();

	/**
	 * send bind
	 * @return
	 */
	template<typename T> result_t send_bind(T& t);
	/**
	 * send message
	 * @param pdu
	 * @param data
	 * @return
	 */
	template<typename T, typename D> result_t send_msg(T& pdu, D& data);

	/**
	 * check error on payload
	 * @param pdu
	 * @param buffer
	 * @return
	 */
	template<typename T> bool check_error(T& pdu,
			const client_connection::uchar_buffer_t& buffer);

	/**
	 * read deliver sm
	 * @param buffer
	 * @return
	 */
	template<typename T>  bool read_msg_sm(T& pdu, const client_connection::uchar_buffer_t& buffer,
			smpp_message_req& msg);
	/**
	 * read sm response
	 * @param pdu
	 * @param buffer
	 * @param msg
	 * @return
	 */
	template<typename T> bool read_sm_resp(T& pdu,
			const client_connection::uchar_buffer_t& buffer,
			smpp_message_resp& msg);

	/**
	 * add_to_queue
	 * @param msg
	 * @return
	 */
	bool add_to_queue(const proto_data_interface& msg);
	/**
	 * increment sequence number
	 * @return
	 */
	unsigned increment_seqnum() {
		return ++seqnum_;
	}

	/**
	 * write buffer to socket
	 * @param buffer
	 * @param length
	 * @return
	 */
	result_t send_async(boost::uint8_t* buffer, int length);
	/**
	 * Hex dump
	 * @param d
	 * @param len
	 */
	void hex_dump(const boost::uint8_t* d, unsigned len);
	/**
	 * hex dump
	 * @param d
	 * @param len
	 * @param ostrm
	 */
	void hex_dump(const boost::uint8_t* d, unsigned len,
			std::ostringstream& ostrm);

	/**
	 * Process incoming msg
	 * @param
	 * @param
	 */
	bool process_incoming_msg(const client_connection::uchar_buffer_t& buffer);

	/**
	 * get payload size
	 * This is used by client_connection class to read remaning payload
	 * @param
	 * @param
	 * @return
	 */
	size_t get_payload_size(const client_connection::uchar_buffer_t& buffer);

	/**
	 * error handling
	 * @param error_code
	 */
	void handle_error(const boost::system::error_code& error_code);

	/**
	 * send enquire message every keep alive seconds
	 */
	void enquirelink_loop();

private:

	/**
	 * smpp config
	 */
	smpp_config config_;
	/**
	 * sequence number
	 */
	boost::detail::atomic_count seqnum_;
	//check if successfully unbind sent
	bool unbind_sent_;
	bool unbind_resp_received_;
	/**
	 * client connection ptr
	 */
	client_connection_ptr conn_ptr_;

	/**
	 * is stop signal issues
	 */
	bool stop_;
	thread_ptr_t enquirelink_thread_ptr_;
	/**
	 * last activity time which is used to decide whether to send enquire link message
	 */
	time_t last_activity_time_;
	/**
	 * handle incoming message
	 */
	signal_msg_type update_handler_msg;

	std::string type_;







};

}//protocol
}//arawat

#endif /* ARWT_SMPPCONN_HPP_ */
