#include "ascript/return_value.h"
#include "ascript/error.h"

#include <stdexcept>

using namespace ascript;

return_value::return_value(
	const variable& _var
):
	type{types::value},
	value{_var}
{

}

return_value::return_value(
	types _type
):
	type{_type}
{

	if(_type==return_value::types::value) {

		throw std::runtime_error("cannot specify 'value' for return type without a value itself");
	}
}

return_value::operator bool() const {

	return type==return_value::types::value;
}

bool return_value::has_value() const {

	return (bool)*this;
}

bool return_value::is_nothing() const {

	return type==return_value::types::nothing;
}

bool return_value::is_yield() const {

	return type==return_value::types::yield;
}

const variable& return_value::get() const {

	if(type!=return_value::types::value 
		|| !value) {

		throw ascript_error("cannot call get on non-value return type");
	}

	return value.value();
}
