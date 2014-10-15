/*
 * client_connection.hpp
 *
 *  Created on: Jan 5, 2011
 *      Author: rarawat
 *  Arawat Inc Reserved 2010
 */

#ifndef ARWT_CLIENT_CONN_HANDLER_HPP_
#define ARWT_CLIENT_CONN_HANDLER_HPP_

#include <vector>
#include <deque>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/signal.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/resolver_query_base.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp>
#include <uuid.hpp>
#include "log.hpp"

using namespace ::boost;
using namespace arawat::util;

namespace arawat {
namespace protocol {
#define MAX_BUFFER_SIZE 8192 //size of socket read
/**
 * connection handler class
 */
class client_connection {
public:
	typedef std::vector<boost::uint8_t> uchar_buffer_t;
	typedef boost::signal<bool(const uchar_buffer_t&)> signal_payload_type;
	typedef signal_payload_type::slot_type observer_payload_type;

	typedef boost::signal<size_t(const uchar_buffer_t&)>
			signal_header_type;
	typedef signal_header_type::slot_type observer_header_type;

	typedef boost::signal<void(const boost::system::error_code&)>
				signal_error_type;
		typedef signal_error_type::slot_type observer_error_type;

	typedef boost::shared_ptr<boost::thread> thread_ptr_t;

public:
	/**
	 * constructor
	 * @param service
	 * @param t
	 * @return
	 */
	explicit client_connection(int read_header_type /*>0=size, =0='\n', <0='\0' */) :
		socket_(service_), write_strand_(service_), read_header_type_(read_header_type),
		stop_(false){
		LOG_IN("");
		id_ = uuid::gen_uid_str();
		LOG_OUT("");
	}
	/**
	 * destructor
	 * @return
	 */
	virtual ~client_connection() {
		LOG_IN("");
		LOG_TRACE("Stopping io service..");
		service_.stop();
		LOG_TRACE("Stopping io service done");

		if(iothread_ptr.get() && iothread_ptr->joinable()) {
		  LOG_TRACE("Joining thread..");
		  iothread_ptr->join();
		  LOG_TRACE("Joining thread done");
		}


		if (socket_.is_open()) {
			LOG_TRACE("closing socket if open.");
			socket_.close();
		}
		LOG_OUT("");

	}
	/**
	 * get the connection id
	 * @return
	 */
	std::string id() const {
		return id_;
	}

	/**
	 * Connect to the server
	 * @return
	 */
	result_t connect(boost::asio::ip::tcp::resolver::query& query) {
		LOG_IN("");

		host_name_ = query.host_name();
		service_name_ = query.service_name();
		read_packet_done_handler_ = boost::bind(
				&client_connection::read_packet_done, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred);

		read_packet_body_handler_ = boost::bind(
				&client_connection::read_packet_body, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred);
		//if socket is open, close it
		if (socket_.is_open()) {
			socket_.close();
		}

		//resolve the query
		boost::asio::ip::tcp::resolver resolver(service_);
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
				resolver.resolve(query);

		boost::asio::ip::tcp::resolver::iterator end;
		//connect
		boost::system::error_code error = boost::asio::error::host_not_found;

		while (error && endpoint_iterator != end) {
			socket_.close();
			socket_.connect(*endpoint_iterator++, error);
		}
		if (error) {
			//update_handler_error(error);
			std::stringstream ss;
			ss << "Failed to connect to host " << query.host_name()
					<< ", error code:" << error.value() <<", error message:" << error.message();
			LOG_ERROR("%s", ss.str().c_str());
			LOG_RET_RESULT(result_t(false,ss.str()));
		}
		//LOG_INFO("Successfully connected to host:[%s]" , socket_.remote_endpoint().address().to_string().c_str());
		//resolver_ = resolver;
		LOG_RET_RESULT(result_t(true,"successfully connected to host:" + socket_.remote_endpoint().address().to_string()));

	}
	/**
	 * tcp no delay
	 * @param nodelay
	 */
	void tcp_no_delay(bool nodelay) {
		LOG_IN("nodelay[%d]", nodelay);
		socket_.set_option(boost::asio::ip::tcp::no_delay(nodelay));
		LOG_OUT();
	}

