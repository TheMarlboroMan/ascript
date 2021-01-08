#include "ascript/parser.h"
#include "ascript/error.h"

#include <stdexcept>
#include <algorithm>
//TODO:
#include <iostream>

using namespace ascript;

std::vector<function> parser::parse(
	const std::vector<token>& _tokens
) {

	tokens={_tokens};
	root_mode();
	return std::move(functions);
}

void parser::root_mode() {

	while(tokens.size()) {

		expect(token::types::kw_beginfunction, "only beginfunction is allowed in root nodes");
		auto functionname=expect(token::types::identifier, "beginfunction must be followed by an identifier");

		std::vector<variable> params;

		if(peek().type!=token::types::semicolon) {

			//TODO: actually, should be arguments mode, so we can have different
			//stuff, types and so on.
			params=arguments_mode();
		}

		expect(token::types::semicolon, "function declaration must end with a semicolon");

		//This mode takes care of the final semicolon after endfunction.
		function_mode(functionname, params, 0);
	};
}

void parser::function_mode(
	const token& _function_tok,
	const std::vector<variable>& _parameters,
	int _block_index
) {

	current_function.name=_function_tok.str_val;

	current_function.parameter_names.clear();
	std::transform(
		std::begin(_parameters),
		std::end(_parameters),
		std::back_inserter(current_function.parameter_names),
		[&_function_tok](const variable& _var) {

			if(_var.type!=variable::types::symbol) {

				error_builder::get()<<"parameters for functions must be expressed as identifiers"<<throw_err{_function_tok.line_number, throw_err::types::parser};
			}

			return _var.str_val;
		}
	);

	//Clear the current function...
	current_function.blocks.clear();
	add_block(block::types::linear, current_function);

	instruction_mode(
		[](const token& _tok) {

			return _tok.type==token::types::kw_endfunction;
		},
		_block_index,
		"unexpected end of file, expected endfunction;"
	);

	//Cleanup last semicolon, of course.
	expect(token::types::semicolon, "endfunction must be followed by a semicolon");
	functions.emplace_back(std::move(current_function));
}

void parser::instruction_mode(
	std::function<bool(const token&)> _fn_end,
	int _block_index,
	const std::string& _eof_err_msg
) {

	while(tokens.size()) {

		auto token=extract();

		if(_fn_end(token)) {

			return;
		}

		switch(token.type) {
			case token::types::kw_return:
				expect(token::types::semicolon, "return must be followed by a semicolon");
				current_function.blocks[_block_index].instructions.emplace_back(
					new instruction_return(token.line_number)
				);
			break;
			case token::types::kw_yield:
				expect(token::types::semicolon, "yield must be followed by a semicolon");
				current_function.blocks[_block_index].instructions.emplace_back(
					new instruction_yield(token.line_number)
				);
			break;
			case token::types::kw_break:
				expect(token::types::semicolon, "break must be followed by a semicolon");
				current_function.blocks[_block_index].instructions.emplace_back(
					new instruction_break(token.line_number)
				);
			break;
			case token::types::kw_let:
				variable_declaration_mode(_block_index);
			break;
			case token::types::kw_set:
				variable_assignment_mode(_block_index);
			break;
			case token::types::pr_out:
			case token::types::pr_fail:
			case token::types::pr_host_set:
			case token::types::pr_host_add:
			case token::types::pr_host_do: {
				auto args=arguments_mode();
				add_procedure(token, args, _block_index);
				expect(token::types::semicolon, "procedure call arguments must be followed by a semicolon");
			break;
			}
			case token::types::kw_if:
				conditional_branch_mode(token.line_number, _block_index);
			break;
			case token::types::kw_loop:
				loop_mode(token.line_number, _block_index);
			break;
			default:

				error_builder::get()<<"unexpected '"
					<<type_to_str(token.type)
					<<"' when parsing instructions"
					<<throw_err{token.line_number, throw_err::types::parser};
		}
	}

	error_builder::get()
		<<_eof_err_msg
		<<throw_err{0, throw_err::types::parser};
}

