#pragma once

#include "ascript/variable.h"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ostream>
#include <optional>

namespace ascript {

struct run_context;

//!base class for all script instructions.
/**
*Each action the script language can perform is represented as an instruction
*of a different type, all sharing a common ancestor. Instructions run oblivious 
*of the any interpreter and the interpreter itself does not try to typecast 
*them at all. As a result, all instructions are supposed to be able to perform
*whatever action or signal through a common "run_context" structure 
*/
struct instruction {

	                        instruction(int _line_number): line_number{_line_number} {}
	virtual                 ~instruction(){}

	int                     line_number; //!Stores the line number.

	//!For debug purposes, all instructions know how to print themselves.
	virtual void            format_out(std::ostream&) const=0;

	//!Executes the instruction.
/**
* Instructions receive a run_context to interact with the interpreter.
* Everything an instruction can do is limited to what can be expressed in the 
* run_context object.
*/
	virtual void            run(run_context&)const=0;
};

//!Base class for all instructions that will execute something without returning
//!anything.
struct instruction_procedure:instruction {

                            instruction_procedure(int _line_number):instruction{_line_number}{}
	virtual                 ~instruction_procedure(){}
	//!Stores procedure arguments.
	std::vector<variable>   arguments;
};

//!Base class for all instructions that will generate a value, stuff like
//!is_equal, host_query...
struct instruction_function:instruction {

                            instruction_function(int _line_number):instruction{_line_number}{}
	virtual                 ~instruction_function(){}
	//!All functions must be able to generate their value through a call to evaluate.
	virtual variable        evaluate(run_context&) const=0;
	//!Stores function arguments.
	std::vector<variable>   arguments;
};

////////////////////////////////////////////////////////////////////////////////
// Derived classes

//!instruction to print something out.
struct instruction_out:instruction_procedure {

