/*
 * conn_mgr.hpp
 *
 *  Created on: Mar 5, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_CONNECTION_MGR_HPP_
#define JOSHI_CONNECTION_MGR_HPP_
#include <vector>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/signal.hpp>

#include <proto_data_interface.hpp>
#include <log.hpp>
#include <boost_logger.hpp>
namespace arawat {
namespace protocol {

/**
 * connection manager
 */
template<typename T, typename C>
class conn_mgr: boost::noncopyable {
public:


	/**
	 * connection ptr
	 */
	typedef typename boost::shared_ptr<T> conn_ptr_t;

	/**
	 * config list
	 */
	typedef typename std::vector<C> config_list_t;

	/**
	 * connection table type
	 * It's a multi index container
	 */
	typedef  boost::multi_index::multi_index_container<
			conn_ptr_t,
			boost::multi_index::indexed_by<
					//sequenced<>,
					boost::multi_index::hashed_unique<
							BOOST_MULTI_INDEX_CONST_MEM_FUN(T, std::string, id)>,
					boost::multi_index::hashed_non_unique<
							BOOST_MULTI_INDEX_CONST_MEM_FUN(T, std::string,
									type)>,
					boost::multi_index::hashed_non_unique<
							boost::multi_index::composite_key<conn_ptr_t,
									BOOST_MULTI_INDEX_CONST_MEM_FUN(T,
											std::string, id),
									BOOST_MULTI_INDEX_CONST_MEM_FUN(T,
											std::string, type)> > > >
			conn_table_t;

	//typedef for ConnectionIdView
	typedef  typename conn_table_t::template nth_index< 0 >::type conn_table_by_id;
	//typedef for ConnectionType view
	typedef typename conn_table_t::template nth_index< 1 >::type conn_table_by_type;
	//typedef for SCAddress view
	typedef typename conn_table_t::template nth_index< 2 >::type conn_table_by_id_type;

	/**
	 * call back function for data and connection id
	 */
	typedef boost::signal<bool(const proto_data_interface&, const std::string&)> signal_msg_type;
	typedef signal_msg_type::slot_type observer_msg_type;

	/**
	 * constructor
	 * @return
	 */
	conn_mgr():conn_id_view_ptr_(NULL),conn_type_view_ptr_(NULL), conn_id_type_view_ptr_(NULL) {
		 LOG_IN("");
		 LOG_OUT("");
	}

	~conn_mgr() {
		LOG_IN("");

		conn_type_view_ptr_ = NULL;
		conn_id_type_view_ptr_ = NULL;
		conn_id_view_ptr_ = NULL;
#if 0
		LOG_TRACE("conn_table_ size[%d]", conn_table_.size());
		typename conn_table_by_id::iterator it =  conn_table_.begin();
		for(; it != conn_id_view_ptr_->end(); ++it) {
			conn_ptr_t p = *it;
			LOG_INFO("id[%s], Use count[%d]", ((conn_ptr_t)*it)->id().c_str(), ((conn_ptr_t)*it).use_count());
			conn_id_view_ptr_->erase(it);
			LOG_INFO("Use count[%d]", ((conn_ptr_t)*it).use_count());

		}
#endif
		LOG_TRACE("conn_table_ size[%d]", conn_table_.size());


		LOG_OUT("");
	}
	/**
	 * init
	 * @param c
	 * @return
	 */
	result_t init(const config_list_t& configs, const observer_msg_type& observer) {
		LOG_IN("size of config[%d]", configs.size());

		BOOST_FOREACH(const C& config, configs) {
			conn_ptr_t cptr(new T(config, observer));
			result_t r = cptr->init();
			if (r.get<0> ()) {
				LOG_TRACE("Inserting connection with id[%s]", cptr->id().c_str());
				conn_table_.insert(cptr);
				LOG_INFO("Initialize connection[%s] successfully",
										config.id_.c_str());
			} else {
				LOG_ERROR("Failed to initialize connection[%s]. Error:[%s]",
						config.id_.c_str(), r.get<1> ().c_str());
				conn_table_.clear();
				return r;
			}
		}
		conn_id_view_ptr_ = &conn_table_.template get<0>();
		conn_type_view_ptr_ = &conn_table_.template get<1>();
		conn_id_type_view_ptr_ = &conn_table_.template get<2>();

		LOG_RET_RESULT(result_t(true, "Successfully initialized"));
	}

