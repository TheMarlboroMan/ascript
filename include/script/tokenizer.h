#pragma once

#include "token.h"

#include <vector>
#include <string>
#include <map>


namespace script {

class tokenizer {

	public: 

	                            tokenizer();
	std::vector<token>          from_string(const std::string&);
	std::vector<token>          from_file(const std::string&);

	private:

	void                        peel_token(std::string&, std::vector<token>&, std::vector<token>&, bool&, int);
	bool                        try_keyword(const std::string&, std::vector<token>&, int);
	bool                        try_boolean(const std::string&, std::vector<token>&, int);
	bool                        try_integer(const std::string&, std::vector<token>&, int);

	std::map<std::string, token::types> typemap;
};

}
