#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ostream>

namespace script {

struct variable {

	enum class              types{boolean, integer, string, symbol} type{types::integer};

	                        variable(bool);
	                        variable(int);
	                        variable(const std::string&);
	                        variable(const std::string&, types); //hacky symbol constructor.
	bool                    bool_val{false};
	int                     int_val{0};
	std::string             str_val;
};

//base class for all script instructions.
struct instruction {

	virtual                 ~instruction(){}
	virtual void            format_out(std::ostream&) const=0;
};

//Base class for all instructions that will generate a value, stuff like
//is_equal, host_query...
struct instruction_function:instruction {

};

//instruction to print something out.
struct instruction_out:instruction {

	                        instruction_out(std::vector<variable>&);
	std::vector<variable>   parameters;

	void                    format_out(std::ostream&) const;
};

//instruction to stop execution of script with error
struct instruction_fail:instruction {

	                        instruction_fail(std::vector<variable>&);
	std::vector<variable>   parameters;

	void                    format_out(std::ostream&) const;
};

//instruction to compare the first parameter with the rest
struct instruction_is_equal:instruction_function {

	                        instruction_is_equal(std::vector<variable>&);
	std::vector<variable>   parameters;

	void                    format_out(std::ostream&) const;
};

//instruction to declare a variable with a static value "let x be y;"
struct instruction_declaration_static:instruction {

	                        instruction_declaration_static(const std::string&, const variable&);
	std::string             identifier;
	variable                value;
	void                    format_out(std::ostream&) const;
};

//instruction to exit a script.
struct instruction_return:instruction {

	void                    format_out(std::ostream&) const;
};

//instruction to yield cycles to the host.
struct instruction_yield:instruction {

	void                    format_out(std::ostream&) const;
};

//instruction to break of loop.
struct instruction_break:instruction {

	void                    format_out(std::ostream&) const;
};

struct context {

	enum class types {linear, loop}             type;
	std::vector<std::unique_ptr<instruction>>  instructions;
};

struct script {

	std::string                                 name;
	std::vector<context>                        contexts;
};

std::ostream& operator<<(std::ostream&, const variable&);
std::ostream& operator<<(std::ostream&, const instruction&);
std::ostream& operator<<(std::ostream&, const context&);
std::ostream& operator<<(std::ostream&, const script&);



}