void parser::loop_mode(
	int _line_number,
	int _block_index
) {
	expect(token::types::semicolon, "loop must be followed by a semicolon");

	add_block(block::types::loop, current_function);
	int next_block_index=current_function.blocks.size()-1;

	instruction_mode(
		[](const token& _tok) -> bool {
			return _tok.type==token::types::kw_endloop;
		},
		next_block_index,
		"unexpected end of file, expected endloop"
	);

	expect(token::types::semicolon, "endloop must be followed by a semicolon");

	current_function.blocks[_block_index].instructions.emplace_back(
		new instruction_loop(_line_number, next_block_index)
	);
}

void parser::conditional_branch_mode(
	int _line_number,
	int _block_index
) {
	//TODO: this needs a refactor... 

	//if [not] fn [arg, arg, arg];

	auto add=[this](instruction_conditional_branch& _ifbr) -> int {

		bool negated=false;
		if(peek().type==token::types::kw_not) {

			negated=true;
			extract();
		}

		auto function=extract();
		auto fnptr=build_function(function);
		fnptr->arguments=arguments_mode();
		expect(token::types::semicolon, "function for conditional branch declaration must end with semicolon");

		//Add a new block for the branch...
		add_block(block::types::linear, current_function);
		int next_block_index=current_function.blocks.size()-1;
		_ifbr.branches.push_back({std::move(fnptr), next_block_index, function.line_number, negated});
		return next_block_index;
	};

	//Start the if instruction...
	instruction_conditional_branch * ifbr=new instruction_conditional_branch(_line_number);

	//Add a first block...
	int next_block_index=add(*ifbr);

	//Now we run the linear mode with this next block... We store the last 
	//breaking token so we can know if we need to look for more branches.

	token::types last_type=token::types::kw_if;
	int last_line=0;

	while(true) {

		instruction_mode(
			[&last_type, &last_line, this](const token& _tok) -> bool {
				
				if(_tok.type==token::types::kw_elseif 
					|| _tok.type==token::types::kw_else
					|| _tok.type==token::types::kw_endif
				) {
					last_type=_tok.type;
					last_line=_tok.line_number;
					return true;
				}
			
				return false;
			},
			next_block_index,
			"unexpected end of file, expected else, elseif or endif"
		);

		if(last_type==token::types::kw_elseif) {

			next_block_index=add(*ifbr);
		}
		else if(last_type==token::types::kw_else) {

			add_block(block::types::linear, current_function);
			next_block_index=current_function.blocks.size()-1;
			ifbr->branches.push_back({nullptr, next_block_index, last_line, false});
			expect(token::types::semicolon, "else must end with a semicolon");
		}
		else if(last_type==token::types::kw_endif) {

			expect(token::types::semicolon, "endif must end with a semicolon");
			break;
		}
	}

	//Once all branches are evaluated, we put the if instruction in the original
	//block.
	current_function.blocks[_block_index].instructions.emplace_back(
		ifbr
	);
}

//!Retrieves the parameters from the node previous to [ to ].
std::vector<variable> parser::arguments_mode(
) {

	auto open_bracket=expect(token::types::open_bracket, "argument lists must begin with a left bracket");
	std::vector<variable> parameters;

	while(true) {

		if(!tokens.size()) {

			error_builder::get()<<"unexpected end of file, expected function parameters"<<throw_err{open_bracket.line_number, throw_err::types::parser};
		}

		auto token=extract();
		switch(token.type) {
			case token::types::close_bracket:

				return parameters;
			case token::types::val_string:
				parameters.push_back({token.str_val});
				if(peek().type==token::types::comma) {
					extract();
				}
			break;
			case token::types::val_bool:
				parameters.push_back({token.bool_val});
				if(peek().type==token::types::comma) {
					extract();
				}
			break;
			case token::types::val_int:
				parameters.push_back({token.int_val});
				if(peek().type==token::types::comma) {
					extract();
				}
			break;
			case token::types::identifier:
				parameters.push_back({token.str_val, variable::types::symbol});
				if(peek().type==token::types::comma) {
					extract();
				}
			break;
			default:
				error_builder::get()<<"unexpected '"<<type_to_str(token.type)<<"' in argument list "<<throw_err{token.line_number, throw_err::types::parser};
		}
	}
}

