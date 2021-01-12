#pragma once

#include <string>
#include <ostream>

namespace ascript {

//!More like a "value". Represents variables, parameters, return values...
/**
* The implementation is easy (a member for each type) though likely very 
* ineficcient.
*/
struct variable {

	//!Each variable can be of a given type.
	enum class              types{
		boolean, 
		integer, 
		string, 
		decimal, 
		symbol
	} type{types::integer}; //!< Current type.

	//!Class constructor for booleans.
	                        variable(bool);
	//!Class constructor for integers.
	                        variable(int);
	//!Class constructor for doubles.
	                        variable(double);
	//!Class constructor for strings.
	                        variable(const std::string&);
	//!Class constructor for strings.
	                        variable(const char);
	//!Class constructor for strings.
	                        variable(const char *);
	//!Hacky class constructor for a symbol, it does not really matter what the types parameter express.
	                        variable(const std::string&, types);
	//!Comparison operator. These are quite stringent and will want the types to match.
	bool                    operator==(const variable&) const;
	//!Unequality operator.
	bool                    operator!=(const variable& _other) const {return !(*this==_other);}
	//!Aritmetic operator which only works on numeric types.
	bool                    operator<(const variable&) const;
	//!Aritmetic operator which only works on numeric types.
	bool                    operator>(const variable&) const;
	//!Aritmetic operator which only works on numeric types.
	variable                operator+(const variable&) const;
	//!Aritmetic operator which only works on numeric types.
	variable                operator-(const variable&) const;
	bool                    bool_val{false}; //!<Boolean value
	int                     int_val{0}; //!<Integer value
	double                  double_val{0.}; //!<Double value
	std::string             str_val; //!<String value
};

std::ostream& operator<<(std::ostream&, const variable&);

}
