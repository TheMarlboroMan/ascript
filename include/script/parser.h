#pragma once

#include <vector>
#include "parser_nodes.h"
#include "token.h"

namespace script {

class parser {

	public:

	void                    parse(const std::vector<token>&);

	private:

	void                    root_mode();
	void                    script_mode(const std::string&);
	void                    function_mode(token::types, int);
	void                    add_function(token::types, std::vector<variable>&, int);
	token                   expect(token::types);
	token                   extract();
	token                   peek();

	std::vector<token>      tokens;
	script                  current_script;
};

}