void parser::variable_declaration_mode(
	int _block_index
) {
	//"let" has been already consumed so... identifier + be + value + semicolon...
	auto identifier=expect(token::types::identifier, "let must be followed by an identifier");
	expect(token::types::kw_be, "identifier in let declaration must be followed by be");

	auto value=extract();
	switch(value.type) {
		case token::types::val_string:
		case token::types::val_bool:
		case token::types::val_int:
		case token::types::val_double:

			current_function.blocks[_block_index].instructions.emplace_back(
				new instruction_declaration_static(
					value.line_number, 
					identifier.str_val, 
					build_variable(value)
				)
			);
			expect(token::types::semicolon, "variable declaration must be finished with a semicolon");
			return;
		case token::types::fn_is_equal:
		case token::types::fn_is_lesser_than:
		case token::types::fn_is_greater_than: 
		case token::types::fn_is_int:
		case token::types::fn_is_bool:
		case token::types::fn_is_double:
		case token::types::fn_is_string:
		case token::types::fn_host_has:
		case token::types::fn_host_get:
		case token::types::fn_host_query:
		case token::types::fn_add:
		case token::types::fn_substract:
		{

			auto fnptr=build_function(value);
			fnptr->arguments=arguments_mode();

			current_function.blocks[_block_index].instructions.emplace_back(
				new instruction_declaration_dynamic(
					value.line_number,
					identifier.str_val, 
					fnptr
				)
			);

			expect(token::types::semicolon, "variable declaration must be finished with a semicolon");
			return;
		}
		default:

			error_builder::get()<<"unexpected '"<<type_to_str(value.type)<<"' in variable declaration "<<throw_err{value.line_number, throw_err::types::parser};
	}
}

void parser::variable_assignment_mode(
	int _block_index
) {
	//"set" has been already consumed so... identifier + to + value + semicolon...
	auto identifier=expect(token::types::identifier, "set must be followed by an identifier");
	expect(token::types::kw_to, "identifier in set declaration must be followed by to");

	auto value=extract();
	switch(value.type) {
		case token::types::val_string:
		case token::types::val_bool:
		case token::types::val_int:
		case token::types::val_double:

			current_function.blocks[_block_index].instructions.emplace_back(
				new instruction_assignment_static(
					value.line_number, 
					identifier.str_val, 
					build_variable(value)
				)
			);
			expect(token::types::semicolon, "variable assignment must be finished with a semicolon");
			return;
		case token::types::fn_is_equal:
		case token::types::fn_is_lesser_than:
		case token::types::fn_is_greater_than: 
		case token::types::fn_is_int:
		case token::types::fn_is_bool:
		case token::types::fn_is_double:
		case token::types::fn_is_string:
		case token::types::fn_host_has:
		case token::types::fn_host_get:
		case token::types::fn_host_query:
		case token::types::fn_add:
		case token::types::fn_substract:
		{

			auto fnptr=build_function(value);
			fnptr->arguments=arguments_mode();

			current_function.blocks[_block_index].instructions.emplace_back(
				new instruction_assignment_dynamic(
					value.line_number,
					identifier.str_val, 
					fnptr
				)
			);

			expect(token::types::semicolon, "variable declaration must be finished with a semicolon");
			return;
		}
		default:

			error_builder::get()<<"unexpected '"<<type_to_str(value.type)<<"' in variable declaration "<<throw_err{value.line_number, throw_err::types::parser};
	}
}

