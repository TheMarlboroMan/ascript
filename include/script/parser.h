#pragma once

#include "parser_nodes.h"
#include "token.h"

#include <vector>
#include <map>

namespace script {

class parser {

	public:

	void                    parse(const std::vector<token>&);

	private:

	//!Root mode, little more than declaring scripts
	void                    root_mode();

	//!Linear mode inside a script.
	void                    script_mode(const std::string&, const std::vector<variable>&, int);

	//!Reading bracket delimiter arguments.
	std::vector<variable>   arguments_mode();

	//!Reading a variable declaration.
	void                    variable_declaration_mode(int);

	//!Builds a function instruction.
	std::unique_ptr<instruction_function> build_function(token::types);

	//!Reading if branches...
	void                    conditional_branch_mode(int);

	//!Reading a procedure (out, fail...)
	void                    add_procedure(token::types, std::vector<variable>&, int);

	//!Throws if the next token is not of the given type. Returns next token.
	token                   expect(token::types, const std::string&);

	//!Returns next token.
	token                   extract();

	//!Returns next token without removing it from the token list.
	token                   peek();

	std::vector<token>      tokens;
	std::map<std::string, script> scripts;
	script                  current_script;
};

}
