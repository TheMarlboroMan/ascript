#pragma once

#include <sstream>

namespace ascript {

struct ascript_error:std::runtime_error {

	                        ascript_error(const std::string& _msg):std::runtime_error(_msg){}
};

struct parser_error:ascript_error {

	                        parser_error(const std::string& _msg):ascript_error(_msg){}
};

struct interpreter_error:ascript_error {

	                        interpreter_error(const std::string& _msg):ascript_error(_msg){}
};

struct user_error:ascript_error {

	                        user_error(const std::string& _msg):ascript_error(_msg){}
};

struct throw_err {

	int                     line_number;
	enum class types        {parser, interpreter, user} type;
};

struct error_builder {


	std::stringstream       ss;
	
	template<typename T>
	error_builder&          operator<<(const T& _val) {

		ss<<_val;
		return *this;
	}

	void                    operator<<(const throw_err& _val) {

		ss<<" on line "<<_val.line_number;

		switch(_val.type) {
			case throw_err::types::parser:
				throw parser_error(ss.str());
			case throw_err::types::interpreter:
				throw interpreter_error(ss.str());
			case throw_err::types::user:
				throw user_error(ss.str());
		}
	}

	static error_builder    get() {return {std::stringstream{""}};}
};

}