                            instruction_out(int _line_number):instruction_procedure{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to stop execution of script with error
struct instruction_fail:instruction_procedure {

                            instruction_fail(int _line_number):instruction_procedure{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to set a value in the host.
struct instruction_host_set:instruction_procedure {

                            instruction_host_set(int _line_number):instruction_procedure{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to create a new value in the host.
struct instruction_host_add:instruction_procedure {

                            instruction_host_add(int _line_number):instruction_procedure{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to delete a value in the host.
struct instruction_host_delete:instruction_procedure {

                            instruction_host_delete(int _line_number):instruction_procedure{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to ask the host to perform a complex action.
struct instruction_host_do:instruction_procedure {

                            instruction_host_do(int _line_number):instruction_procedure{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//Functions.

//!Instruction that generates a value, for assignment purposes. It looks like
//!a function, but it really is not.
struct instruction_generate_value:instruction_function{

                            instruction_generate_value(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!Instruction to handle return values. Of internal use only, has no use for
//!end users.
struct instruction_copy_from_return_register:instruction_function{

                            instruction_copy_from_return_register(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to compare the first parameter with the rest (for equality).
struct instruction_is_equal:instruction_function {

                            instruction_is_equal(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to compare the first parameter with the rest (returns true when 
//!the first parameter is less than the others).
struct instruction_is_lesser_than:instruction_function {

                            instruction_is_lesser_than(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to compare the first parameter with the rest (returns true when
//!the first parameter is greater than the others).
struct instruction_is_greater_than:instruction_function {

                            instruction_is_greater_than(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to add n numeric parameters
struct instruction_add:instruction_function {

                            instruction_add(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to subsract n numeric parameters
struct instruction_substract:instruction_function {

                            instruction_substract(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to concatenate string parameters
struct instruction_concatenate:instruction_function {

                            instruction_concatenate(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to ask a host if it holds a symbol on its table.
struct instruction_host_has:instruction_function {

                            instruction_host_has(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!returns true if all of the parameters are of integer type.
struct instruction_is_int:instruction_function {

                            instruction_is_int(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!returns true if all of the parameters are of bool type.
struct instruction_is_bool:instruction_function {

                            instruction_is_bool(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!returns true if all of the parameters are of double type.
struct instruction_is_double:instruction_function {

                            instruction_is_double(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!returns true if all of the parameters are of string type.
struct instruction_is_string:instruction_function {

                            instruction_is_string(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to return a value from the host's symbol table.
struct instruction_host_get:instruction_function {

                            instruction_host_get(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

//!instruction to ask the host to perform a calculation and return its result.
struct instruction_host_query:instruction_function {

                            instruction_host_query(int _line_number):instruction_function{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                evaluate(run_context&) const;
};

////////////////////////////////////////////////////////////////////////////////
// Language instructions.

//!instruction to run a function call [fnname, params...];
struct instruction_function_call:instruction {

	                        instruction_function_call(int, const variable&, const std::vector<variable>&);
	variable                function_name;
	std::vector<variable>   arguments;
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to declare a variable. The dynamic part comes from a time where
//!there was a type for static declarations (let a be 33) and dynamic 
//!declarations (let a be fn [param]);
struct instruction_declaration_dynamic:instruction {

	                        instruction_declaration_dynamic(int, const std::string&, std::unique_ptr<instruction_function>&);
	std::string             identifier;
	std::unique_ptr<instruction_function> function;
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to assign a variable.
struct instruction_assignment_dynamic:instruction {

	                        instruction_assignment_dynamic(int, const std::string&, std::unique_ptr<instruction_function>&);
	std::string             identifier;
	std::unique_ptr<instruction_function> function;
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to return from a function, optionally with a value.
struct instruction_return:instruction {

                            instruction_return(int _line_number):instruction{_line_number}{}
                            instruction_return(int _line_number, variable _var):instruction{_line_number}, returned_value{_var}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	//!optional returned value.
	std::optional<variable> returned_value;
};

//!instruction to yield to the host. The script will remain stopped until 
//!the interpreter is asked to resume.
struct instruction_yield:instruction {

                            instruction_yield(int _line_number);
                            instruction_yield(int _line_number, const variable&);
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
	variable                yield_ms;
};

//!instruction to break of a loop.
struct instruction_break:instruction {

	                        instruction_break(int _line_number):instruction{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to fully abort execution.
struct instruction_exit:instruction {

	                        instruction_exit(int _line_number):instruction{_line_number}{}
	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!each branch of an if is represented by one of these: evaluation function
//!and index of the target block to execute.
struct conditional_path {

	std::unique_ptr<instruction_function>   function;
	int                                     target_block_index,
	                                        line_number;
	bool                                    negated;
};

//!instruction to execute conditional logic.
struct instruction_conditional_branch:instruction {

                            instruction_conditional_branch(int _line_number):instruction{_line_number}{}
	std::vector<conditional_path>           branches;

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!instruction to execute a loop.
struct instruction_loop:instruction {

	                        instruction_loop(int, int);
	int                     target_block_index;

	void                    format_out(std::ostream&) const;
	void                    run(run_context&)const;
};

//!a block of instructions, looped or linear.
struct block {

	enum class types {linear, loop}             type;
	std::vector<std::unique_ptr<instruction>>   instructions;
};

//!a parameter definition, which is a name and a type.
struct parameter {

	std::string                                 name;
	enum class types{integer, decimal, boolean, string, any} type;
};

//!a script definition. A script is made up of a list of blocks, whose 
//!first is the main one. Plus a list of argument names that must be inserted
//!on the table.
struct function {

	std::string                                 name;
	std::vector<block>                          blocks;
	std::vector<parameter>                      parameters;
};

//!Returns a vector of variables from the given vector of variables, resolving
//!any symbols.
std::vector<variable>   solve(const std::vector<variable>&, const std::map<std::string, variable>&, int);

//!returns a variable from the given variable, resolving it if it's a symbol.
variable                solve(const variable&, const std::map<std::string, variable>&, int);

//!output stream operator for an instruction, for debug purposes.
std::ostream& operator<<(std::ostream&, const instruction&);

//!output stream operator for a conditional path, for debug purposes.
std::ostream& operator<<(std::ostream&, const conditional_path&);

//!output stream operator for a block, for debug purposes.
std::ostream& operator<<(std::ostream&, const block&);

//!output stream operator for a parameter, for debug purposes.
std::ostream& operator<<(std::ostream&, const parameter&);

//!output stream operator for a function, for debug purposes.
std::ostream& operator<<(std::ostream&, const function&);


}
