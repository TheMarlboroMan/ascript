#include <iostream>
#include <string>

#include "ascript/tokenizer.h"
#include "ascript/parser.h"
#include "ascript/host.h"
#include "ascript/interpreter.h"

class script_host:
	public ascript::host {

	public:

	bool                host_has(const std::string _symbol) const {return symbol_table.count(_symbol);}

	void                host_delete(const std::string _symbol) {

		if(!symbol_table.count(_symbol)) {

			throw ascript::host_error(_symbol+" -> host_delete -> symbol not defined");
		}

		symbol_table.erase(_symbol);
	}

	ascript::variable   host_get(const std::string _symbol) const {

		if(!symbol_table.count(_symbol)) {

			throw ascript::host_error(_symbol+" -> host_get -> symbol not defined");
		}

		return symbol_table.at(_symbol);
	}

	ascript::variable   host_query(const std::vector<ascript::variable>&) const {

		return {false};
	}

	void                host_add(
		const std::string& _symbol, 
		ascript::variable _val
	) {

		if(symbol_table.count(_symbol)) {

			throw ascript::host_error(
				std::string{"symbol '"}
				+_symbol
				+"'	already defined");
		}

		symbol_table.insert(std::make_pair(_symbol, _val));
	}

	void                host_set(
		const std::string& _symbol, 
		ascript::variable _val
	) {

		if(!symbol_table.count(_symbol)) {

			throw ascript::host_error(_symbol+" -> host_set -> symbol not defined");
		}

		symbol_table.at(_symbol)=_val;
	}

	//!Must send a procedure message to the host.
	void                host_do(const std::vector<ascript::variable>&) {

	}

	std::map<std::string, ascript::variable> symbol_table;
};

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

		ascript::parser p;
		const auto scripts=p.parse(tokens);

		script_host sh;
		sh.symbol_table.insert(std::make_pair("var", 12));
		sh.symbol_table.insert(std::make_pair("some_string", "this string"));

		ascript::interpreter i;

		for(const auto& s : scripts) {
			i.add_function(s);
//			std::cout<<s<<std::endl;
		}

//		i.run(sh, "print_stuff", {"lol", true});

		i.run(sh, "main", {});

		while(true) {

			if(i.is_finished()) {

				break;
			}
			else {

				std::string in;
				std::getline(std::cin, in);

				if(in=="done") {

					sh.symbol_table.insert(std::make_pair("done", true));
				}
				else if(in=="exit") {

					sh.symbol_table.insert(std::make_pair("exit", true));
				}

				i.resume();
			}
		}

		return 0;
	}
	catch(std::exception& e) {

		std::cout<<"error: "<<e.what()<<std::endl;
		return 1;
	}
}
