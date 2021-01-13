#pragma once

#include "ascript/variable.h"

#include <optional>

namespace ascript {

//!value to be returned from a function to the calling environment.
/**
* A function might or might not return a value, additionally it might also 
* not return a value *yet* but might return it later (because of a yield
* statement).
*/

class return_value {

	public:

	//!Indicates the type of returned value.
	enum class types{value, nothing, yield};

	//!class constructor
	                return_value(const variable&);

	//!class constructor
	                return_value(types);

	//!returns true if there's a value to collect (not yielding, nor nothing).
	                operator bool() const;

	//!alias for the bool operator.
	bool            has_value() const;

	//!returns true if no return value was specified but the function returned.
	bool            is_nothing() const;

	//!returns true if there's no return value because of a yield statement.
	bool            is_yield() const;

	//!returns the value (if any, throws if not).
	const variable& get() const;

	private:

	types           type; //!<Holds the type of return value.
	std::optional<variable> value; //<!Holds the value.
};

}
