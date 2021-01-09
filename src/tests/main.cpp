#include <iostream>

#include "ascript/tokenizer.h"
#include "ascript/parser.h"
#include "ascript/interpreter.h"

int main(
	int _argc,
	char ** _argv
) {

	if(2!=_argc) {

		std::cerr<<"use test filename"<<std::endl;
		return 1;
	}

	try {
		ascript::tokenizer tk;
		const auto tokens=tk.from_file(_argv[1]);
/*
std::cout<<"token list ----------------"<<std::endl;
		for(const auto& r : tokens) {
			std::cout<<r<<std::endl;
		}
*/
		ascript::parser p;
		const auto scripts=p.parse(tokens);
/*
std::cout<<"instruction list ----------"<<std::endl;
		for(const auto& script : scripts) {

			std::cout<<script<<std::endl;
		}
*/

		ascript::interpreter i;
		i.run(scripts.at(0), {"lol", true});
		i.run(scripts.at(1), {});

		return 0;
	}
	catch(std::exception& e) {

		std::cout<<"error: "<<e.what()<<std::endl;
		return 1;
	}
}
