#include "ascript/interpreter.h"
#include "ascript/error.h"

//TODO:
#include <iostream>

using namespace ascript;

void interpreter::run(
	host& _host,
	const std::string& _funcname, 
	const std::vector<variable>& _arguments
) {

	run(_host, *functions.at(_funcname), _arguments);
}

void interpreter::run(
	host& _host,
	const function& _function, 
	const std::vector<variable>& _arguments
) {
	current_host=&_host;

	auto symbol_table=prepare_symbol_table(_function, _arguments, 0);

	//Start the first stack...
	stacks.push_back(
		{&_function, 0, 0, {current_host}}
	);

	current_stack=&stacks.back();
	current_stack->context.symbol_table=symbol_table;

	break_signal=false;
	yield_signal=false;

	interpret();
}

void interpreter::resume() {

	if(!yield_signal) {

		throw std::runtime_error("called resume on non yielding process");
	}

	yield_signal=false;
	interpret();
}

void interpreter::interpret() {

	while(stacks.size()) {

		const auto& current_block=(current_stack->current_function->blocks[current_stack->block_index]);
//std::cout<<" >> ["<<stacks.size()<<"] "<<current_stack->current_function->name<<":"<<current_stack->block_index<<":"<<current_stack->instruction_index<<" / "<<current_block.instructions.size()<<std::endl;

		if(break_signal) {

			if(current_block.type==block::types::loop) {

				break_signal=false;
			}

			pop_stack(true, current_block.instructions.back()->line_number);
			continue;
		}

		//There's a possibility that the block is over at the beginning of the
		//loop if we close two blocks like endloop; endif; or if we reenter
		//at the end of a block after yielding. That's why we check for the end 
		//here.

		std::size_t total_instructions=current_block.instructions.size();
		bool is_done=total_instructions <= (std::size_t)current_stack->instruction_index;

		if(is_done) {

			if(current_block.type==block::types::loop) {

				current_stack->instruction_index=0;
				continue;
			}

			pop_stack(false, current_block.instructions.back()->line_number);
			continue; //Continue so we exit if the stacks are empty.
		}

		const auto& instruction=current_block.instructions[current_stack->instruction_index];
		current_stack->context.reset();

//std::cout<<*instruction<<std::endl;

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
			case run_context::signals::sigreturnval:
			case run_context::signals::sigreturn:{

				auto exiting_stack=*current_stack;

				while(true) {

					auto exiting_fn_block=current_stack->block_index;

					pop_stack(false, instruction->line_number);
					if(!stacks.size()) {
						return;
					}

					//Did we unwind the full function?
					if(0==exiting_fn_block) {

						//Are we returning to another function? If so, copy
						//the returned value to the current stack...
						if(exiting_stack.context.signal==run_context::signals::sigreturnval) {

							current_stack->context.return_register=exiting_stack.context.return_register;
						}

						break;
					}
				}
			}
			break;
			case run_context::signals::sigexit:

				stacks.clear();
				return;
			break;
			case run_context::signals::sigyield:

				yield_signal=true;
				return;
			break;

			case run_context::signals::sigcall:{

				//Check if the function exists...
				if(!functions.count(current_stack->context.value.str_val)) {

					error_builder::get()<<"undefined function "
						<<current_stack->context.value.str_val
						<<throw_err{instruction->line_number, throw_err::types::user};
				}

				auto symbol_table=prepare_symbol_table(
					*functions.at(current_stack->context.value.str_val), 
					current_stack->context.arguments, 
					instruction->line_number
				);

				push_stack(
					functions.at(current_stack->context.value.str_val),
					0,
					symbol_table
				);
			}
			break;

			case run_context::signals::sigjump:

				push_stack(
					current_stack->current_function,
					current_stack->context.value.int_val
				);
			break;
		}
	}
}

void interpreter::push_stack(
	const function * _function,
	int _stack_index
) {

	auto exiting_table=current_stack->context.symbol_table;

	stacks.push_back(
		{_function, _stack_index, 0, {current_host}}
	);

	current_stack=&stacks.back();
	current_stack->context.symbol_table=exiting_table;
}

void interpreter::push_stack(
	const function * _function, 
	int _stack_index, 
	std::map<std::string, variable>& _symbol_table
) {

	stacks.push_back(
		{_function, _stack_index, 0, {current_host}}
	);

	current_stack=&stacks.back();
	current_stack->context.symbol_table=std::move(_symbol_table);
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
				<<current_stack->current_function->name
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

void interpreter::add_function(
	const function& _func
) {

	if(functions.count(_func.name)) {

		throw std::runtime_error(std::string{"function "}
			+_func.name
			+"already exists"
		);
	}

	functions.insert(std::make_pair(_func.name, &_func));
}

std::map<std::string, variable> interpreter::prepare_symbol_table(
	const function& _function, 
	const std::vector<variable>& _arguments,
	int _line_number
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
			<<throw_err{_line_number, throw_err::types::interpreter};
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
				<<throw_err{0, throw_err::types::interpreter};
		}

		symbol_table.insert(std::make_pair(param.name, _arguments[index++]));
	}

	return symbol_table;
}
