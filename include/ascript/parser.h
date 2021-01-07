#pragma once

#include "parser_nodes.h"
#include "token.h"

#include <vector>
#include <map>
#include <functional>

namespace ascript {

struct parser_error:std::runtime_error {

	                        parser_error(const std::string& _msg):runtime_error(_msg){}
};

class parser {

	public:

	std::vector<function>     parse(const std::vector<token>&);

	private:

	//!Root mode, little more than declaring functions
	void                    root_mode();

	//!Starts a function.
	void                    function_mode(const std::string&, const std::vector<variable>&, int);

	//!Regular operation parsing mode.
	void                    instruction_mode(std::function<bool(const token&)>, int, const std::string&);

	//!Reading bracket delimiter arguments.
	std::vector<variable>   arguments_mode();

	//!Reading a variable declaration.
	void                    variable_declaration_mode(int);

	//!Builds a function instruction.
	std::unique_ptr<instruction_function> build_function(token::types);

	//!Reading if branches...
	void                    conditional_branch_mode(int);

	//!Reading loops...
	void                    loop_mode(int);

	//!Reading a procedure (out, fail...)
	void                    add_procedure(token::types, std::vector<variable>&, int);

	//!Adds a new block.to the given function.
	void                    add_block(block::types, function&);

	//!Throws if the next token is not of the given type. Returns next token.
	token                   expect(token::types, const std::string&);

	//!Returns next token.
	token                   extract();

	//!Returns next token without removing it from the token list.
	token                   peek();

	std::vector<token>      tokens;
	std::vector<function>   functions;
	function                current_function;
};

}
