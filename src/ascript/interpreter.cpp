#include "ascript/interpreter.h"
#include "ascript/error.h"

//TODO:
#include <iostream>

using namespace ascript;

void interpreter::run(
	const function& _function, 
	const std::vector<variable>& _arguments
) {

	std::map<std::string, variable> symbol_table;

	if(_arguments.size() != _function.parameters.size()) {

		error_builder::get()
			<<"wrong parameter count for "
			<<_function.name
			<<", expected "
			<<_function.parameters.size()
			<<", got "
			<<_arguments.size()
			<<throw_err{0, throw_err::types::parser};
	}

	std::size_t index=0;
	for(const auto& param : _function.parameters) {

		const auto& arg=_arguments[index];
		bool failed=false;

		switch(param.type) {
			case parameter::types::integer:
				if(arg.type!=variable::types::integer) {
					failed=true;
				}
			break;
			case parameter::types::decimal:
				if(arg.type!=variable::types::decimal) {
					failed=true;
				}
			break;
			case parameter::types::boolean:

				if(arg.type!=variable::types::boolean) {
					failed=true;
				}
			break;
			case parameter::types::string:
				if(arg.type!=variable::types::string) {
					failed=true;
				}
			break;
			case parameter::types::any: break;
		}

		if(failed) {
			error_builder::get()<<"type mismatch argument for parameter "
				<<param.name
				<<" in function "
				<<_function.name
				<<throw_err{0, throw_err::types::parser};
		}

		symbol_table.insert(std::make_pair(param.name, _arguments[index++]));
	}

	current_function=&_function;

	//Start the first stack...
	stacks.push_back(
		{0, 0, run_context{}}
	);

	current_stack=&stacks.back();
	current_stack->context.symbol_table=symbol_table;
	interpret();
}

void interpreter::resume() {

	//TODO: This should go ahead if we yielded.
	//I guess we run... interpret?
}

void interpreter::interpret() {

	bool breaking=false,
		 is_done=false;

	//This methods runs a single stack.
	while(!breaking && !is_done) {

		current_stack->context.reset();
		const auto& current_block=(current_function->blocks[current_stack->block_index]);
		const auto& instruction=current_block.instructions[current_stack->instruction_index];

//std::cout<<" >> "<<current_stack->block_index<<":"<<current_stack->instruction_index<<" -> "<<*instruction<<std::endl;

		++current_stack->instruction_index;

		instruction->run(current_stack->context);


		switch(current_stack->context.signal) {

			case run_context::signals::none: 
			break;
			case run_context::signals::sigfail: 

				error_builder::get()<<"fail signal raised: "
					<<current_stack->context.value.str_val
					<<throw_err{instruction->line_number, throw_err::types::user};
			break;

			case run_context::signals::sigbreak:
				break_signal=true;
			break;
			case run_context::signals::sigreturn:

				stacks.clear();
				return;
			break;
			case run_context::signals::sigyield:

				//TODO: Hmmm... this will not yield in N depth... I guess there
				//should be a generic "yield" signal that makes all shit 
				//return, just as break.
				return; 
			break;
			case run_context::signals::sigjump:
				push_stack(current_stack->context.aux);
				interpret();
			break;
		}

		if(break_signal) {

			if(current_block.type==block::types::loop) {

				break_signal=false;
			}

			pop_stack(true, instruction->line_number);
			return;
		}

		is_done=current_block.instructions.size()==(std::size_t)current_stack->instruction_index;

		if(is_done && current_block.type==block::types::loop) {

			current_stack->instruction_index=0;
			is_done=false;
			continue;
		}
	}

	pop_stack(false, 0);
}

void interpreter::push_stack(
	int _stack_index
) {

	auto exiting_table=current_stack->context.symbol_table;

	stacks.push_back(
		{_stack_index, 0, run_context{}}
	);

	current_stack=&stacks.back();
	current_stack->context.symbol_table=exiting_table;
}

void interpreter::pop_stack(
	bool into_break,
	int _line_number
) {

	auto exiting_table=current_stack->context.symbol_table;

	stacks.pop_back();

	if(!stacks.size()) {

		if(into_break) {

			error_builder::get()
				<<"unexpected break outside loop in "
				<<current_function->name
				<<throw_err{_line_number, throw_err::types::interpreter};
		}

		current_stack=nullptr;
		return;
	}

	current_stack=&stacks.back();
	for(auto& symbol : current_stack->context.symbol_table) {
	
		if(exiting_table.count(symbol.first)) {

			symbol.second=exiting_table.at(symbol.first);
		}
	}
}
