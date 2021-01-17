#include "ascript/interpreter.h"
#include "ascript/error.h"

//TODO:
#include <iostream>

using namespace ascript;

return_value interpreter::run(
	host& _host,
	out_interface& _out_facility,
	const std::string& _funcname, 
	const std::vector<variable>& _arguments
) {

	return run(_host, _out_facility, *functions.at(_funcname), _arguments);
}

return_value interpreter::run(
	host& _host,
	out_interface& _out_facility,
	const function& _function, 
	const std::vector<variable>& _arguments
) {
	current_host=&_host;
	out_facility=&_out_facility;

	auto symbol_table=prepare_symbol_table(_function, _arguments, 0);

	//Start the first stack...
	stacks.push_back(
		{&_function, 0, 0, {current_host, out_facility}}
	);

	current_stack=&stacks.back();
	current_stack->context.symbol_table=symbol_table;

	//Reset all signals and enter the main loop.
	break_signal=false;
	yield_signal=false;
	failed_signal=false;

	return interpret();
}

return_value interpreter::resume() {

	if(!yield_signal) {

		throw std::runtime_error("called resume on non yielding process");
	}

	auto now=std::chrono::system_clock::now();
	if(std::chrono::duration_cast<std::chrono::milliseconds>(yield_release_time-now).count() > 0) {

		return {return_value::types::yield};
	}

	yield_signal=false;
	return interpret();
}

return_value interpreter::interpret() {

	try {

	//This function will be running for as long as there are stacks and 
	//instructions left (unless we exit, return or yield). There are no 
	//recursive calls so it's intuitive to recover from a yield.
	while(stacks.size()) {

		const auto& current_block=(current_stack->current_function->blocks[current_stack->block_index]);
//std::cout<<" >> ["<<stacks.size()<<"] "<<current_stack->current_function->name<<":"<<current_stack->block_index<<":"<<current_stack->instruction_index<<" / "<<current_block.instructions.size()<<std::endl;

		//Evaluate a signal to break from a loop resulting of a previous instruction.
		//pop the stack until we exit a loop block.
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

			//For loops, the symbol table must need to be refreshes. 
			//Popping and pushing the stack will just reset everything we need.
			if(current_block.type==block::types::loop) {

				const auto stack_copy=*current_stack;

				pop_stack(false, current_block.instructions.back()->line_number);
				push_stack(
					stack_copy.current_function,
					stack_copy.block_index
				);
			}
			else {

				//!This would pop the last stack, prompting the end of this method.
				pop_stack(false, current_block.instructions.back()->line_number);
			}

			continue; //Continue so we exit if the stacks are empty.
		}

		const auto& instruction=current_block.instructions[current_stack->instruction_index];
		current_stack->context.reset(); //Reset any data derived from the previous instruction.

//std::cout<<*instruction<<std::endl;

		++current_stack->instruction_index;

		instruction->run(current_stack->context);

		//!Evaluate signals.
		switch(current_stack->context.signal) {

			case run_context::signals::none: break;
			case run_context::signals::sigfail: 

				error_builder::get()<<"fail signal raised: "
					<<current_stack->context.value.str_val
					<<throw_err{instruction->line_number, throw_err::types::user};
			break;

			case run_context::signals::sigbreak:

				break_signal=true;
			break;
			//!Return and return with a value are different signals: a hack.
			case run_context::signals::sigreturnval:
			case run_context::signals::sigreturn:{

				auto exiting_stack=*current_stack;

				//Pop stacks until we pop the last of a function. Remember that
				//a function might have more than one stack (one per block).
				while(true) {

					auto exiting_fn_block=current_stack->block_index;

					pop_stack(false, instruction->line_number);

					//Did we unwind the full function?
					if(0==exiting_fn_block) {

						//Are we returning to another function? If so, copy
						//the returned value to the current stack...
						if(stacks.size()) {
							if(exiting_stack.context.signal==run_context::signals::sigreturnval) {

								current_stack->context.return_register=exiting_stack.context.return_register;
							}

							break;
						}
						//Returning from the main function of this interpreter.
						else {

							return (exiting_stack.context.signal==run_context::signals::sigreturnval)
								? return_value{exiting_stack.context.return_register.value()}
								: return_value{return_value::types::nothing};
						}
					}
				}
			}
			break;
			case run_context::signals::sigexit:

				stacks.clear();
				return {return_value::types::nothing};
			break;
			case run_context::signals::sigyield:

				yield_signal=true;

				//If there was a time expression in the yield, calculate the
				//moment in which this interpreter becomes available again.
				if(current_stack->context.value.int_val) {

					auto now=std::chrono::system_clock::now();
					yield_release_time=now+std::chrono::milliseconds(current_stack->context.value.int_val);
				}

				return {return_value::types::yield};
			break;

			case run_context::signals::sigcall:{

				//Check if the function exists...
				if(!functions.count(current_stack->context.value.str_val)) {

					error_builder::get()<<"undefined function "
						<<current_stack->context.value.str_val
						<<throw_err{instruction->line_number, throw_err::types::interpreter};
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

			//This actually means to enter another block, like entering if 
			//branches or for loops.
			case run_context::signals::sigjump:

				push_stack(
					current_stack->current_function,
					current_stack->context.value.int_val
				);
			break;
		}
	}
	}
	catch(std::exception &e) {

		failed_signal=true;
		throw;
	}

	//Default return type for falling out of a function.
	return {return_value::types::nothing};
}

void interpreter::push_stack(
	const function * _function,
	int _stack_index
) {

	//Copy the current symbol table to make it available on the next stack.
	auto exiting_table=current_stack->context.symbol_table;

	stacks.push_back(
		{_function, _stack_index, 0, {current_host, out_facility}}
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
		{_function, _stack_index, 0, {current_host, out_facility}}
	);

	current_stack=&stacks.back();
	current_stack->context.symbol_table=std::move(_symbol_table);
}

void interpreter::pop_stack(
	bool into_break,
	int _line_number
) {

	//Get the exiting table so we can overwrite the symbols on the parent 
	//table if they existed on the one we just ran.
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

void interpreter::remove_function(
	const std::string& _funcname
) {

	if(!functions.count(_funcname)) {

		throw std::runtime_error(std::string{"function "}
			+_funcname
			+"does not exist"
		);
	}

	functions.erase(_funcname);
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

int interpreter::get_yield_ms_left() const {

	if(!yield_signal) {

		error_builder::get()<<"get_yield_ms_left cannot be called on non-yielding interpreter"
			<<throw_err{0, throw_err::types::interpreter};
	}

	//Is it a timed yield?
	if(false) {

		return 0;
	}

	auto now=std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(yield_release_time-now).count();
}
