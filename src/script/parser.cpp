#include "parser.h"

#include <stdexcept>
#include <algorithm>
//TODO:
#include <iostream>

using namespace script;

void parser::parse(
	const std::vector<token>& _tokens
) {

	tokens={_tokens};
	root_mode();
}

void parser::root_mode() {

	while(tokens.size()) {

		expect(token::types::kw_beginscript, "only beginscript is allowed in root nodes");
		auto scriptname=expect(token::types::identifier, "beginscript must be followed by an identifier");

		std::vector<variable> params;

		if(peek().type!=token::types::semicolon) {

			//TODO: actually, should be arguments mode, so we can have different
			//stuff, types and so on.
			params=arguments_mode();
		}

		expect(token::types::semicolon, "script declaration must end with a semicolon");

		//This mode takes care of the final semicolon after endscript.
		script_mode(scriptname.str_val, params, 0);
	};

	for(const auto& script : scripts) {

		std::cout<<script.second<<std::endl;
	}
}

void parser::script_mode(
	const std::string& _scriptname,
	const std::vector<variable>& _parameters,
	int _context_index
) {

	current_script.name=_scriptname;

	current_script.parameter_names.clear();
	std::transform(
		std::begin(_parameters),
		std::end(_parameters),
		std::back_inserter(current_script.parameter_names),
		[](const variable& _var) {

			//TODO: make sure these are declared as string parameters!!!
			return _var.str_val;
		}
	);

	current_script.contexts.clear();
	current_script.contexts.push_back(
		{
			context::types::linear, 
			std::vector<std::unique_ptr<instruction>>{}
		}
	);

	while(tokens.size()) {

		auto token=extract();
		
		switch(token.type) {
			case token::types::kw_endscript:

				expect(token::types::semicolon, "endscript must be followed by a semicolon");
				scripts.insert(std::make_pair(_scriptname, std::move(current_script)));
				return;
			case token::types::kw_return:
				expect(token::types::semicolon, "return must be followed by a semicolon");
				current_script.contexts[_context_index].instructions.emplace_back(
					new instruction_return()
				);
			break;
			case token::types::kw_yield:
				expect(token::types::semicolon, "yield must be followed by a semicolon");
				current_script.contexts[_context_index].instructions.emplace_back(
					new instruction_yield()
				);
			break;
			case token::types::kw_break:
				expect(token::types::semicolon, "break must be followed by a semicolon");
				current_script.contexts[_context_index].instructions.emplace_back(
					new instruction_break()
				);
			break;
			case token::types::kw_let:
				variable_declaration_mode(_context_index);
			break;
			case token::types::pr_out:
			case token::types::pr_fail:{
				auto args=arguments_mode();
				add_procedure(token.type, args, _context_index);
				expect(token::types::semicolon, "procedure call arguments must be followed by a semicolon");
			break;
			}
			case token::types::kw_if:
				conditional_branch_mode(_context_index);
			break;
			default:
				throw std::runtime_error(
					std::string{"unexpected '"}
					+type_to_str(token.type)
					+"' on line "
					+std::to_string(token.line_number)
				);
		}
	}

	throw std::runtime_error("unexpected end of file, expected endscript;");
}

void parser::conditional_branch_mode(
	int _context_index
) {
	//if fn [arg, arg, arg];
	auto function=extract();
	auto fnptr=build_function(function.type);
	fnptr->arguments=arguments_mode();
	expect(token::types::semicolon, "conditional branch declaration must end with semicolon");

	instruction_conditional_branch * ifbr=new instruction_conditional_branch();
	//TODO!!
	int next_context_index=999;
	ifbr->branches.push_back({std::move(fnptr), next_context_index});

	//TODO:
	//This goes at the end...
	current_script.contexts[_context_index].instructions.emplace_back(
		ifbr
	);

	/*
struct conditional_path {

	std::unique_ptr<instruction_function>   function;
	int                                     target_context_index;
}

//!instruction to execute conditional logic.
struct instruction_conditional_branch {

	std::vector<conditional_path>           branches;
}
*/
	
	//TODO: first, we can add a new context, right? a linear context and 

	//TODO: with this shit we can build an if 
	//instruction and keep reading until "elseif, else or endif".
	//TODO: let's see how we handle this...

/*
if (function call)... or maybe if (boolean var)
*/
}

