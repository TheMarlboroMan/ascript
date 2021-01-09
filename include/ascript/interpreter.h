#pragma once

#include "instructions.h"
#include "run_context.h"

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
	void                run(host&, const function&, const std::vector<variable>&);

	//!Runs a named function that should have been added before.
	void                run(host&, const std::string&, const std::vector<variable>&);

	//!Returns true if the function has finished executing (only makes sense if
	//!the function yielded.
	bool                is_finished() const {return stacks.size()==0;}

	//!Resumes a yielding execution. Throws if the execution is not stopped.
	void                resume();

	//!Adds a function. The function object MUST outlive the interpreter.
	void                add_function(const function&);

	private:

	void                interpret();
	void                push_stack(const function *, int);
	void                pop_stack(bool, int);

	//!Functions are implied to be owned by some other thing.
	std::map<std::string, const function *> functions;
	host *              current_host{nullptr};
	std::vector<stack>  stacks;
	stack *             current_stack{nullptr};
	bool                break_signal{false},
	                    yield_signal{false};
};

}
