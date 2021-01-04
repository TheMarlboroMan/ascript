#pragma once

#include <string>
#include <ostream>

namespace script {

struct token {

	enum class types {
		identifier,
		val_string,
		val_bool,
		val_int,
		fn_is_equal,
		fn_is_greater_than,
		fn_is_lesser_than,
		fn_host_has,
		fn_host_add,
		fn_host_get,
		fn_host_set,
		fn_host_query,
		fn_host_do,
		fn_out, //done
		fn_fail, //done
		kw_not,
		kw_if, 
		kw_elseif,
		kw_else,
		kw_endif,
		kw_loop,
		kw_break,
		kw_endloop,
		kw_yield,
		kw_return, //done
		kw_let,
		kw_be,
		kw_integer,
		kw_string,
		kw_bool,
		kw_as,
		kw_beginscript,
		kw_endscript,
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