//!Retrieves the parameters from the node previous to [ to ].
std::vector<variable> parser::arguments_mode(
) {

	expect(token::types::open_bracket, "argument lists must begin with a left bracket");
	std::vector<variable> parameters;

	while(true) {

		if(!tokens.size()) {

			//TODO Throw custom error.
			throw std::runtime_error("unexpected end of file, expected function parameters");
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
				//TODO Throw custom error.
				throw std::runtime_error(
					std::string{"unexpected '"}
						+type_to_str(token.type)
						+"' in argument list on line "
						+std::to_string(token.line_number)
				);
		}
	}
}

void parser::variable_declaration_mode(
	int _context_index
) {
	//"let" has been already consumed so... identifier + be + value + semicolon...
	auto identifier=expect(token::types::identifier, "let must be followed by an identifier");
	expect(token::types::kw_be, "identifier in let declaration must be followed by be");

	auto value=extract();
	switch(value.type) {
		case token::types::val_string:
			current_script.contexts[_context_index].instructions.emplace_back(
				new instruction_declaration_static(identifier.str_val, {value.str_val})
			);
			expect(token::types::semicolon, "variable declaration must be finished with a semicolon");
			return;
		case token::types::val_bool:
			current_script.contexts[_context_index].instructions.emplace_back(
				new instruction_declaration_static(identifier.str_val, {value.bool_val})
			);
			expect(token::types::semicolon, "variable declaration must be finished with a semicolon");
			return;
		break;
		case token::types::val_int:
			current_script.contexts[_context_index].instructions.emplace_back(
				new instruction_declaration_static(identifier.str_val, {value.int_val})
			);
			expect(token::types::semicolon, "variable declaration must be finished with a semicolon");
			return;
		case token::types::fn_is_equal:
		case token::types::fn_is_lesser_than:
		case token::types::fn_is_greater_than: {

			auto fnptr=build_function(value.type);
			fnptr->arguments=arguments_mode();

			current_script.contexts[_context_index].instructions.emplace_back(
				new instruction_declaration_dynamic(
					identifier.str_val, 
					fnptr
				)
			);

			expect(token::types::semicolon, "variable declaration must be finished with a semicolon");
			return;
		}
		default:
			//TODO Throw custom error.
			throw std::runtime_error(
				std::string{"unexpected '"}
					+type_to_str(value.type)
					+"' in variable declaration on line "
					+std::to_string(value.line_number)
			);
	}
}

std::unique_ptr<instruction_function> parser::build_function(
	token::types _type
) {
	std::unique_ptr<instruction_function> fnptr;

	switch(_type) {
		case token::types::fn_is_equal: 
			fnptr.reset(new instruction_is_equal()); 
			return fnptr;
		case token::types::fn_is_lesser_than: 
			fnptr.reset(new instruction_is_lesser_than()); 
			return fnptr;
		case token::types::fn_is_greater_than:
			fnptr.reset(new instruction_is_greater_than());
			return fnptr;

		default: 
			throw std::runtime_error(
				std::string{"unkwnown function type '"}
				+type_to_str(_type)
				+"', this must mean the function is not added to the list of buildable functions"
			);
	}

	return fnptr;
}

void parser::add_procedure(
	token::types _type, 
	std::vector<variable>& _parameters,
	int _context_index
) {
	switch(_type) {
		case token::types::pr_out:
			current_script.contexts[_context_index].instructions.emplace_back(
				new instruction_out(_parameters)
			);
			return;
		case token::types::pr_fail:
			current_script.contexts[_context_index].instructions.emplace_back(
				new instruction_fail(_parameters)
			);
			return;
			return;
		default:
			throw std::runtime_error(
				std::string{"unknown function type '"}
				+type_to_str(_type)
				+"'"
			);
	}
}

token parser::expect(
	token::types _type,
	const std::string& _err_msg
) {

	if(!tokens.size()) {
		//TODO: other type
		throw std::runtime_error(
			std::string{"expect called with no tokens left ("}
			+_err_msg
			+")"
		);
	}

	auto token=tokens.front();
	tokens.erase(std::begin(tokens));
	if(token.type!=_type) {

		//TODO: throw custom exception.
		throw std::runtime_error(
			std::string{"expected '"}
			+type_to_str(_type)
			+"' got '"
			+type_to_str(token.type)
			+"', "
			+_err_msg+", on line "
			+std::to_string(token.line_number)
		);
	}

	return token;
}

token parser::extract() {

	if(!tokens.size()) {

		//TODO Throw custom exception
		throw std::runtime_error("extract called with no tokens left");
	}

	auto token=tokens.front();
	tokens.erase(std::begin(tokens));
	return token;
}

token parser::peek() {

	if(!tokens.size()) {

		//TODO Throw custom exception
		throw std::runtime_error("peek called with no tokens left");
	}

	auto token=tokens.front();
	return token;
}

