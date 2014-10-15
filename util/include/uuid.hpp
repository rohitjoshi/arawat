/*
 * uuid.hpp
 *
 *  Created on: Mar 11, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_UUID_HPP_
#define JOSHI_UUID_HPP_
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
namespace arawat {
namespace util {
/**
 * class provide utility function to generate uuid
 */
class uuid : boost::noncopyable {
public:
	/**
	 * generate random string uuid
	 * @return
	 */
	static std::string gen_uid_str() {
		std::stringstream ss;
		ss << gen_uid_uuid();
		return ss.str();
	}
	/**
	 * generate random boost::uuid
	 * @return
	 */
	static boost::uuids::uuid  gen_uid_uuid() {
		static boost::uuids::random_generator gen;
		boost::uuids::uuid uid = gen();
		return uid;
	}
};
}
}

#endif /* JOSHI_UUID_HPP_ */
