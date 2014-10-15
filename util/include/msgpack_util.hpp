/*
 * msgpack_util.hpp
 *
 *  Created on: Feb 28, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef ARWT_MSGPACK_UTIL_HPP_
#define ARWT_MSGPACK_UTIL_HPP_

#include <boost/cstdint.hpp>
#include<boost/noncopyable.hpp>
#include <msgpack.hpp>

namespace arawat {
namespace util {
/**
 * msgpack utility class which encode/decode custom classes into msgpack format
 */
template<typename T>
class msgpack_util : boost::noncopyable{
public:
	/**
	 * encode
	 * @param t
	 * @param sbuf
	 */
	static  void encode(const T& t, msgpack::sbuffer& sbuf) {
			msgpack::pack(sbuf, t);
	}

	/**
	 * decode
	 * @param sbuf
	 * @param req
	 */
	static void decode(const msgpack::sbuffer& sbuf, T& req) {
			msgpack::zone zone;
			msgpack::object obj;
			msgpack::unpack(sbuf.data(), sbuf.size(), NULL, &zone, &obj);
			obj.convert(&req);
		}
};

}
}


#endif /* ARWT_MSGPACK_UTIL_HPP_ */
