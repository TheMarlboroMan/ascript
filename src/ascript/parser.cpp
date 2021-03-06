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

		std::vector<parameter> params;

		if(peek().type!=token::types::semicolon) {

			params=parameters_mode();
		}

		expect(token::types::semicolon, "function declaration must end with a semicolon");

		//This mode takes care of the final semicolon after endfunction.
		function_mode(functionname, params, 0);
	};
}

void parser::function_mode(
	const token& _function_tok,
	const std::vector<parameter>& _parameters,
	int _block_index
) {

	current_function.name=_function_tok.str_val;
	current_function.parameters=_parameters;

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

		if(token.type==token::types::kw_return) {

			return_mode(_block_index, token);
		}
		else if(token.type==token::types::kw_yield) {

			yield_mode(_block_index, token);
		}
		else if(token.type==token::types::kw_break) {

			expect(token::types::semicolon, "break must be followed by a semicolon");
			current_function.blocks[_block_index].instructions.emplace_back(
				new instruction_break(token.line_number)
			);
		}
		else if(token.type==token::types::kw_exit) {

			expect(token::types::semicolon, "exit must be followed by a semicolon");
			current_function.blocks[_block_index].instructions.emplace_back(
				new instruction_exit(token.line_number)
			);
		}
		else if(token.type==token::types::kw_let) {

			variable_manipulation_mode(_block_index, variable_modes::declaration);
		}
		else if(token.type==token::types::kw_set) {

			variable_manipulation_mode(_block_index, variable_modes::assignment);
		}
		else if(token.type==token::types::identifier) {

			function_call_mode(_block_index, token);
		}
		else if(token.type==token::types::kw_if) {

			conditional_branch_mode(token.line_number, _block_index);
		}
		else if(token.type==token::types::kw_loop) {

				loop_mode(token.line_number, _block_index);
		}
		else if(is_built_in_procedure(token)) {

			auto args=arguments_mode();
			add_procedure(token, args, _block_index);
			expect(token::types::semicolon, "procedure call arguments must be followed by a semicolon");
		}
		else {

			//As can be seen, functions are just not acceptable in this mode and
			//can only be used to assign or recall values.
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

void parser::yield_mode(
	int _block_index,
	const token& _token
) {

	if(peek().type==token::types::kw_for) {

		extract();
		auto ms_token=extract();

		variable ms{0};
		if(is_static_value(ms_token)) {
			
			ms=build_variable(ms_token);
		}
		else if(ms_token.type==token::types::identifier) {

			ms={ms_token.str_val, variable::types::symbol};
		}
		else {

			error_builder::get()<<"integer or symbol expected after yield for"<<throw_err{ms_token.line_number, throw_err::types::parser};
		}

		expect(token::types::semicolon, "yield must be followed by a semicolon");
		current_function.blocks[_block_index].instructions.emplace_back(
			new instruction_yield(_token.line_number, ms)
		);

	}
	else {

		expect(token::types::semicolon, "yield must be followed by a semicolon");
		current_function.blocks[_block_index].instructions.emplace_back(
			new instruction_yield(_token.line_number)
		);
	}

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

void parser::return_mode(
	int _block_index,
	const token& _token

) {

	std::optional<variable> returnval;

	//Return might or not might return a value. I wonder if I should
	//use different keywords, like "return [3]" and "done".
	if(peek().type==token::types::open_bracket) {

		auto args=arguments_mode();
		check_argcount(1, args, _token);

		current_function.blocks[_block_index].instructions.emplace_back(
			new instruction_return(_token.line_number, args.front())
		);

	}
	else {
		current_function.blocks[_block_index].instructions.emplace_back(
			new instruction_return(_token.line_number)
		);
	}

	expect(token::types::semicolon, "return must be followed by a semicolon");
}

void parser::conditional_branch_mode(
	int _line_number,
	int _block_index
) {
	//TODO: this needs a refactor... 

	//if [not] fn [arg, arg, arg];

	//This lambda expresses the intention of adding a new block to an if branch
	//statement. It will just add the if instruction with its evaluation 
	//function and create the block where the following instructions will go.
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
	//Last line will store the last line of a branch, signaling where the next
	//starts.

	token::types last_type=token::types::kw_if;
	int last_line=0;

	while(true) {

		//TODO it would be clear if this was a lambda itself to be invoked
		//after each if below.

		//Read instructions into the current branch...
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

		//Is there an elseif? add instruction and go back to reading instructions into this new branch.
		if(last_type==token::types::kw_elseif) {

			next_block_index=add(*ifbr);
		}
		//Is there an else? These are represented as "functionless" branches.   Add and come back again to read instructions into it.
		else if(last_type==token::types::kw_else) {

			add_block(block::types::linear, current_function);
			next_block_index=current_function.blocks.size()-1;
			ifbr->branches.push_back({nullptr, next_block_index, last_line, false});
			expect(token::types::semicolon, "else must end with a semicolon");
		}
		//Signals the end of the if statements.
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

			error_builder::get()<<"unexpected end of file, expected function arguments"<<throw_err{open_bracket.line_number, throw_err::types::parser};
		}

		auto token=extract();

		if(token.type==token::types::close_bracket) {

			return parameters;
		}
		else if(is_static_value(token)) {
			
			parameters.push_back(build_variable(token));
			if(peek().type==token::types::comma) {
				extract();
			}
		}
		else if(token.type==token::types::identifier) {

			parameters.push_back({token.str_val, variable::types::symbol});
			if(peek().type==token::types::comma) {
				extract();
			}
		}
		else {
			error_builder::get()<<"unexpected '"<<type_to_str(token.type)<<"' in argument list "<<throw_err{token.line_number, throw_err::types::parser};
		}
	}
}

//!Retrieves the parameters from the node previous from [ to ].
std::vector<parameter> parser::parameters_mode(
) {

	auto open_bracket=expect(token::types::open_bracket, "parameter lists must begin with a left bracket");

	std::vector<parameter> parameters;

	while(true) {

		auto identifier=expect(token::types::identifier, "expected identifier in function parameter declaration");
		expect(token::types::kw_as, "expected as after parameter name");
		auto type=extract();

		auto ptype=parameter::types::integer;

		switch(type.type) {

			case token::types::kw_integer: ptype=parameter::types::integer; break;
			case token::types::kw_string: ptype=parameter::types::string; break;
			case token::types::kw_bool: ptype=parameter::types::boolean; break;
			case token::types::kw_double: ptype=parameter::types::decimal; break;
			case token::types::kw_anytype: ptype=parameter::types::any; break;
			default:
				error_builder::get()
					<<"unexpected '"
					<<type_to_str(type.type)
					<<"', expected data type in function parameter declaration"
					<<throw_err{type.line_number, throw_err::types::parser};
		}

		parameters.push_back({identifier.str_val, ptype});

		auto next=extract();

		switch(next.type) {
			case token::types::close_bracket:
				return parameters;
			case token::types::comma: break;
			default:
				error_builder::get()
					<<"unexpected '"
					<<type_to_str(next.type)
					<<"', expected comma or close bracket after function parameter declaration"
					<<throw_err{next.line_number, throw_err::types::parser};
		}
	}
}

void parser::variable_manipulation_mode(
	int _block_index,
	variable_modes _mode
) {
	//"let/set" has been already consumed so... identifier + be + value + semicolon...
	auto identifier=expect(token::types::identifier, "let must be followed by an identifier");

	switch(_mode) {
		case variable_modes::declaration:
			expect(token::types::kw_be, "identifier in let declaration must be followed by be");
		break;
		case variable_modes::assignment:
			expect(token::types::kw_to, "identifier in set declaration must be followed by to");
		break;
	}

	std::unique_ptr<instruction_function> fnptr{nullptr};

	auto value=extract();

	if(is_static_value(value)) {

		fnptr=build_function(value);
		fnptr->arguments.push_back(build_variable(value));
		expect(token::types::semicolon, "variable declaration/assignment must be finished with a semicolon");
	}
	else if(is_built_in_function(value)) {

		fnptr=build_function(value);
		fnptr->arguments=arguments_mode();
		if(value.type==token::types::fn_host_get) {

			check_argcount(1, fnptr->arguments, value);
		}
		expect(token::types::semicolon, "variable declaration/assignment must be finished with a semicolon");
	}
	else if(value.type==token::types::identifier) {

		//Returning is simulated with a call instruction, that would return
		//a value stored into a specific register and an instruction to 
		//read from it into the identifier.
		function_call_mode(_block_index, value);
		fnptr=build_function(value);
	}
	else {

		error_builder::get()<<"unexpected '"<<type_to_str(value.type)<<"' in variable declaration "<<throw_err{value.line_number, throw_err::types::parser};
	}

	switch(_mode) {

		case variable_modes::declaration:

			current_function.blocks[_block_index].instructions.emplace_back(
				new instruction_declaration_dynamic(
					value.line_number,
					identifier.str_val, 
					fnptr
				)
			);
		break;
		case variable_modes::assignment:

			current_function.blocks[_block_index].instructions.emplace_back(
				new instruction_assignment_dynamic(
					value.line_number,
					identifier.str_val, 
					fnptr
				)
			);
		break;
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
		case token::types::fn_concatenate:
			fnptr.reset(new instruction_concatenate(_token_fn.line_number)); 
			return fnptr;
		case token::types::fn_substract:
			fnptr.reset(new instruction_substract(_token_fn.line_number)); 
			return fnptr;
		case token::types::identifier:
			fnptr.reset(new instruction_copy_from_return_register(_token_fn.line_number)); 
			return fnptr;
		default:
			//These are not really functions, but can build a function that returns
			//a value of the given type.
			if(is_static_value(_token_fn)) {
				fnptr.reset(new instruction_generate_value(_token_fn.line_number)); 
				return fnptr;
			}

			error_builder::get()<<"unknown function type '"<<type_to_str(_token_fn.type)<<"' this must mean the function is not added to the list of buildable functions "<<throw_err{_token_fn.line_number, throw_err::types::parser};
		break;
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
		case token::types::pr_host_delete:
			check_argcount(1, _arguments, _token);
			prptr=new instruction_host_delete(_token.line_number);
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

void parser::function_call_mode(
	int _block_index,
	const token& _token
) {

	//fnname [ param1, params2... ];
	//or if no params, fnname [];
	auto arguments=arguments_mode();
	expect(token::types::semicolon, "expected semicolon after function call");

	current_function.blocks[_block_index].instructions.emplace_back(
		new instruction_function_call(
			_token.line_number,
			_token.str_val,
			arguments
		)
	);
}

bool parser::is_static_value(
	const token& _token
) const {

	switch(_token.type) {
		case token::types::val_string:
		case token::types::val_bool:
		case token::types::val_int:
		case token::types::val_double:
			return true;
		default:
			return false;
	}

	return false; //Overzealous compilers rule.

}

bool parser::is_built_in_function(
	const token& _token
) const {

	switch(_token.type) {
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
		case token::types::fn_concatenate:
			return true;
		default:
			return false;
	}

	return false;
}

bool parser::is_built_in_procedure(
	const token& _token
) const {

	switch(_token.type) {
		case token::types::pr_out:
		case token::types::pr_fail:
		case token::types::pr_host_set:
		case token::types::pr_host_add:
		case token::types::pr_host_delete:
		case token::types::pr_host_do:
			return true;
		default:
			return false;
	}

	return false;
}
