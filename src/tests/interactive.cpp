#include <iostream>
#include <string>
#include <sstream>
#include <tuple>

#include "ascript/tokenizer.h"
#include "ascript/parser.h"
#include "ascript/host.h"
#include "ascript/interpreter.h"
#include "ascript/stdout_out.h"

//TODO: rewrite this example using the "environment" class.

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

using function_table=std::map<std::string, ascript::function>;
using interpreter_representation=std::tuple<std::size_t, std::string, ascript::interpreter>;

std::string prompt();
bool process_input(std::stringstream&, function_table&, script_host&, ascript::out_interface&, std::vector<interpreter_representation>& _interpreters, std::size_t& _current_id);
void help();
void load_file(std::stringstream&, function_table&);
void dump_functions(const function_table&);
void erase_function(std::stringstream&, function_table&);
void status(const std::vector<interpreter_representation>&);
void remove_interpreter(std::stringstream&, std::vector<interpreter_representation>&);
void resume_interpreter(std::stringstream&, std::vector<interpreter_representation>&);
void run(std::stringstream&, std::vector<interpreter_representation>&, script_host&, ascript::out_interface&, const function_table&, std::size_t&);
void add_symbol(std::stringstream&, script_host&);
void erase_symbol(std::stringstream&, script_host&);
void dump_table(const script_host&);
ascript::variable var_from_string(const std::string&);

int main(int, char **) {

	function_table functions;
	script_host sh;
	ascript::stdout_out outfacility;
	std::vector<interpreter_representation> interpreters;

	std::cout<<"type 'help' for help"<<std::endl<<std::endl;
	std::size_t current_id=0;

	while(true) {

		try {

			std::stringstream ss(prompt());
			if(!process_input(ss, functions, sh, outfacility, interpreters, current_id)) {
				break;
			}
		}
		catch(std::exception& e) {

			std::cout<<"error: "<<e.what()<<std::endl;
		}
	}

	return 0;
}


std::string prompt() {

	std::cout<<">>";
	std::string in;
	std::getline(std::cin, in);
	return in;
}


bool process_input(
	std::stringstream& _ss,
	function_table& _functions,
	script_host& _host,
	ascript::out_interface& _outfacility,
	std::vector<interpreter_representation>& _interpreters,
	std::size_t& _current_id
) {
	std::string command;
	_ss>>command;

	if(command=="help") {

		help();
	}
	else if(command=="exit") {

		return false;
	}
	else if(command=="load") {

		load_file(_ss, _functions);
	}
	else if(command=="show_functions") {

		dump_functions(_functions);
	}
	else if(command=="erase_function") {

		erase_function(_ss, _functions);
	}
	else if(command=="run") {

		run(_ss, _interpreters, _host, _outfacility, _functions, _current_id);
	}
	else if(command=="status") {

		status(_interpreters);
	}
	else if(command=="add_symbol") {

		add_symbol(_ss, _host);
	}
	else if(command=="erase_symbol") {

		erase_symbol(_ss, _host);
	}
	else if(command=="show_symbols") {

		dump_table(_host);
	}
	else if(command=="resume") {

		resume_interpreter(_ss, _interpreters);
	}
	else if(command=="end") {

		remove_interpreter(_ss, _interpreters);
	}
	else {
		std::cout<<"unknown command '"<<command<<"', type 'help' for help"<<std::endl;
	}

	return true;
}

void remove_interpreter(
	std::stringstream& _ss, 
	std::vector<interpreter_representation>& _interpreters
) {

	if(_ss.eof()) {

		std::cout<<"error: provide at least one interpreter id"<<std::endl;
		return;
	}

	std::vector<std::size_t> ids;

	while(true) {

		std::size_t id;
		_ss>>id;

		ids.push_back(id);

		if(_ss.eof()) {

			break;
		}
	}

	const auto it=std::remove_if(
		std::begin(_interpreters),
		std::end(_interpreters),
		[ids](const interpreter_representation& _int) {

			return std::find(
				std::begin(ids),
				std::end(ids),
				std::get<0>(_int)
			)!=std::end(ids);
		}
	);

	_interpreters.erase(it, std::end(_interpreters));
}

void resume_interpreter(
	std::stringstream& _ss, 
	std::vector<interpreter_representation>& _interpreters
) {

	if(_ss.eof()) {

		std::cout<<"error: provide the interpreter id"<<std::endl;
		return;
	}

	std::size_t id;
	_ss>>id;

	auto it=std::find_if(
		std::begin(_interpreters),
		std::end(_interpreters),
		[id](const interpreter_representation& _int) {
			return std::get<0>(_int)==id;
		}
	);

	if(it==std::end(_interpreters)) {

		std::cout<<"bad id"<<std::endl;
		return;
	}

	auto& interpreter=std::get<2>(*it);

	if(interpreter.is_failed()) {

		std::cout<<"cannot resume failed interpreter"<<std::endl;
	}

	if(interpreter.is_finished()) {

		std::cout<<"already finished"<<std::endl;
		return;
	}

	auto result=interpreter.resume();

	if(result) {

		std::cout<<"execution return with "<<result.get()<<std::endl;
	}
	else if(result.is_yield()) {

		std::cout<<"interpreter is in yield state"<<std::endl;
	}
	else if(result.is_nothing()) {

		std::cout<<"execution returned nothing"<<std::endl;
	}
}

void help() {

	std::cout<<R"str(
exit: exit interpreter
help: shows this help

load [filename...]: loads functions from the given filename(s)
show_functions: shows the list of functions
erase_function [funcname...]: erases the given function(s)

status : shows interpreter status
run [funcname int:param str:param double:param bool:param]: runs the given function in a new interpreter with the given parameters
resume [interpreterid] : resumes the execution of an interpreter.
end [interpreterid...] : removes an interpreter (or more)

show_symbols: shows the current symbol table
add_symbol [name type:val]: adds a value (int, bool, string, double) to the symbol table
erase_symbol [name]: erases the given symbol
)str"
	<<std::endl;
}

