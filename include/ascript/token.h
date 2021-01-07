#pragma once

#include <string>
#include <ostream>

namespace ascript {

struct token {

	enum class types {
		identifier,
		val_string,
		val_bool,
		val_int,
		fn_is_equal, //done
		fn_is_greater_than, //done
		fn_is_lesser_than, //done
		fn_is_int, //done
		fn_is_bool,//done
		fn_is_double, //done
		fn_is_string, //done
		fn_host_has, //done
		fn_host_get, //done
		fn_host_query, //done
		pr_host_set, //done
		pr_host_add, //done
		pr_host_do, //done
		pr_out, //done
		pr_fail, //done
		kw_not, //done
		kw_if, //done
		kw_elseif, //done
		kw_else, //done
		kw_endif, //done
		kw_loop, //done
		kw_break, //done
		kw_endloop, //done
		kw_yield, //done
		kw_return, //done
		kw_let, //done
		kw_be, //done
		kw_integer,
		kw_string,
		kw_bool,
		kw_as,
		kw_beginfunction, //done
		kw_endfunction, //done
		semicolon,
		comma,
		open_bracket,
		close_bracket
	};

	types       type;
	std::string str_val;
	int         int_val{0};
	bool        bool_val{false};
	int         line_number{0};
};

std::string type_to_str(token::types);
std::ostream& operator<<(std::ostream&, const token&);
}
