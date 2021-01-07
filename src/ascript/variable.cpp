#include "ascript/variable.h"

using namespace ascript;

variable::variable(
	bool _val
):
	type{types::boolean},
	bool_val{_val},
	int_val{0},
	double_val{0.}
{}

variable::variable(
	int _val
):
	type{types::integer},
	bool_val{false},
	int_val{_val},
	double_val{0.}
{}

variable::variable(
	double _val
):
	type{types::integer},
	bool_val{false},
	int_val{0},
	double_val{_val}
{}

variable::variable(
	const std::string& _val
):
	type{types::string},
	bool_val{false},
	int_val{0},
	double_val{0.},
	str_val{_val}
{}

variable::variable(
	const std::string& _identifier,
	types /*_unused*/
):
	type{types::symbol},
	bool_val{false},
	int_val{0},
	double_val{0.},
	str_val{_identifier}
{}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const variable& _var
) {
	
	switch(_var.type) {

		case variable::types::integer:
			_stream<<"integer:"<<_var.int_val;
			return _stream;
		case variable::types::boolean:
			_stream<<"boolean:"<<_var.bool_val;
			return _stream;
		case variable::types::string:
			_stream<<"string:"<<_var.str_val;
			return _stream;
		case variable::types::decimal:
			_stream<<"double:"<<_var.double_val;
			return _stream;
		case variable::types::symbol:
			_stream<<"symbol:"<<_var.str_val;
			return _stream;
	}
	
	return _stream;
}

