#include "ascript/variable.h"

#include <stdexcept>

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
	type{types::decimal},
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
	const char _val
):
	type{types::string},
	bool_val{false},
	int_val{0},
	double_val{0.},
	str_val{_val}
{}

variable::variable(
	const char * _val
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

bool variable::operator==(
	const variable& _other
) const {

	if(type!=_other.type) {

		return false;
	}

	switch(type) {

		case variable::types::integer:
			return int_val==_other.int_val;
		case variable::types::boolean:
			return bool_val==_other.bool_val;
		case variable::types::string:
		case variable::types::symbol:
			return str_val==_other.str_val;
		case variable::types::decimal:
			return double_val==_other.double_val;
	}

	return false;
}

bool variable::operator<(
	const variable& _other
) const {

	if(type!=_other.type) {

		throw std::runtime_error("lesser than type mismatch");
	}

	switch(type) {

		case variable::types::integer:
			return int_val < _other.int_val;
		case variable::types::decimal:
			return double_val < _other.double_val;
		case variable::types::boolean:
		case variable::types::string:
		case variable::types::symbol:
			throw std::runtime_error("lesser than is only applicable to numeric types");
	}

	return false;
}

bool variable::operator>(
	const variable& _other
) const {

	if(type!=_other.type) {

		throw std::runtime_error("greater than type mismatch");
	}

	switch(type) {

		case variable::types::integer:
			return int_val > _other.int_val;
		case variable::types::decimal:
			return double_val > _other.double_val;
		case variable::types::boolean:
		case variable::types::string:
		case variable::types::symbol:
			throw std::runtime_error("greater than is only applicable to numeric types");
	}

	return false;
}

variable variable::operator+(
	const variable& _other
) const {

	if(type!=_other.type) {

		throw std::runtime_error("addition type mismatch");
	}

	switch(type) {

		case variable::types::integer:
			return int_val+_other.int_val;
		case variable::types::decimal:
			return double_val+_other.double_val;
		case variable::types::boolean:
		case variable::types::string:
		case variable::types::symbol:
			throw std::runtime_error("addition is only applicable to numeric types");
	}

	return false;
}

variable variable::operator-(
	const variable& _other
) const {

	if(type!=_other.type) {

		throw std::runtime_error("substraction type mismatch");
	}

	switch(type) {

		case variable::types::integer:
			return int_val-_other.int_val;
		case variable::types::decimal:
			return double_val-_other.double_val;
		case variable::types::boolean:
		case variable::types::string:
		case variable::types::symbol:
			throw std::runtime_error("substraction is only applicable to numeric types");
	}

	return false;
}