std::unique_ptr<instruction_function> parser::build_function(
	const token& _token_fn
) {
	std::unique_ptr<instruction_function> fnptr;

	switch(_token_fn.type) {
		case token::types::fn_is_equal: 
			fnptr.reset(new instruction_is_equal(_token_fn.line_number)); 
			return fnptr;
		case token::types::fn_is_lesser_than: 
			fnptr.reset(new instruction_is_lesser_than(_token_fn.line_number)); 
			return fnptr;
		case token::types::fn_is_greater_than:
			fnptr.reset(new instruction_is_greater_than(_token_fn.line_number));
			return fnptr;
		case token::types::fn_is_int:
			fnptr.reset(new instruction_is_int(_token_fn.line_number));
			return fnptr;
		case token::types::fn_is_bool:
			fnptr.reset(new instruction_is_bool(_token_fn.line_number));
			return fnptr;
		case token::types::fn_is_double:
			fnptr.reset(new instruction_is_double(_token_fn.line_number));
			return fnptr;
		case token::types::fn_is_string:
			fnptr.reset(new instruction_is_string(_token_fn.line_number));
			return fnptr;
		case token::types::fn_host_has:
			fnptr.reset(new instruction_host_has(_token_fn.line_number)); 
			return fnptr;
		case token::types::fn_host_get:
			fnptr.reset(new instruction_host_get(_token_fn.line_number)); 
			return fnptr;
		case token::types::fn_host_query:
			fnptr.reset(new instruction_host_query(_token_fn.line_number)); 
			return fnptr;
		case token::types::fn_add:
			fnptr.reset(new instruction_add(_token_fn.line_number)); 
			return fnptr;
		case token::types::fn_substract:
			fnptr.reset(new instruction_substract(_token_fn.line_number)); 
			return fnptr;

		default:

			error_builder::get()<<"unknown function type '"<<type_to_str(_token_fn.type)<<"' this must mean the function is not added to the list of buildable functions "<<throw_err{_token_fn.line_number, throw_err::types::parser};
	}

	return fnptr;
}

void parser::add_procedure(
	const token& _token, 
	std::vector<variable>& _arguments,
	int _block_index
) {
	instruction_procedure * prptr{nullptr};

	switch(_token.type) {
		case token::types::pr_out:
			prptr=new instruction_out(_token.line_number);
		break;
		case token::types::pr_fail:
			prptr=new instruction_fail(_token.line_number);
		break;
		case token::types::pr_host_set:
			check_argcount(2, _arguments, _token);
			prptr=new instruction_host_set(_token.line_number);
		break;
		case token::types::pr_host_add:
			check_argcount(2, _arguments, _token);
			prptr=new instruction_host_add(_token.line_number);
		break;
		case token::types::pr_host_do:
			prptr=new instruction_host_do(_token.line_number);
		break;
		default:

			error_builder::get()<<"unknown procedure type '"<<type_to_str(_token.type)<<"' "<<throw_err{_token.line_number, throw_err::types::parser};
	}

	prptr->arguments=_arguments;

	current_function.blocks[_block_index].instructions.emplace_back(
		prptr
	);
}

void parser::check_argcount(
	std::size_t _expected, 
	const std::vector<variable>& _arguments, 
	const token& _token
) {

	if(_expected==_arguments.size()) {

		return;
	}

	error_builder::get()<<"invalid argument count for '"
		<<type_to_str(_token.type)
		<<"' expected "
		<<_expected
		<<", got "
		<<_arguments.size()
		<<throw_err{_token.line_number, throw_err::types::parser};
}

token parser::expect(
	token::types _type,
	const std::string& _err_msg
) {

	if(!tokens.size()) {
		throw parser_error(
			std::string{"expect called with no tokens left ("}
			+_err_msg
			+")"
		);
	}

	auto token=tokens.front();
	tokens.erase(std::begin(tokens));
	if(token.type!=_type) {

		error_builder::get()<<"expected '"
			<<type_to_str(_type)
			<<"' got '"
			<<type_to_str(token.type)
			<<"' "
			<<throw_err{token.line_number, throw_err::types::parser};
	}

	return token;
}

token parser::extract() {

	if(!tokens.size()) {

		throw parser_error("extract called with no tokens left");
	}

	auto token=tokens.front();
	tokens.erase(std::begin(tokens));
	return token;
}

token parser::peek() {

	if(!tokens.size()) {

		throw parser_error("peek called with no tokens left");
	}

	auto token=tokens.front();
	return token;
}


void parser::add_block(
	block::types _type, 
	function& _function
) {

	_function.blocks.push_back(
		{
			_type,
			std::vector<std::unique_ptr<instruction>>{}
		}
	);
}

variable parser::build_variable(
	const token& _token
) {

std::cout<<_token<<std::endl;

	switch(_token.type) {
		case token::types::val_string: return _token.str_val;
		case token::types::val_bool: return _token.bool_val;
		case token::types::val_int: return _token.int_val;
		case token::types::val_double: return _token.double_val;
		default:
			error_builder::get()<<"invalid type for build_variable"<<throw_err{_token.line_number, throw_err::types::parser};
	}

	return false;
}
