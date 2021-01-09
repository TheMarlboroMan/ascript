#pragma once

#include "instructions.h"
#include "token.h"
#include "error.h"

#include <vector>
#include <map>
#include <functional>

namespace ascript {

class parser {

	public:

	std::vector<function>     parse(const std::vector<token>&);

	private:

	//!Root mode, little more than declaring functions
	void                    root_mode();

	//!Starts a function.
	void                    function_mode(const token&, const std::vector<parameter>&, int);

	//!Regular operation parsing mode.
	void                    instruction_mode(std::function<bool(const token&)>, int, const std::string&);

	//!Reading bracket delimiter arguments.
	std::vector<variable>   arguments_mode();

	//!Reading bracket delimited function parameters.
	std::vector<parameter>   parameters_mode();

	//!Reading a variable declaration.
	void                    variable_declaration_mode(int);

	//!Reading variable assignment.
	void                    variable_assignment_mode(int);

	//!Builds a function instruction.
	std::unique_ptr<instruction_function> build_function(const token&);

	//!Reading if branches...
	void                    conditional_branch_mode(int, int);

	//!Reading loops...
	void                    loop_mode(int, int);

	//!Reading a procedure (out, fail...)
	void                    add_procedure(const token&, std::vector<variable>&, int);

	//!Will throw if the count of arguments does not match the expected value.
	void                    check_argcount(std::size_t, const std::vector<variable>&, const token&);

	//!Adds a new block.to the given function.
	void                    add_block(block::types, function&);

	//!Creates a variable from a value token.
	variable                build_variable(const token&);

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
