#include "ascript/token.h"

using namespace ascript;

std::string ascript::type_to_str(
	token::types _type
) {

	switch(_type) {
		case token::types::identifier: return "identifier"; 
		case token::types::val_string: return "string_value"; 
		case token::types::val_bool: return "boolean_value";
		case token::types::val_int: return "integer_value";
		case token::types::val_double: return "double_value";
		case token::types::fn_is_equal: return "fn_is_equal";
		case token::types::fn_is_greater_than: return "fn_is_greater_than"; 
		case token::types::fn_is_lesser_than: return "fn_is_lesser_than"; 
		case token::types::fn_add: return "fn_add";
		case token::types::fn_substract: return "fn_substract";
		case token::types::fn_is_int: return "fn_is_int";
		case token::types::fn_is_bool: return "fn_is_bool";
		case token::types::fn_is_double: return "fn_is_double";
		case token::types::fn_is_string: return "fn_is_string";
		case token::types::fn_host_has: return "fn_host_has";
		case token::types::fn_host_get: return "fn_host_get";
		case token::types::fn_host_query: return "fn_host_query";
		case token::types::pr_host_delete: return "pr_host_delete";
		case token::types::pr_host_set: return "pr_host_set";
		case token::types::pr_host_add: return "pr_host_add";
		case token::types::pr_host_do: return "pr_host_do";
		case token::types::pr_out: return "out";
		case token::types::pr_fail: return "fail";
		case token::types::kw_not: return "not";
		case token::types::kw_if: return "if";
		case token::types::kw_elseif: return "elseif";
		case token::types::kw_else: return "else";
		case token::types::kw_endif: return "endif";
		case token::types::kw_loop: return "loop";
		case token::types::kw_break: return "break";
		case token::types::kw_endloop: return "endloop";
		case token::types::kw_yield: return "yield"; 
		case token::types::kw_exit: return "exit";
		case token::types::kw_return: return "return";
		case token::types::kw_let: return "let";
		case token::types::kw_be: return "be";
		case token::types::kw_set: return "set";
		case token::types::kw_to: return "to";
		case token::types::kw_integer: return "kw_integer";
		case token::types::kw_string: return "kw_string";
		case token::types::kw_bool: return "kw_bool";
		case token::types::kw_double: return "kw_double";
		case token::types::kw_anytype: return "kw_anytype";
		case token::types::kw_as: return "as";
		case token::types::kw_call: return "call";
		case token::types::kw_beginfunction: return "beginfunction";
		case token::types::kw_endfunction: return "endfunction";
		case token::types::semicolon: return ";";
		case token::types::comma: return ",";
		case token::types::open_bracket: return "[";
		case token::types::close_bracket: return "]";
	}

	return "";
}

std::ostream& ascript::operator<<(
	std::ostream& _stream,
	const token& _token
) {

	_stream<<type_to_str(_token.type);

	if(_token.type==token::types::val_string) {

		_stream<<" (\""<<_token.str_val<<"\")";
	}
	else if(_token.type==token::types::val_bool) {

		_stream<<" (\""<<(_token.bool_val ? "true" : "false")<<"\")";
	}
	else if(_token.type==token::types::val_int) {

		_stream<<" (\""<<_token.int_val<<"\")";
	}
	else if(_token.type==token::types::val_double) {

		_stream<<" (\""<<_token.double_val<<"\")";
	}
	else if(_token.type==token::types::identifier) {

		_stream<<" \""<<_token.str_val<<"\"";
	}

	
	return _stream;
}

