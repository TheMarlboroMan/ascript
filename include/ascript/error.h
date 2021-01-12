#pragma once

#include <sstream>

namespace ascript {

/**
* Generic exception thrown by the library.
**/
struct ascript_error:std::runtime_error {

	//!Class constructor.
	                        ascript_error(const std::string& _msg):std::runtime_error(_msg){}
};

/**
* Generic exception thrown by the library.
**/
struct parser_error:ascript_error {

	//!Class constructor.
	                        parser_error(const std::string& _msg):ascript_error(_msg){}
};

/**
* Exception thrown at parse time (stuff that does not make sense).
**/
struct interpreter_error:ascript_error {

	//!Class constructor.
	                        interpreter_error(const std::string& _msg):ascript_error(_msg){}
};

/**
* Exception thrown when a script invokes "fail".
**/
struct user_error:ascript_error {

	//!Class constructor.
	                        user_error(const std::string& _msg):ascript_error(_msg){}
};

/**
* Exception that must be thrown by user implementations of the host.
**/
struct host_error:ascript_error {

	//!Class constructor.
	                        host_error(const std::string& _msg):ascript_error(_msg){}
};

/**
* Helper structure to be passed to the error_builder component that throws an 
* exception. Just exists for ease of use, so we can build error messages with
* ease.
**/
struct throw_err {

	int                     line_number; //!<Line number in which the error happens.
	enum class types        {
		parser, 
		interpreter, 
		user, 
		host
	} type; //!<Error type, determines the type of exception to be thrown.
};

/**
* Simple class to build error messages. Get one git "get", append error 
* information with << and append a throw_err structure to throw.
**/
struct error_builder {

	//!Procures an error builder object.
	static error_builder    get() {return {std::stringstream{""}};}

	//!Adds error information.
	template<typename T>
	error_builder&          operator<<(const T& _val) {

		ss<<_val;
		return *this;
	}

	//!Adds the line and error type and throws.
	void                    operator<<(const throw_err& _val) {

		ss<<" on line "<<_val.line_number;

		switch(_val.type) {
			case throw_err::types::parser:
				throw parser_error(ss.str());
			case throw_err::types::interpreter:
				throw interpreter_error(ss.str());
			case throw_err::types::user:
				throw user_error(ss.str());
			case throw_err::types::host:
				throw host_error(ss.str());
		}
	}

	private:

	//!Internal stream.
	std::stringstream       ss;
};

}