	/**
	 * start
	 */
	void start() {
		LOG_IN("Starting...");
		BOOST_FOREACH (conn_ptr_t conn_ptr, *conn_id_view_ptr_) {
			conn_ptr->start();
		}
		LOG_OUT();

	}
	/**
	 * stop
	 */
	void stop(bool force=false) {
		LOG_IN("Stopping...");
		BOOST_FOREACH (conn_ptr_t conn_ptr, *conn_id_view_ptr_) {
			conn_ptr->stop(force);
		}
		LOG_OUT();

	}

	/**
	 * get connection by id
	 * @param id
	 * @return
	 */
	conn_ptr_t get_conn_by_id(const std::string& id) {
		LOG_IN("id[%s]", id.c_str());
		conn_ptr_t conn_ptr((T*)NULL);
		if (!conn_id_view_ptr_->size()) {
			LOG_WARN("Connection id view size is 0");
			return conn_ptr;
		}

		typename conn_table_by_id::const_iterator it =  conn_id_view_ptr_->find(id);
		while (it != conn_id_view_ptr_->end()) {

			if (it->get() && (*it)->running()) {
				conn_ptr = *it;
				LOG_TRACE("Found connection with id[%s]",
						conn_ptr->id().c_str());
				LOG_TRACE("Returning %p", conn_ptr.get());
				return conn_ptr;
			}
			it++;
		}
		LOG_TRACE("Returning %p", conn_ptr.get());
		return conn_ptr;
	}
	/**
	 * get connection by type
	 * @param type
	 * @return
	 */
	conn_ptr_t get_conn_by_type(const std::string& type) {
		LOG_IN("type[%s]", type.c_str());
		conn_ptr_t conn_ptr((T*)NULL);
		if (!conn_type_view_ptr_->size()) {
			LOG_TRACE("Condtion type container view size is 0. returning null.");
			return conn_ptr;
		}

		typename conn_table_by_type::const_iterator it = conn_type_view_ptr_->find(
				type);
		if (it == conn_type_view_ptr_->end()) {
			LOG_WARN("No connection found with type[%s]. Returning null",
					type.c_str());
			return conn_ptr;
		}

		unsigned total = conn_type_view_ptr_->size();
		static unsigned last_index = 0;
		unsigned current_index = last_index;
		//if last used index is last one from the container, reset to zero else increment
		{
			boost::mutex::scoped_lock guard(mtx_type_);
			if (last_index >= (total - 1)) {
				last_index = 0;
				current_index = 0;
			} else {
				current_index = last_index;
			}
		}
		unsigned tmp = 0;
		//now, we need to iterate to the current index in the container
		while (tmp++ < current_index) {
			if (it == conn_type_view_ptr_->end()) {
				it = conn_type_view_ptr_->begin();
			}
			++it;
		}
		while (total--) {
			if (it == conn_type_view_ptr_->end()) {
				it = conn_type_view_ptr_->begin();
			}
			if (it->get() && (*it)->running()) {
				conn_ptr = *it;
				LOG_TRACE("Found connection with type[%s]",
						conn_ptr->type().c_str());
				LOG_TRACE("Returning %p", conn_ptr.get());
				{
					boost::mutex::scoped_lock guard(mtx_type_);
					last_index++;
				}
				return conn_ptr;
			}
			it++;
		}
		LOG_TRACE("Returning %p", conn_ptr.get());
		return conn_ptr;
	}
	/**
	 * get connection by give id and type
	 * @param id
	 * @param id
	 * @return
	 */
	conn_ptr_t get_conn_by_id_type(const std::string& id,
			const std::string& type) {
		LOG_IN("id[%s], type[%s]", id.c_str(), type);
		conn_ptr_t conn_ptr((T*)NULL);
		if (!conn_id_type_view_ptr_->size()) {
			return conn_ptr;
		}

		typename conn_table_by_id_type::const_iterator it =
				conn_id_type_view_ptr_->find(boost::make_tuple(id, type));
		while (it != conn_id_type_view_ptr_->end()) {
			if (it->get() && (*it)->running()) {
				conn_ptr = *it;
				LOG_TRACE("Found connection with id[%s], type[%s]",
						conn_ptr->id().c_str(), conn_ptr->type().c_str());
				LOG_TRACE("Returning %p", conn_ptr.get());
				return conn_ptr;
			}
			it++;
		}
		LOG_TRACE("Returning %p", conn_ptr.get());
		return conn_ptr;
	}


private:

	conn_table_t conn_table_;


	boost::mutex mtx_type_;
	conn_table_by_id* conn_id_view_ptr_;
	conn_table_by_type* conn_type_view_ptr_;
	conn_table_by_id_type* conn_id_type_view_ptr_;


};
}
}

#endif /* JOSHI_CONNECTION_MGR_HPP_ */
