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

	//Start the first stack...
	stacks.push_back(
		{&_function, 0, 0, {current_host}}
	);

	current_stack=&stacks.back();
	current_stack->context.symbol_table=symbol_table;
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

	bool breaking=false;

	//This methods runs a single stack.
	while(!breaking && stacks.size()) {

		current_stack->context.reset();
		const auto& current_block=(current_stack->current_function->blocks[current_stack->block_index]);
		const auto& instruction=current_block.instructions[current_stack->instruction_index];

//std::cout<<" >> "<<current_stack->current_function->name<<":"<<current_stack->block_index<<":"<<current_stack->instruction_index<<" -> "<<*instruction<<std::endl;

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

				yield_signal=true;
				return; 
			break;
			case run_context::signals::sigjump:
				push_stack(
					current_stack->current_function,
					current_stack->context.aux
				);
			break;
		}

		if(yield_signal) {

			return;
		}

		if(break_signal) {

			if(current_block.type==block::types::loop) {

				break_signal=false;
			}

			pop_stack(true, instruction->line_number);
			return;
		}

		bool is_done=current_block.instructions.size()==(std::size_t)current_stack->instruction_index;

		if(is_done && current_block.type==block::types::loop) {

			current_stack->instruction_index=0;
			continue;
		}
		else if(is_done) {

			pop_stack(false, instruction->line_number);
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
