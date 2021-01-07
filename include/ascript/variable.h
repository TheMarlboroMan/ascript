#pragma once

#include <string>
#include <ostream>

namespace ascript {

struct variable {

	enum class              types{boolean, integer, string, decimal, symbol} type{types::integer};

	                        variable(bool);
	                        variable(int);
	                        variable(double);
	                        variable(const std::string&);
	                        variable(const std::string&, types); //hacky symbol constructor.
	bool                    bool_val{false};
	int                     int_val{0};
	double                  double_val{0.};
	std::string             str_val;
};

std::ostream& operator<<(std::ostream&, const variable&);

}
