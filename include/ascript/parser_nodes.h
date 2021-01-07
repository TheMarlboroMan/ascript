#pragma once

#include "ascript/variable.h"

#include <string>
#include <vector>
#include <memory>
#include <ostream>

namespace ascript {

//TODO: Perhaps these should be in their own namespace.

struct run_context;

//base class for all script instructions.
struct instruction {

	virtual                 ~instruction(){}

	//!For debug purposes, all instructions know how to print themselves.
	virtual void            format_out(std::ostream&) const=0;
	virtual void            run(run_context&)const=0;
};

//Base class for all instructions that will execute something without returning
//anything.
struct instruction_procedure:instruction {

	virtual                 ~instruction_procedure(){}
	std::vector<variable>   arguments;
};

//Base class for all instructions that will generate a value, stuff like
//is_equal, host_query...
struct instruction_function:instruction {

	virtual                 ~instruction_function(){}
	std::vector<variable>   arguments;
};

////////////////////////////////////////////////////////////////////////////////
// Derived classes

//instruction to print something out.
struct instruction_out:instruction_procedure {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to stop execution of script with error
struct instruction_fail:instruction_procedure {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_host_set:instruction_procedure {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_host_add:instruction_procedure {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_host_do:instruction_procedure {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to compare the first parameter with the rest
struct instruction_is_equal:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to compare the first parameter with the rest
struct instruction_is_lesser_than:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to compare the first parameter with the rest
struct instruction_is_greater_than:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_host_has:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_is_int:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_is_bool:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_is_double:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_is_string:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_host_get:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

struct instruction_host_query:instruction_function {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to declare a variable with a static value "let x be y;"
struct instruction_declaration_static:instruction {

	                        instruction_declaration_static(const std::string&, const variable&);
	std::string             identifier;
	variable                value;
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to declare a variable with a dynamic value derived from a 
//function "let x be lala [a, b, c];"
struct instruction_declaration_dynamic:instruction {

	                        instruction_declaration_dynamic(const std::string&, std::unique_ptr<instruction_function>&);
	std::string             identifier;
	std::unique_ptr<instruction_function> function;
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to exit a script.
struct instruction_return:instruction {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to yield cycles to the host.
struct instruction_yield:instruction {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//instruction to break of loop.
struct instruction_break:instruction {

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//each branch of an if is represented by one of these: evaluation function
//and index of the target block to execute.
struct conditional_path {

	std::unique_ptr<instruction_function>   function;
	int                                     target_block_index;
	bool                                    negated;
};

//!instruction to execute conditional logic.
struct instruction_conditional_branch:instruction {

	std::vector<conditional_path>           branches;

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to execute a loop.
struct instruction_loop:instruction {

	                        instruction_loop(int);
	int                     target_block_index;

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!A block of instructions, looped or linear.
struct block {

	enum class types {linear, loop}             type;
	std::vector<std::unique_ptr<instruction>>   instructions;
};

//!A script definition. A script is made up of a list of blocks, whose 
//!first is the main one. Plus a list of argument names that must be inserted
//!on the table.
struct function {

	std::string                                 name;
	std::vector<block>                          blocks;
	//TODO: We'll see about that when it comes to implementing it.
	std::vector<std::string>                    parameter_names;
};

std::ostream& operator<<(std::ostream&, const instruction&);
std::ostream& operator<<(std::ostream&, const conditional_path&);
std::ostream& operator<<(std::ostream&, const block&);
std::ostream& operator<<(std::ostream&, const function&);

}
