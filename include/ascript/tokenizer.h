#pragma once

#include "token.h"

#include <vector>
#include <string>
#include <map>


namespace ascript {

//!The tokenizer is the piece that takes a text containing a program and 
//!converts it into significant tokens that the parser can consume to produce
//!the final functions. The tokenizer knows no error states, whatever it is fed
//!it will try to interpret.
class tokenizer {

	public: 

	//!Class constructor.
	                            tokenizer();
	//!Retrieves a vector of tokens from the given string.
	std::vector<token>          from_string(const std::string&);
	//!Retrieves a vector of tokens from the given file.
	std::vector<token>          from_file(const std::string&);

	private:

	//!Internal function that peels a string from leading and read significant characters.
	void                        peel_token(std::string&, std::vector<token>&, std::vector<token>&, bool&, bool&, int);
	//!Tries to interpret a string as a keyword.
	bool                        try_keyword(const std::string&, std::vector<token>&, int);
	//!Tries to interpret a string as a boolean.
	bool                        try_boolean(const std::string&, std::vector<token>&, int);
	//!Tries to interpret a string as an integer.
	bool                        try_integer(const std::string&, std::vector<token>&, int);
	//!Tries to interpret a string as a double.
	bool                        try_double(const std::string&, std::vector<token>&, int);

	//!Stores a map of strings to tokens (like "loop" to the "loop token".
	std::map<std::string, token::types> typemap;
};

}
