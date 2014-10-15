/*
 * rvariant.hpp
 *
 *  Created on: Feb 27, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef ARWT_RVARIANT_HPP_
#define ARWT_RVARIANT_HPP_

#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <map>
#include <sstream>
#include <msgpack.hpp>
#include <typeinfo>
//include <type_info>
//namespace
namespace arawat {
namespace util {
const std::string TRUE_STR("true");
const std::string FALSE_STR("false");

namespace {
//only available locally
typedef std::vector<boost::recursive_variant_> vector_rvariant_t;
typedef std::map<std::string, boost::recursive_variant_> map_rvariant_t;
}
typedef boost::make_recursive_variant<bool, boost::uint8_t, boost::uint32_t,
		boost::int32_t, double, std::string, boost::uuids::uuid,
		vector_rvariant_t, map_rvariant_t>::type rvariant_type;

typedef boost::make_recursive_variant<bool, boost::uint8_t, boost::uint32_t,
		boost::int32_t, double, std::string, boost::uuids::uuid>::type rvariant_type_1;

typedef std::map<std::string, rvariant_type > map_str_rvariant_type;
typedef std::vector<rvariant_type> vector_rvariant_type;

#if 0
template <typename T>
std::ostream& operator<<(std::ostream& out, const  vector_rvariant_type & t ) {
	//msgpack::sbuffer sbuf;
	//std::stringstream ss;
	//ss << t;
	out << t;
	//msgpack::pack(sbuf, ss.str());
	return out;
}
#endif
struct variant_serializer : boost::static_visitor<> {

    template <typename T>
    void operator()( const T & t )  {
     // msgpack::sbuffer sbuf;
      msgpack::pack(sbuf_, t);
    //  return sbuf;
    }

    void operator()( const boost::uuids::uuid & t )  {
        //  msgpack::sbuffer sbuf;
          std::stringstream ss;
          ss << t;
          msgpack::pack(sbuf_, ss.str());
        //  return sbuf;
    }
#if 0
    void operator()( const vector_rvariant_type & t )  {

    	     std::stringstream ss;
    	     for(unsigned i=0; i < t.size();++i) {
    	    	 variant_serializer v;
    	    	 boost::apply_visitor(v, t[i]);
    	    	 ss<< v.sbuf_;
    	    	// msgpack::pack(sbuf_, v.sbuf_);
    	     }
          //   std::stringstream ss;
           //  ss << t;
             msgpack::pack(sbuf_, ss.str());
          //   return sbuf;
       }
#endif
    msgpack::sbuffer sbuf_;

};



#if 0
template<typename T>
class rvariant_visitor: public boost::static_visitor<>
/*: public boost::static_visitor<>,
 public boost::static_visitor<int>,
 public boost::static_visitor<double>,
 public boost::static_visitor<std::string>,
 public boost::static_visitor<boost::uuids::uuid>,
 public boost::static_visitor<vector_rvariant_t>,
 public boost::static_visitor<map_rvariant_t>
 */
{
public:

	//	T operator()(const T& t) const {
	//		return t;
	//	}

};
#endif



/**
 * boolean
 */
class rvariant_visitor : public boost::static_visitor<bool> ,
 public boost::static_visitor<boost::uint8_t>,
 public boost::static_visitor<boost::uint32_t>,
 public boost::static_visitor<boost::int32_t>,
 public boost::static_visitor<double>,
 public boost::static_visitor<std::string>,
 public boost::static_visitor<boost::uuids::uuid> {
#if 0
 public boost::static_visitor<vector_rvariant_t>,
 public boost::static_visitor<map_rvariant_t> {
#endif
public:
	bool operator()(bool t) const {
		return t;
	}

	boost::uint8_t operator()( boost::uint8_t t) const {
			return t;
	}

	boost::uint32_t operator()(boost::uint32_t t) const {
		return t;
	}

	boost::int32_t operator()(boost::int32_t t) const {
			return t;
	}

	double operator()(double t) const {
		return t;
	}

	std::string operator()(const std::string& s) const {
		return s;
	}
	boost::uuids::uuid operator()(const boost::uuids::uuid& uid) const {
		return uid;
	}
#if 0
	vector_rvariant_t operator()(const vector_rvariant_t& v) const {
			return v;
	}
	map_rvariant_t operator()(const map_rvariant_t& m) const {
		return m;
	}
#endif

};

#if 0
int operator()(int i) const {
	return i;
}
//	int operator()(const std::string& s) const {
//		return boost::lexical_cast<int>(s);
//	}
//double
double operator()(double i) const {
	return i;
}
//	double operator()(const std::string& s) const {
//		return boost::lexical_cast<double>(s);
//	}


//string
std::string operator()(const std::string& s) const {
	return s;
}

//	std::string operator()(int i) const {
//		return boost::lexical_cast<std::string>(i);
//	}

//	std::string operator()(double i) const {
//		return boost::lexical_cast<std::string>(i);
//	}

//	std::string operator()(bool b) const {
//		if (b) {
//			return TRUE_STR;
//		}
//		return FALSE_STR;
//	}
//	bool operator()(const std::string& s) const {
//		std::string sl = s;
//		boost::algorithm::to_lower(sl);
//		if (s == TRUE_STR) {
//			return true;
//		} else {
//			return false;
//		}
//	}
//uuid

//	std::string operator()(const boost::uuids::uuid& uid) const {
//			std::stringstream ss;
//			ss << uid;
//			return ss.str();
//	}

//vector


//	std::string operator()(const vector_rvariant_t& v) const {
//		std::string vs = "(";
//		for (vector_rvariant_t::it = v.begin(); it != v.end(); ++it) {
//			vs += " ";
//			vs += boost::apply_visitor(rvariant_visitor(), *it);
//		}
//		vs += ")";
//		return vs;
//	}

//map
//	map_rvariant_t operator()(const map_rvariant_t& m) const {
//		return m;
//	}

//	std::string operator()(const map_rvariant_t& m) const {
//		std::string ms = "( ";
//		for (vector_rvariant_t::it = v.begin(); it != v.end(); ++it) {
//			vs += "( " + it->first + ",";
//			vs += boost::apply_visitor(rvariant_visitor(), it->second);
//			vs += ")";
//		}
//		vs += " )";
//		return vs;
//	}
};
#endif

class rvariant {
public:

	template <typename T>
	rvariant(const T& data) {
		data_ = data;
		type_ =  (std::type_info*)&typeid(data);

	}

	template <typename T>
		rvariant& operator=(const T& data) {
		data_ = data;
		type_ = (std::type_info*)&typeid(data);
		return *this;
	}

	template<typename T> T get() {
		return boost::get<T>(data_);
		// T *t  = (T*)(&data_);
		// return *t;
		 //return (T) (data_.which())(data_);
	}

	template <bool>
		rvariant(bool data) {
		data_ = data;
	}
	template <bool>
		rvariant& operator=(bool data) {
		data_ = data;
		return *this;
	}
    void encode( msgpack::sbuffer& sbuf) {
    	variant_serializer v;
    	boost::apply_visitor( v, data_);
			//msgpack::pack(sbuf, boost::get<type_>(data_));
    	 //   variant_serializer s;
    	 //   boost::apply_visitor( s, data_);

	 }

private:
    rvariant_type_1 data_;
	std::type_info* type_;
	//MSGPACK_DEFINE(data_);
};



}//util
}//arawat

#endif /* ARWT_RVARIANT_HPP_ */
