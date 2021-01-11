#pragma once

#include "instructions.h"
#include "run_context.h"
#include "out_interface.h"

#include <vector>
#include <string>

namespace ascript {

//!This is a stack... the interpreter keeps a list of these 
struct stack {

	const function *                current_function{nullptr};
	int                             block_index,
	                                instruction_index;
	run_context                     context;
};

//!A function runner. Runs a single function at a time.

class interpreter {

	public:

	//!Directly runs a function object.
	void                run(host&, out_interface&, const function&, const std::vector<variable>&);

	//!Runs a named function that should have been added before.
	void                run(host&, out_interface&, const std::string&, const std::vector<variable>&);

	//!Returns true if the function has finished executing (only makes sense if
	//!the function yielded.
	bool                is_finished() const {return !is_failed() && stacks.size()==0;}

	//!Returns true if the function failed.
	bool                is_failed() const {return failed_signal;}

	//!Resumes a yielding execution. Throws if the execution is not stopped.
	void                resume();

	//!Returns true if a function with the given name can be found;
	bool                has_function(const std::string& _funcname) const {

		return functions.count(_funcname);
	}

	//!Removes a function by name. Will throw if a function by the given name
	//!does not exist.
	void                remove_function(const std::string&);

	//!Adds a function. The function object MUST outlive the interpreter. 
	//!This includes the use of the parser, that must not go out of scope. Will
	//!throw if a function by that name exists. Notice that the function 
	//!object already includes its name so they cannot be aliased.
	void                add_function(const function&);

	private:

	void                interpret();
	std::map<std::string, variable> prepare_symbol_table(const function&, const std::vector<variable>&, int);
	void                push_stack(const function *, int);
	void                push_stack(const function *, int, std::map<std::string, variable>&);
	void                pop_stack(bool, int);

	//!Functions are implied to be owned by some other thing.
	std::map<std::string, const function *> functions;
	host *              current_host{nullptr};
	out_interface *     out_facility{nullptr};
	std::vector<stack>  stacks;
	stack *             current_stack{nullptr};
	bool                break_signal{false},
	                    yield_signal{false},
	                    failed_signal{false}; //Raised only when an exception is thrown.
};

}
