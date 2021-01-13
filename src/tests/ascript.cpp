#include <iostream>
#include <string>
#include <sstream>

#include "ascript/tokenizer.h"
#include "ascript/parser.h"
#include "ascript/host.h"
#include "ascript/interpreter.h"
#include "ascript/stdout_out.h"

//Basic host implementation for demo purposes.
class script_host:
	public ascript::host {

	public:

	bool                host_has(
		const std::string _symbol
	) const {
		return symbol_table.count(_symbol);
	}

	void                host_delete(
		const std::string _symbol
	) {

		if(!symbol_table.count(_symbol)) {

			throw ascript::host_error(_symbol+" -> host_delete -> symbol not defined");
		}

		symbol_table.erase(_symbol);
	}

	ascript::variable   host_get(
		const std::string _symbol
	) const {

		if(!symbol_table.count(_symbol)) {

			throw ascript::host_error(_symbol+" -> host_get -> symbol not defined");
		}

		return symbol_table.at(_symbol);
	}

	ascript::variable   host_query(
		const std::vector<ascript::variable>&
	) const {

		//Unimplemented.
		throw ascript::host_error("host_query is unimplemented");
	}

	void                host_add(
		const std::string& _symbol, 
		ascript::variable _val
	) {

		if(symbol_table.count(_symbol)) {

			throw ascript::host_error(_symbol+" -> host_add -> already defined");
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

	void                host_do(
		const std::vector<ascript::variable>&
	) {
		//Unimplemented.
		throw ascript::host_error("host_do is unimplemented");
	}

	std::map<std::string, ascript::variable> symbol_table;
};

int main(
	int _argc, 
	char ** _argv
) {

	if(3 != _argc) {

		std::cerr<<"use ascript filename functionname"<<std::endl;
		return 1;
	}

	try {

		script_host sh;
		ascript::interpreter i;
		ascript::stdout_out outfacility;

		ascript::tokenizer tk;
		const auto tokens=tk.from_file(_argv[1]);

		ascript::parser p;
		const auto scripts=p.parse(tokens);
		for(const auto& s : scripts) {
			i.add_function(s);
		}

		auto result=i.run(sh, outfacility, _argv[2], {});

		if(result) {

			std::cout<<"interpreter got a return value of "<<result.get()<<std::endl;
		}
		else if(result.is_yield()) {

			std::cout<<"interpreter yielded, execution not finished"<<std::endl;
		}
		else if(result.is_nothing()) {

			std::cout<<"interpreter got no return value"<<std::endl;
		}
	}
	catch(std::exception& e) {

		std::cout<<"error: "<<e.what()<<std::endl;
		return 1;
	}

	return 0;
}

