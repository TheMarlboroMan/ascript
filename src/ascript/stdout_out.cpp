#include "ascript/stdout_out.h"

#include <iostream>
#include <stdexcept>

using namespace ascript;

void stdout_out::out(
	const variable& _arg
) {

	//The overloads are, so far, for debugging purposes so...
	switch(_arg.type) {
		case variable::types::boolean: std::cout<<(_arg.bool_val ? "true" : "false"); break;
		case variable::types::integer: std::cout<<_arg.int_val; break;
		case variable::types::string: std::cout<<_arg.str_val; break;
		case variable::types::decimal: std::cout<<_arg.double_val; break;
		case variable::types::symbol: 
			throw std::runtime_error("should never happen");
	}
}

void stdout_out::flush() {

	std::cout<<std::endl;
}