	/**
	 * add message handler
	 * @param observer
	 * @return
	 */
	boost::signals::connection add_message_handler(
			const observer_payload_type& observer) {
		return update_handler_payload.connect(observer);
	}

	/**
	 * add header handler
	 * @param observer
	 * @return
	 */
	boost::signals::connection add_header_handler(
			const observer_header_type& observer) {
		return update_handler_header.connect(observer);
	}

	/**
	 * error handler
	 * @param observer
	 * @return
	 */
	boost::signals::connection add_error_handler(
				const observer_error_type& observer) {
		return update_handler_error.connect(observer);
	}

	/**
	 * get the socket
	 * @return
	 */
	boost::asio::ip::tcp::socket& socket() {
		return socket_;
	}

	/**
	 * start read
	 */
	void start() {
		stop_ = false;
		LOG_IN("");
		if (!send_packet_queue_.empty()) {
			start_packet_send();
		}
		read_packet_async();
		thread_ptr_t tmp(new boost::thread(boost::bind(&boost::asio::io_service::run, &service_)));
		iothread_ptr = tmp;


		LOG_OUT("");
	}
    /**
     * stop
     */
	void stop(bool force=false) {
		LOG_IN("");
		stop_ = true;
		unsigned counter = 3;
		LOG_INFO("Stopping... Pending messages in queue[%d]",send_packet_queue_.size());
		if(!force) {
		  while(counter-- && !send_packet_queue_.empty()) {
			sleep(5); //sleep for five seconds if queue is not empty
			LOG_INFO("Stopping... Pending messages in queue[%d]",send_packet_queue_.size());
		  }
		}
		if(send_packet_queue_.size()) {
			LOG_WARN("Stopping... Pending messages in queue[%d] will be lost",send_packet_queue_.size());
		}
		LOG_INFO("Stopping io service...");
		service_.stop();
		LOG_INFO("Stopping io service done.");

		if(iothread_ptr.get() && iothread_ptr->joinable()) {
			LOG_INFO("Joining threads...");
		    iothread_ptr->join();
		    thread_ptr_t tmp((boost::thread*)NULL);
		    iothread_ptr = tmp;
		    LOG_INFO("Joining threads done.");
		}

		LOG_OUT("");
	}

	/**
	 * send message
	 * @param message
	 */
	bool send_async(uchar_buffer_t& message) {
		LOG_IN("message.size()[%d]", message.size());
		if(stop_) {
			LOG_INFO("Stop flag is enabled. not accepting messages...");
			LOG_RET("failed", false);
		}
		service_.post(
				write_strand_.wrap(boost::bind(
						&client_connection::queue_message, this,
						message)));
		LOG_RET("success", true);
	}

private:
	/**
	 * connect
	 * @return
	 */
	result_t connect() {
		boost::asio::ip::tcp::resolver::query query(host_name_, service_name_);
		return connect(query);
	}
	/**
	 * read packet
	 */
	void read_packet_async() {
		LOG_IN("");
		if(stop_) {
			LOG_INFO("Stop flag is enabled. not reading...");
			LOG_OUT("");
		}
		buffer_.clear();
		if (read_header_type_ > 0) {
			LOG_TRACE("Reading %d bytes asynchronously.", read_header_type_);
			buffer_.resize(read_header_type_);
			boost::asio::async_read(socket_,
					boost::asio::buffer(&buffer_[0],read_header_type_),
					read_packet_body_handler_);
		}
//		else {
//			buffer_.reserve(MAX_BUFFER_SIZE);
//			boost::asio::async_read_until(socket_, boost::asio::streambuf(buffer_),
//					(read_header_type_
//					== 0) ? '\n' : '\0', read_packet_done_handler_);
//		}
		LOG_OUT("");
	}
	/**
	 * Read the body
	 * @param error
	 * @param bytes_transferred
	 */
	void read_packet_body(const boost::system::error_code& error,
			size_t bytes_transferred) {

		LOG_IN("error[%s], bytes_transferred[%d]", error.message().c_str(),bytes_transferred);
		if (!error) {
			size_t buff_size = buffer_.size();
			LOG_TRACE("Buffer size[%d]",buff_size);
			size_t body_len = update_handler_header(buffer_);
			LOG_TRACE("Remaining payload size[%d]", body_len);
            if(body_len) {

            	buffer_.resize(buff_size + body_len);
				//todo add prepare statement
				boost::asio::async_read(socket_,
						boost::asio::buffer(&buffer_[buff_size],body_len),
						read_packet_done_handler_);
			} else {
				size_t s = read_packet_done(error, body_len);
				LOG_TRACE("read_packet_body[%d]", s);
			}
		} else {
			LOG_ERROR("Failed to read packet data.  Error[%s]",
					error.message().c_str());

			update_handler_error(error);

		}
		LOG_OUT("");

	}
	/**
	 * read packet done
	 * @param error
	 * @param bytes_transferred
	 */
	size_t read_packet_done(const boost::system::error_code& error,
			int bytes_transferred) {
		LOG_IN("bytes_transferred[%d]", bytes_transferred);
		size_t result = buffer_.size();
		LOG_TRACE("Buffer size[%d]", result);
		if (!error) {
			try {
				update_handler_payload(buffer_);
				read_packet_async();
				LOG_RET("Byte Read[%d]", result);
			} catch (std::exception& ex) {
				LOG_ERROR("Failed to read packet data.  Exception[%s]",
						ex.what());
			}

		} else {
			LOG_ERROR("Failed to read packet data.  Error[%s]",
					error.message().c_str());
			update_handler_error(error);

		}
		LOG_RET("Byte Read[%d]", result);
	}

