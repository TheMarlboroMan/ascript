#include "ascript/tokenizer.h"

#include <tools/file_utils.h>
#include <tools/string_utils.h>
#include <tools/string_reader.h>

//TODO: erase.
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace ascript;

tokenizer::tokenizer() {

	typemap["is_equal"]=token::types::fn_is_equal;
	typemap["is_greater_than"]=token::types::fn_is_greater_than;
	typemap["is_lesser_than"]=token::types::fn_is_lesser_than;
	typemap["not"]=token::types::kw_not;
	typemap["if"]=token::types::kw_if;
	typemap["elseif"]=token::types::kw_elseif;
	typemap["else"]=token::types::kw_else;
	typemap["endif"]=token::types::kw_endif;
	typemap["loop"]=token::types::kw_loop;
	typemap["break"]=token::types::kw_break;
	typemap["endloop"]=token::types::kw_endloop;
	typemap["yield"]=token::types::kw_yield;
	typemap["return"]=token::types::kw_return;
	typemap["fail"]=token::types::pr_fail;
	typemap["let"]=token::types::kw_let;
	typemap["be"]=token::types::kw_be;
	typemap["int"]=token::types::kw_integer;
	typemap["string"]=token::types::kw_string;
	typemap["bool"]=token::types::kw_bool;
	typemap["as"]=token::types::kw_as;
	typemap["host_has"]=token::types::fn_host_has;
	typemap["host_add"]=token::types::pr_host_add;
	typemap["host_get"]=token::types::fn_host_get;
	typemap["host_set"]=token::types::pr_host_set;
	typemap["host_query"]=token::types::fn_host_query;
	typemap["host_do"]=token::types::pr_host_do;
	typemap["out"]=token::types::pr_out;
	typemap["beginfunction"]=token::types::kw_beginfunction;
	typemap["endfunction"]=token::types::kw_endfunction;
}

std::vector<ascript::token> tokenizer::from_file(
	const std::string& _filename
) {

	return from_string(tools::dump_file(_filename));
}

std::vector<ascript::token> tokenizer::from_string(
	const std::string& _str
) {

	std::vector<ascript::token> result;
	tools::string_reader reader{_str, '#', tools::string_reader::flags::ltrim | tools::string_reader::flags::rtrim};

	while(true) {

		std::stringstream ss{reader.read_line()};
		if(reader.is_eof()) {

			break;
		}

		int line_number=reader.get_line_number();

		//Start reading tokens. . 

		std::string strtoken;
		while(true) {

			if(ss.eof()) {
				break;
			}

			ss>>strtoken;

			std::vector<token> affix;

			bool starts_string{false};
			peel_token(strtoken, result, affix, starts_string, line_number);

			//Did it start a string literal???
			if(starts_string) {

				//a string of a single word...
				if(strtoken.back()=='"') {

					strtoken.pop_back();
					result.push_back({token::types::val_string, strtoken, 0, false, line_number});
					if(affix.size()) {

						result.insert(std::end(result), std::rbegin(affix), std::rend(affix));
					}

					continue;
				}

				//read more until reaching "...
				while(true) {
					if(ss.peek()=='"') {

						ss.get();
						result.push_back({token::types::val_string, strtoken, 0, false, line_number});
						break;
					}
					else {

						strtoken+=ss.get();
						if(ss.eof()) {
							break; //Strings do not continue into new lines.
						}
					}
				}

				//Add affixes.in reverse order.
				if(affix.size()) {
					result.insert(std::end(result), std::rbegin(affix), std::rend(affix));
				}

				continue;
			}

			//This is a disaster... we may find a token like "], which is two
			//affixes.
			if(strtoken.size()) {

				//Maybe it is a known keyword...
				if(try_keyword(strtoken, result, line_number)) {

					//Noop.
				}
				else if(try_boolean(strtoken, result, line_number)) {

					//Noop.
				}
				else if(try_integer(strtoken, result, line_number)) {

					//Noop.
				}
				else {

					//Well, an identifier it is...
					result.push_back({token::types::identifier, strtoken, 0, false, line_number});
				}
			}

			//Add affixes.in reverse order.
			if(affix.size()) {
				
				result.insert(std::end(result), std::rbegin(affix), std::rend(affix));
			}
		}
	}

	return result;
}

void tokenizer::peel_token(
	std::string& _strtoken, 
	std::vector<token>& _result,
	std::vector<token>& _affix,
	bool& _starts_string,
	int _line_number
) {

	if(!_strtoken.size()) {

		return;
	}

	//Peel of any special tokens at the beginning, comma, token, open bracket.
	char first=_strtoken.front();
	if(first=='"') {

		_strtoken.erase(0, 1);
		_starts_string=true;
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}

	if(first==';') {

		_result.push_back({token::types::semicolon, "", 0, false, _line_number});
		_strtoken.erase(0, 1);
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}

	if(first==',') {

		_result.push_back({token::types::comma, "", 0, false, _line_number});
		_strtoken.erase(0, 1);
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}
	
	if(first==']') {

		_result.push_back({token::types::close_bracket, "", 0, false, _line_number});
		_strtoken.erase(0, 1);
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}

	if(first=='[') {

		_result.push_back({token::types::open_bracket, "", 0, false, _line_number});
		_strtoken.erase(0, 1);
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}

	//There may  be tokens to affix...
	char last=_strtoken.back();
	if(last==';') {

		_affix.push_back({token::types::semicolon, "", 0, false, _line_number});
		_strtoken.pop_back();
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}

	if(last==',') {

		_affix.push_back({token::types::comma, "", 0, false, _line_number});
		_strtoken.pop_back();
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}
	
	if(last==']') {

		_affix.push_back({token::types::close_bracket, "", 0, false, _line_number});
		_strtoken.pop_back();
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}

	if(last=='[') {

		_affix.push_back({token::types::open_bracket, "", 0, false, _line_number});
		_strtoken.pop_back();
		return peel_token(_strtoken, _result, _affix, _starts_string, _line_number);
	}

	//The rest, is the regular token to work with.
}

bool tokenizer::try_keyword(
	const std::string& _strtoken, 
	std::vector<token>& _result,
	int _line_number
) {

	if(typemap.count(_strtoken)) {

		_result.push_back({typemap.at(_strtoken), "", 0, false, _line_number});
		return true;
	}

	return false;
}

bool tokenizer::try_boolean(
	const std::string& _strtoken,
	std::vector<token>& _result,
	int _line_number
) {

	if(_strtoken=="true") {
	
		_result.push_back({token::types::val_bool, "", 0, true, _line_number});
		return true;
	}

	if(_strtoken=="false") {
	
		_result.push_back({token::types::val_bool, "", 0, false, _line_number});
		return true;
	}
	
	return false;
}

bool tokenizer::try_integer(
	const std::string& _strtoken,
	std::vector<token>& _result,
	int _line_number
) {

	char * c;
	long int n = std::strtol(_strtoken.c_str(), &c, 10);
	if(*c == 0) {

		_result.push_back({token::types::val_int, "", (int)n, false, _line_number});
		return true;
	}

	return false;
}