void status(
	const std::vector<interpreter_representation>& _interpreters
) {

	if(!_interpreters.size()) {
		
		std::cout<<"no interpreters"<<std::endl;
		return;
	}

	for(const auto& entry : _interpreters) {

		std::cout<<std::get<0>(entry)<<": "<<std::get<1>(entry)<<" -> ";

		const auto& interpreter=std::get<2>(entry);
		if(interpreter.is_failed()) {
			std::cout<<"[failed]";
		}
		else if(!interpreter.is_finished()) {
			std::cout<<"[yielding for "<<interpreter.get_yield_ms_left()<<" ms]";
		}
		else {
			std::cout<<"[finished]";
		}

		std::cout<<std::endl;
	}
}

void dump_table(
	const script_host& _host
) {

	for(const auto pair : _host.symbol_table) {

		std::cout<<pair.first<<" : "<<pair.second<<std::endl;
	}
}

void add_symbol(
	std::stringstream& _ss, 
	script_host& _host
) {

	std::string varname, vardata;
	_ss>>varname>>vardata;

	_host.symbol_table.insert(std::make_pair(varname, var_from_string(vardata)));
}

void erase_symbol(
	std::stringstream& _ss, 
	script_host& _host
) {

	if(_ss.eof()) {

		std::cout<<"error: provide at least one symbol name"<<std::endl;
		return;
	}

	while(true) {

		std::string symbol;
		_ss>>symbol;

		if(!_host.symbol_table.count(symbol)) {
			std::cout<<"symbol "<<symbol<<" does not exist!"<<std::endl;
		}
		else {
			
			_host.symbol_table.erase(symbol);
		std::cout<<"symbol "<<symbol<<" erased."<<std::endl;
		}

		if(_ss.eof()) {
			return;
		}
	}
}

void erase_function(
	std::stringstream& _ss, 
	function_table& _functions
) {
	if(_ss.eof()) {

		std::cout<<"error: provide at least one function name"<<std::endl;
		return;
	}

	while(true) {

		std::string funcname;
		_ss>>funcname;

		if(!_functions.count(funcname)) {

			std::cout<<"warning: function '"<<funcname<<"' was not loaded"<<std::endl;
		}
		else {

			_functions.erase(funcname);
			std::cout<<"function "<<funcname<<" erased."<<std::endl;
		}

		if(_ss.eof()) {
			return;
		}
	}
}

void dump_functions(
	const function_table& _functions
) {

	for(const auto& pair : _functions) {

		std::cout<<pair.second.name<<std::endl;
	}
}

void load_file(
	std::stringstream& _ss,
	function_table& _functions
) {

	if(_ss.eof()) {

		std::cout<<"error: provide at least one filename"<<std::endl;
		return;
	}

	while(true) {

		std::string filename;
		_ss>>filename;

		std::cout<<"reading from "<<filename<<"..."<<std::endl;
		ascript::tokenizer tk;
		const auto tokens=tk.from_file(filename);

		ascript::parser p;
		auto scripts=p.parse(tokens);
		for(auto& s : scripts) {

			const std::string funcname=s.name;
			
			if(_functions.count(funcname)) {
			
				std::cout<<"warning: there is already a function named '"<<funcname<<"', will not be loaded"<<std::endl;
				continue;
			}

			_functions.emplace(std::make_pair(s.name, std::move(s)));
			std::cout<<"loaded "<<funcname<<std::endl;
		}

		if(_ss.eof()) {
			return;
		}
	}
}

void run(
	std::stringstream& _ss, 
	std::vector<interpreter_representation>& _interpreters, 
	script_host& _host, 
	ascript::out_interface& _out,
	const function_table& _functions,
	std::size_t& _current_id
) {

	if(_ss.eof()) {

		std::cout<<"error: provide at least the function name"<<std::endl;
		return;
	}

	std::string funcname;
	_ss>>funcname;

	std::vector<ascript::variable> arguments;

	if(!_ss.eof()) {

		while(true) {

			std::string paramdata;
			_ss>>paramdata;
			arguments.push_back(var_from_string(paramdata));
			
			if(_ss.eof()) {
				break;
			}
		}
	}

	ascript::interpreter interpreter;
	for(const auto& pair: _functions) {
		interpreter.add_function(pair.second);
	}

	_interpreters.push_back({
		++_current_id,
		funcname,
		std::move(interpreter)
	});

	auto result=std::get<2>(_interpreters.back()).run(_host, _out, funcname, arguments);

	if(result) {

		std::cout<<"execution return with "<<result.get()<<std::endl;
	}
	else if(result.is_yield()) {

		std::cout<<"interpreter is in yield state"<<std::endl;
	}
	else if(result.is_nothing()) {

		std::cout<<"execution returned nothing"<<std::endl;
	}
}

ascript::variable var_from_string(
	const std::string& _str
) {

	auto colonpos=_str.find(':');
	if(colonpos==std::string::npos) {

		throw std::runtime_error("error in value expression syntax (type:value)");
	}

	std::string type=_str.substr(0, colonpos);
	if(type=="int") {

		return std::stoi(_str.substr(colonpos+1));
	}
	else if(type=="bool") {

		std::string boolval=_str.substr(colonpos+1);
		return boolval=="true";
	}
	else if(type=="double") {

		return std::stod(_str.substr(colonpos+1));
	}
	else if(type=="string") {

		return _str.substr(colonpos+1);
	}
	else {

		throw std::runtime_error("error in value expression, unknown type");
	}

	return false;
}

