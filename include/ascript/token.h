#pragma once

#include <string>
#include <ostream>

namespace ascript {

//!Tokens derived from the first pass through a program file. Each word or
//!meaningful symbol is translated to one of these.
/**
*Certain tokens might not only represent an idea (such as "open parameters") but
*also a value (e.g. a literal string contains the idea of being a literal 
*string, but also the string itself. These are represented through dedicated
*(if likely unneficient) members.
*/
struct token {

	//!Token types.
	enum class types {
		identifier,
		val_string,
		val_bool,
		val_int,
		val_double,
		fn_is_equal,
		fn_is_greater_than,
		fn_is_lesser_than,
		fn_add,
		fn_substract,
		fn_is_int,
		fn_is_bool,//done
		fn_is_double,
		fn_is_string,
		fn_host_has,
		fn_host_get,
		fn_host_query,
		pr_host_delete,
		pr_host_set,
		pr_host_add,
		pr_host_do,
		pr_out,
		pr_fail,
		kw_not,
		kw_if,
		kw_elseif,
		kw_else,
		kw_endif,
		kw_loop,
		kw_break,
		kw_endloop,
		kw_yield,
		kw_return,
		kw_exit,
		kw_let,
		kw_be,
		kw_set,
		kw_to,
		kw_integer,
		kw_string,
		kw_bool,
		kw_double,
		kw_anytype,
		kw_as,
		kw_beginfunction,
		kw_endfunction,
		semicolon,
		comma,
		open_bracket,
		close_bracket
	};

	types       type; //!< Current token type.
	std::string str_val; //!< String value, if any.
	int         int_val{0}; //!< Integer value, if any.
	double      double_val{0.}; //!< Double value, if any.
	bool        bool_val{false}; //!< Boolean value, if any.
	int         line_number{0}; //!< Stores the line where the word originating the token was.
};

//!Converts a token type to a readable string representation.
std::string type_to_str(token::types);

//!output overload for a token, for debug purposes.
std::ostream& operator<<(std::ostream&, const token&);
}
