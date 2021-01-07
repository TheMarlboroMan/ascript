#pragma once

#include "parser_nodes.h"
#include "run_context.h"

#include <vector>
#include <string>

namespace ascript {

//!This is a stack... the interpreter keeps a list of these 
struct stack {

	int                             block_index,
	                                instruction_index;
	run_context                     context;
};

class interpreter {

	public:

	void                run(const function&, const std::vector<variable>&);
	bool                is_finished() const {return stacks.size()==0;}
	void                resume();

	private:

	void                interpret();

	const function *    current_function{nullptr};
	std::vector<stack>  stacks;
	stack *             current_stack{nullptr};
};

}