	/**
	 * queue message
	 * @param message
	 */
	void queue_message(uchar_buffer_t& message) {
		LOG_IN("");
		bool write_in_progress = !send_packet_queue_.empty();
		send_packet_queue_.push_back(message);
		// if we aren’t currently doing a write start one
		if (!write_in_progress) {
			start_packet_send();
		}
		LOG_OUT("");
	}

	/**
	 * start packet send
	 */
	void start_packet_send() {
		LOG_IN("");
		//send_packet_queue.front() += NULL;
		// register the send
		boost::asio::async_write(socket_, boost::asio::buffer(
				send_packet_queue_.front()), write_strand_.wrap(boost::bind(
				&client_connection::packet_send_done, this,
				boost::asio::placeholders::error)));
		LOG_OUT("");
	}

	/**
	 * packet send done
	 * @param error
	 */
	void packet_send_done(const boost::system::error_code& error) {
		LOG_IN("");
		if (!error) {
			// pop the sent packet from the deque and start the next one if we have more
			send_packet_queue_.pop_front();
			if (!send_packet_queue_.empty()) {
				LOG_TRACE("Size of Send Queue[%d]", send_packet_queue_.size());
				start_packet_send();

			}
		} else {
			std::stringstream ss;
			ss << "Failed to send packet data.  Error:"
					<< error.message()
					<< ". Size of Queue:" << send_packet_queue_.size();
			LOG_ERROR("%s", ss.str().c_str());

			update_handler_error(error);
		}

		LOG_OUT("");


	}

	//boost::asio::ip::tcp::resolver resolver_;
	boost::asio::io_service service_;
	boost::asio::ip::tcp::socket socket_;
	//boost::asio::ip::tcp::resolver::query& query_;
	boost::asio::io_service::strand write_strand_;
	int read_header_type_;
	uchar_buffer_t buffer_;
	std::deque<uchar_buffer_t> send_packet_queue_;
	signal_payload_type update_handler_payload;
	signal_header_type update_handler_header;
	signal_error_type  update_handler_error;

	boost::function<size_t(const boost::system::error_code& error, size_t)>
			read_packet_done_handler_;
	boost::function<void(const boost::system::error_code& error, size_t)>
			read_packet_body_handler_;

	// boost::asio::ip::basic_resolver_query::protocol_type  protocol_;
	std::string host_name_;
	std::string service_name_;
	boost::asio::ip::resolver_query_base::flags resolve_flags_;

	thread_ptr_t  iothread_ptr;

	bool stop_;
	std::string id_;

};
typedef boost::shared_ptr<client_connection> client_connection_ptr;
} //ns protocol
} //ns arawat
#endif /* ARWT_CLIENT_CONN_HANDLER_HPP_ */
