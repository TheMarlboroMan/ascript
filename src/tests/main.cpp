#include <iostream>

#include "script/tokenizer.h"
#include "script/parser.h"

int main(
	int _argc,
	char ** _argv
) {

	if(2!=_argc) {

		std::cerr<<"use test filename"<<std::endl;
		return 1;
	}

	try {
		script::tokenizer tk;
		const auto tokens=tk.from_file(_argv[1]);

//		for(const auto& r : tokens) {
//		std::cout<<r<<std::endl;
//		}

		script::parser p;
		p.parse(tokens);

		return 0;
	}
	catch(std::exception& e) {

		std::cout<<"error: "<<e.what()<<std::endl;
		return 1;
	}
}
