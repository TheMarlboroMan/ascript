#pragma once

#include "instructions.h"
#include "run_context.h"
#include "return_value.h"
#include "out_interface.h"

#include <vector>
#include <string>
#include <chrono>

namespace ascript {

//!This is a stack... the interpreter keeps a list of these 
/**
* Each stack corresponds with a block, keeping track of the function the block
* belongs to, the block index (in the blocks vector) and the next instruction
* index to be executed. Of course, there's also the context, which contains 
* symbol tables and exchange values.
**/
struct stack {

	//!current stack function.
	const function *                current_function{nullptr};
	//!index of the block this stack corresponds to.
	int                             block_index,
	//!index of the next instruction to be executed.
	                                instruction_index;
	//!data context.
	run_context                     context;
};

//!The interpreter.
/**
* The interpreter is the outmost part of ascript. Client code spawns a 
* interprer and asks it to run any number of functions. After being asked to
* run a function, an interpreter can be in any of three states: finished 
* (the function executed correctly), yielding (the script yields control back
* to the outmost layer) or error (there was an error in the execution).
* 
*/
class interpreter {

	public:

	//!Directly runs a function object.
	return_value        run(host&, out_interface&, const function&, const std::vector<variable>&);

	//!Runs a named function that should have been added before.
	return_value        run(host&, out_interface&, const std::string&, const std::vector<variable>&);

	//!Resumes a yielding execution. Throws if the execution is not stopped.
	return_value        resume();

	//!Returns true if the function has finished executing correctly.
	//!the function yielded.
	bool                is_finished() const {return !is_failed() && stacks.size()==0;}

	//!Returns true if the function failed.
	bool                is_failed() const {return failed_signal;}

	//!Returns true if the interpreter is yielding.
	bool                is_yield() const {return yield_signal;}

	//!Returns the number of milliseconds until a time yield ends. Throws if
	//!there's no yield, returns 0 if not a timed yield.
	int                 get_yield_ms_left() const;

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

	//!Main loop function. There are no recursive calls to this function.
	return_value        interpret();

	//!Prepares a symbol table for a function call to be called (makes parameters available).
	std::map<std::string, variable> prepare_symbol_table(const function&, const std::vector<variable>&, int);
	//!Pushes a new stack.
	void                push_stack(const function *, int);
	//!Pushes a new stack with the given symbol table.
	void                push_stack(const function *, int, std::map<std::string, variable>&);
	//!Removes the topmost stack.
	void                pop_stack(bool, int);

	//!Functions that this script can use. Functions are implied to be owned by some other thing.
	std::map<std::string, const function *> functions;
	//!Current host pointer.
	host *              current_host{nullptr};
	//!Current output facility pointer.
	out_interface *     out_facility{nullptr};
	//!Stacks. Read the stack description.
	std::vector<stack>  stacks;
	//!Current stack (unsurprisingly, the topmost one).
	stack *             current_stack{nullptr};
	//!Signal reserved for breaking out of a loop.
	bool                break_signal{false},
	//!Signal reserved to indicate that the script yields.
	                    yield_signal{false},
	//!Signal reserved to indicate that execution failed, raised only when an exception is thrown.
	                    failed_signal{false};
	//!Point in time in which a timed yield will release.
	std::chrono::time_point<std::chrono::system_clock> yield_release_time;

};

}
