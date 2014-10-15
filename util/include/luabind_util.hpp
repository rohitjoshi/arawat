/*
 * luabind_util.hpp
 *
 *  Created on: Mar 4, 2012
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_LUABIND_UTIL_HPP_
#define JOSHI_LUABIND_UTIL_HPP_

//test.cpp
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <luabind/luabind.hpp>
#include <luabind/function.hpp>
#include "log.hpp"
//namespace
namespace arawat {
namespace util {
using namespace luabind;

/**
 * luabind utility
 */
class luabind_util {
public:

	/**
	 * Load file
	 * @param file
	 * @return
	 */
	static result_t load_file(const std::string& file) {
		//load file
		int error = 0;
		try {
			if ((error = luaL_loadfile(instance().lua_state_, file.c_str()))
					!= 0) {
				LOG_RET_RESULT(result_t(false, std::string(lua_tostring(instance().lua_state_, error))));
			}
		} catch (luabind::error& ex) {
			std::string err("Failed to load file:" + file
					+ ". Exception:luaL_loadfile():");
			LOG_RET_RESULT(result_t(false, err + ex.what()));

		} catch (std::exception& ex) {
			std::string err("Failed to load file:" + file
					+ ". Exception:luaL_loadfile():");
			LOG_RET_RESULT(result_t(false, err + ex.what()));

		}
		//pcall
		try {
			if ((error = lua_pcall(instance().lua_state_, 0, LUA_MULTRET, 0))
					!= 0) {
				LOG_RET_RESULT(result_t(false, std::string(lua_tostring(instance().lua_state_, error))));
			}
		} catch (luabind::error& ex) {
			std::string err("Failed to load file:" + file
					+ ". Exception:luaL_loadfile():");
			LOG_RET_RESULT(result_t(false, err + ex.what()));

		} catch (std::exception& ex) {
			std::string err("Failed to load file:" + file
					+ ". Exception:lua_pcall():");
			LOG_RET_RESULT(result_t(false, err + ex.what()));
		}

		LOG_RET_RESULT(result_t(true, "success"));
	}

	/**
	 * Load file
	 * @param luastr
	 * @return
	 */
	static result_t load_string(const std::string& luastr) {

		//load string
		int error = 0;
		try {
			if ((error = luaL_loadstring(instance().lua_state_, luastr.c_str()))
					!= 0) {
				LOG_RET_RESULT(result_t(false, std::string(lua_tostring(instance().lua_state_, error))));
			}
		} catch (luabind::error& ex) {
			std::string err("Failed to load string:" + luastr
					+ ". Exception:luaL_loadstring():");
			LOG_RET_RESULT(result_t(false, err + ex.what()));
		} catch (std::exception& ex) {
			std::string err("Failed to load string:" + luastr
					+ ". Exception:luaL_loadstring():");
			LOG_RET_RESULT(result_t(false, err + ex.what()));

		}
		//pcall
		try {
			if ((error = lua_pcall(instance().lua_state_, 0, LUA_MULTRET, 0))
					!= 0) {
				LOG_RET_RESULT(result_t(false, std::string(lua_tostring(instance().lua_state_, error))));
			}
		} catch (luabind::error& ex) {
			std::string err("Failed to load string:" + luastr
					+ ". Exception:luaL_loadstring():");
			LOG_RET_RESULT(result_t(false, err + ex.what()));

		} catch (std::exception& ex) {
			std::string err("Failed to load string:" + luastr
					+ ". Exception:lua_pcall():");
			LOG_RET_RESULT(result_t(false, err + ex.what()));
		}

		LOG_RET_RESULT(result_t(true, "success"));
	}

	/**
	 * get lua state
	 * @return
	 */
	static lua_State* get_lua_state() {
		return instance().lua_state_;
	}

private:
	/**
	 * constructor
	 * @return
	 */
	luabind_util() :
		lua_state_(NULL) {
		lua_state_ = lua_open();
		luaL_openlibs(lua_state_);
		//connect luabind to luad state
		luabind::open(lua_state_);
	}
	/**
	 * destructor
	 * @return
	 */
	~luabind_util() {
		lua_close(lua_state_);
	}

	/**
	 * get instance
	 * @return
	 */
	inline static luabind_util& instance() {
		static luabind_util instance_;
		return instance_;
	}

private:
	/**
	 * Lua state variable
	 */
	lua_State *lua_state_;

};
} //util
}//arawat
#endif /* JOSHI_LUABIND_UTIL_HPP_ */
