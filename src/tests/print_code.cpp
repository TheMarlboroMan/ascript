#include <iostream>
#include <string>

#include "ascript/tokenizer.h"
#include "ascript/parser.h"

int main(
	int _argc,
	char ** _argv
) {

	if(2!=_argc) {

		std::cerr<<"use print_code filename"<<std::endl;
		return 1;
	}

	try {
		ascript::tokenizer tk;
		const auto tokens=tk.from_file(_argv[1]);

		ascript::parser p;
		const auto scripts=p.parse(tokens);

		for(const auto& s : scripts) {

			std::cout<<s<<std::endl;
		}

		return 0;
	}
	catch(std::exception& e) {

		std::cout<<"error: "<<e.what()<<std::endl;
		return 1;
	}
}
