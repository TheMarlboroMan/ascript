#include "ascript/interpreter.h"

//TODO:
#include <iostream>

using namespace ascript;

void interpreter::run(
	const function& _function, 
	const std::vector<variable>& _parameters
) {

	current_function=&_function;

	//Start the first stack...
	stacks.push_back(
		{0, 0, run_context{}}
	);

	current_stack=&stacks.back();
	interpret();
}

void interpreter::resume() {

	//TODO: This should go ahead if we yielded.
}

void interpreter::interpret() {

	const auto& current_block=(current_function->blocks[current_stack->block_index]);

	while(true) {

		current_stack->context.reset();
		const auto& instruction=current_block.instructions[current_stack->instruction_index];

std::cout<<*instruction<<std::endl;

		instruction->run(current_stack->context);

		switch(current_stack->context.signal) {

			case run_context::signals::none: break;
			case run_context::signals::sigfail: 
				//TODO: 
			break;
			case run_context::signals::sigbreak:

				//TODO: pull stacks until we can pull the first that is a loop
				//type.
			break;
			case run_context::signals::sigreturn:

				stacks.clear();
				return;
			break;
			case run_context::signals::sigyield:

				//TODO: Hmmm... this will not yield in N depth... I guess there
				//should be a generic "yield" signal that makes all shit 
				//return and is reset when 
				return; 
			break;
			case run_context::signals::sigjump:

				//TODO: Copy symbol table...
				stacks.push_back(
					{current_stack->context.aux, 0, run_context{}}
				);

				current_stack=&stacks.back();
				interpret();
				//TODO: The stack sould be pulled.
			break;
		}

		++current_stack->instruction_index;
	
		bool is_done=current_block.instructions.size()==(std::size_t)current_stack->instruction_index;

		if(is_done) {

			if(current_block.type==block::types::loop) {

				current_stack->instruction_index=0;
				continue;
			}

			//TODO: do whatever it takes to the symbol table. Unless we are
			//at the last	
			stacks.pop_back();
			if(!stacks.size()) {

				break;
			}
		}
	}
}
