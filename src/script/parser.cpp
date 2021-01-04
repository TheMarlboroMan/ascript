#include "parser.h"

#include <stdexcept>

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

		expect(token::types::kw_beginscript);
		auto scriptname=expect(token::types::identifier);
		expect(token::types::semicolon);

		script_mode(scriptname.str_val);
	};
}

void parser::script_mode(
	const std::string& _scriptname
) {

	current_script.name=_scriptname;
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

				expect(token::types::semicolon);

				//TODO: Close script, MOVE it to the container be done;
				std::cout<<current_script<<std::endl;
				return;
			case token::types::kw_return:{

				expect(token::types::semicolon);

				current_script.contexts[0].instructions.emplace_back(
					new instruction_return()
				);
			}
			break;
			case token::types::fn_out:

				function_mode(token.type, 0);
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

void parser::function_mode(
	token::types _type,
	int _context_index
) {

	expect(token::types::open_bracket);
	std::vector<variable> parameters;

	while(true) {

		if(!tokens.size()) {

			//TODO Throw custom error.
			throw std::runtime_error("unexpected end of file, expected function parameters");
		}

		auto token=extract();
		switch(token.type) {
			case token::types::close_bracket:
				expect(token::types::semicolon);
					add_function(_type, parameters, _context_index);
				return;
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

void parser::add_function(
	token::types _type, 
	std::vector<variable>& _parameters,
	int _context_index
) {


	switch(_type) {
		case token::types::fn_out:
			current_script.contexts[_context_index].instructions.emplace_back(
				new instruction_out(_parameters)
			);
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
	token::types _type
) {

	auto token=tokens.front();
	tokens.erase(std::begin(tokens));
	if(token.type!=_type) {

		//TODO: throw custom exception.
		throw std::runtime_error(
			std::string{"expected '"}
			+type_to_str(_type)
			+"' got '"
			+type_to_str(token.type)
			+"' on line "
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

/*

parser_root_mode:

	expect beginscript
	expect identifier
	expect semicolon
	createscript identifier
	enter script mode
		
parser_script_mode:

	ends when endscript is found expect semicolon
	try instruction
		add instruction to script
	
*/
