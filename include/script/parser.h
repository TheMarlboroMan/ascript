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

	void                    root_mode();
	void                    script_mode(const std::string&);
	std::vector<variable>   parameters_mode();
	void                    variable_declaration_mode(int);
	void                    add_procedure(token::types, std::vector<variable>&, int);
	token                   expect(token::types);
	token                   extract();
	token                   peek();

	std::vector<token>      tokens;
	std::map<std::string, script> scripts;
	script                  current_script;
};

}
