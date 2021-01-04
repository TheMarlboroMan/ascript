#include "ascript/parser.h"

#include <stdexcept>
#include <algorithm>
//TODO:
#include <iostream>

using namespace ascript;

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

	//Clear the current script...
	current_script.contexts.clear();
	add_context(context::types::linear, current_script);

	instruction_mode(
		[](const token& _tok) {

			return _tok.type==token::types::kw_endscript;
		},
		_context_index,
		"unexpected end of file, expected endscript;"
	);

	//Cleanup last semicolon, of course.
	expect(token::types::semicolon, "endscript must be followed by a semicolon");
	scripts.insert(std::make_pair(_scriptname, std::move(current_script)));
}

void parser::instruction_mode(
	std::function<bool(const token&)> _fn_end,
	int _context_index,
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
			case token::types::pr_fail:
			case token::types::pr_host_set:
			case token::types::pr_host_add:
			case token::types::pr_host_do: {
				auto args=arguments_mode();
				add_procedure(token.type, args, _context_index);
				expect(token::types::semicolon, "procedure call arguments must be followed by a semicolon");
			break;
			}
			case token::types::kw_if:
				conditional_branch_mode(_context_index);
			break;
			case token::types::kw_loop:
				loop_mode(_context_index);
			break;
			default:
				throw std::runtime_error(
					std::string{"unexpected '"}
					+type_to_str(token.type)
					+"' when parsing instructions on line "
					+std::to_string(token.line_number)
				);
		}
	}

	throw std::runtime_error(_eof_err_msg);
}

void parser::loop_mode(
	int _context_index
) {
	expect(token::types::semicolon, "loop must be followed by a semicolon");

	add_context(context::types::loop, current_script);
	int next_context_index=current_script.contexts.size()-1;

	instruction_mode(
		[](const token& _tok) -> bool {
			return _tok.type==token::types::kw_endloop;
		},
		next_context_index,
		"unexpected end of file, expected endloop"
	);

	expect(token::types::semicolon, "endloop must be followed by a semicolon");

	current_script.contexts[_context_index].instructions.emplace_back(
		new instruction_loop(next_context_index)
	);
}

void parser::conditional_branch_mode(
	int _context_index
) {
	//TODO: this needs a refactor... 

	//if [not] fn [arg, arg, arg];

	bool negated=false;
	if(peek().type==token::types::kw_not) {

		negated=true;
		extract();
	}

	auto function=extract();
	auto fnptr=build_function(function.type);
	fnptr->arguments=arguments_mode();
	expect(token::types::semicolon, "function for conditional branch declaration start must end with semicolon");

	instruction_conditional_branch * ifbr=new instruction_conditional_branch();

	//Add a new context for the first branch...
	add_context(context::types::linear, current_script);
	int next_context_index=current_script.contexts.size()-1;
	ifbr->branches.push_back({std::move(fnptr), next_context_index, negated});

	//Now we run the linear mode with this next context... We store the last 
	//breaking token so we can know if we need to look for more branches.
	token::types last_type=token::types::kw_if;

	while(true) {

		instruction_mode(
			[&last_type, this](const token& _tok) -> bool {
				
				if(_tok.type==token::types::kw_elseif 
					|| _tok.type==token::types::kw_else
					|| _tok.type==token::types::kw_endif
				) {
					last_type=_tok.type;
					return true;
				}
			
				return false;
			},
			next_context_index,
			"unexpected end of file, expected else, elseif or endif"
		);

		if(last_type==token::types::kw_elseif) {

			bool negated=false;
			if(peek().type==token::types::kw_not) {

				negated=true;
				extract();
			}

			auto function=extract();
			auto fnptr=build_function(function.type);
			fnptr->arguments=arguments_mode();
			expect(token::types::semicolon, "function for subsequent conditional branch declarations must end with semicolon");

			add_context(context::types::linear, current_script);
			next_context_index=current_script.contexts.size()-1;
			ifbr->branches.push_back({std::move(fnptr), next_context_index, negated});
		}
		else if(last_type==token::types::kw_else) {

			add_context(context::types::linear, current_script);
			next_context_index=current_script.contexts.size()-1;
			ifbr->branches.push_back({nullptr, next_context_index, false});
			expect(token::types::semicolon, "else must end with a semicolon");
		}
		else if(last_type==token::types::kw_endif) {

			expect(token::types::semicolon, "endif must end with a semicolon");
			break;
		}
	}

	//Once all branches are evaluated, we put the if instruction in the original
	//context.
	current_script.contexts[_context_index].instructions.emplace_back(
		ifbr
	);
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
		case token::types::fn_is_greater_than: 
		case token::types::fn_host_has:
		case token::types::fn_host_get:
		case token::types::fn_host_query:
		{

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
		case token::types::fn_host_has:
			fnptr.reset(new instruction_host_has()); 
			return fnptr;
		case token::types::fn_host_get:
			fnptr.reset(new instruction_host_get()); 
			return fnptr;
		case token::types::fn_host_query:
			fnptr.reset(new instruction_host_query()); 
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
	std::vector<variable>& _arguments,
	int _context_index
) {
	instruction_procedure * prptr{nullptr};

	switch(_type) {
		case token::types::pr_out:
			prptr=new instruction_out();
		break;
		case token::types::pr_fail:
			prptr=new instruction_fail();
		break;
		case token::types::pr_host_set:
			prptr=new instruction_host_set();
		break;
		case token::types::pr_host_add:
			prptr=new instruction_host_add();
		break;
		case token::types::pr_host_do:
			prptr=new instruction_host_do();
		break;
		default:
			throw std::runtime_error(
				std::string{"unknown procedure type '"}
				+type_to_str(_type)
				+"'"
			);
	}

	prptr->arguments=_arguments;

	current_script.contexts[_context_index].instructions.emplace_back(
		prptr
	);
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


void parser::add_context(
	context::types _type, 
	script& _script
) {

	_script.contexts.push_back(
		{
			_type,
			std::vector<std::unique_ptr<instruction>>{}
		}
	);
}
