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
		fn_is_equal, //done
		fn_is_greater_than, //done
		fn_is_lesser_than, //done
		fn_host_has,
		fn_host_get,
		fn_host_query,
		pr_host_set,
		pr_host_add,
		pr_host_do,
		pr_out, //done
		pr_fail, //done
		kw_not,
		kw_if, 
		kw_elseif,
		kw_else,
		kw_endif,
		kw_loop,
		kw_break, //done
		kw_endloop,
		kw_yield, //done
		kw_return, //done
		kw_let, //done
		kw_be, //done
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
