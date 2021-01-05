#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ostream>

namespace ascript {

struct variable {

	enum class              types{boolean, integer, string, decimal, symbol} type{types::integer};

	                        variable(bool);
	                        variable(int);
	                        variable(double);
	                        variable(const std::string&);
	                        variable(const std::string&, types); //hacky symbol constructor.
	bool                    bool_val{false};
	int                     int_val{0};
	double                  double_val{0.};
	std::string             str_val;
};

//TODO: Perhaps these should be in their own namespace.

//base class for all script instructions.
struct instruction {

	virtual                 ~instruction(){}
	virtual void            format_out(std::ostream&) const=0;
};

//Base class for all instructions that will execute something without returning
//anything.
struct instruction_procedure:instruction {

	std::vector<variable>   arguments;
};

//instruction to print something out.
struct instruction_out:instruction_procedure {

	void                    format_out(std::ostream&) const;
};

//instruction to stop execution of script with error
struct instruction_fail:instruction_procedure {

	void                    format_out(std::ostream&) const;
};

struct instruction_host_set:instruction_procedure {

	void                    format_out(std::ostream&) const;
};

struct instruction_host_add:instruction_procedure {

	void                    format_out(std::ostream&) const;
};

struct instruction_host_do:instruction_procedure {

	void                    format_out(std::ostream&) const;
};

//Base class for all instructions that will generate a value, stuff like
//is_equal, host_query...
struct instruction_function:instruction {

	std::vector<variable>   arguments;
};


//instruction to compare the first parameter with the rest
struct instruction_is_equal:instruction_function {

	void                    format_out(std::ostream&) const;
};

//instruction to compare the first parameter with the rest
struct instruction_is_lesser_than:instruction_function {

	void                    format_out(std::ostream&) const;
};

//instruction to compare the first parameter with the rest
struct instruction_is_greater_than:instruction_function {

	void                    format_out(std::ostream&) const;
};

struct instruction_host_has:instruction_function {

	void                    format_out(std::ostream&) const;
};

struct instruction_is_int:instruction_function {

	void                    format_out(std::ostream&) const;
};

struct instruction_is_bool:instruction_function {

	void                    format_out(std::ostream&) const;
};

struct instruction_is_double:instruction_function {

	void                    format_out(std::ostream&) const;
};

struct instruction_is_string:instruction_function {

	void                    format_out(std::ostream&) const;
};

struct instruction_host_get:instruction_function {

	void                    format_out(std::ostream&) const;
};

struct instruction_host_query:instruction_function {

	void                    format_out(std::ostream&) const;
};

//instruction to declare a variable with a static value "let x be y;"
struct instruction_declaration_static:instruction {

	                        instruction_declaration_static(const std::string&, const variable&);
	std::string             identifier;
	variable                value;
	void                    format_out(std::ostream&) const;
};

//instruction to declare a variable with a dynamic value derived from a 
//function "let x be lala [a, b, c];"
struct instruction_declaration_dynamic:instruction {

	                        instruction_declaration_dynamic(const std::string&, std::unique_ptr<instruction_function>&);
	std::string             identifier;
	std::unique_ptr<instruction_function> function;
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

//each branch of an if is represented by one of these: evaluation function
//and index of the target context to execute.
struct conditional_path {

	std::unique_ptr<instruction_function>   function;
	int                                     target_context_index;
	bool                                    negated;
};

//!instruction to execute conditional logic.
struct instruction_conditional_branch:instruction {

	std::vector<conditional_path>           branches;

	void                    format_out(std::ostream&) const;
};

//!instruction to execute a loop.
struct instruction_loop:instruction {

	                        instruction_loop(int);
	int                     target_context_index;

	void                    format_out(std::ostream&) const;
};

//!A block of instructions, looped or linear.
struct context {

	enum class types {linear, loop}             type;
	std::vector<std::unique_ptr<instruction>>  instructions;
};

//!A script definition. A script is made up of a list of contexts, whose 
//!first is the main one. Plus a list of argument names that must be inserted
//!on the table.
struct script {

	std::string                                 name;
	std::vector<context>                        contexts;
	//TODO: We'll see about that when it comes to implementing it.
	std::vector<std::string>                    parameter_names;
};

std::ostream& operator<<(std::ostream&, const variable&);
std::ostream& operator<<(std::ostream&, const instruction&);
std::ostream& operator<<(std::ostream&, const conditional_path&);
std::ostream& operator<<(std::ostream&, const context&);
std::ostream& operator<<(std::ostream&, const script&);



}
