#include "ascript/instructions.h"
#include "ascript/run_context.h"
#include "ascript/error.h"

#include <algorithm>
#include <numeric>
#include <iostream>

using namespace ascript;

std::vector<variable> ascript::solve(
	const std::vector<variable>& _variables, 
	const std::map<std::string, variable>& _symbol_table,
	int _line_number
) {

	std::vector<variable> result;
	std::transform(
		std::begin(_variables),
		std::end(_variables),
		std::back_inserter(result),
		[&_symbol_table, _line_number](const variable& _var) {

			if(_var.type!=variable::types::symbol) {

				return _var;
			}

			if(!_symbol_table.count(_var.str_val)) {

				error_builder::get()<<"undefined variable "<<_var.str_val<<throw_err{_line_number, throw_err::types::interpreter};
			}

			return _symbol_table.at(_var.str_val);
		}
	);

	return result;
}

instruction_declaration_static::instruction_declaration_static(
	int _line_number,
	const std::string& _identifier, 
	const variable& _value
):
	instruction{_line_number},
	identifier{_identifier},
	value{_value}
{}

instruction_declaration_dynamic::instruction_declaration_dynamic(
	int _line_number,
	const std::string& _identifier, 
	std::unique_ptr<instruction_function>& _fn
):
	instruction{_line_number},
	identifier(_identifier),
	function{std::move(_fn)}
{}

instruction_assignment_static::instruction_assignment_static(
	int _line_number,
	const std::string& _identifier, 
	const variable& _value
):
	instruction{_line_number},
	identifier{_identifier},
	value{_value}
{}

instruction_assignment_dynamic::instruction_assignment_dynamic(
	int _line_number,
	const std::string& _identifier, 
	std::unique_ptr<instruction_function>& _fn
):
	instruction{_line_number},
	identifier(_identifier),
	function{std::move(_fn)}
{}

instruction_loop::instruction_loop(
	int _line_number,
	int _target_block_index
):
	instruction{_line_number},
	target_block_index{_target_block_index}
{}

////////////////////////////////////////////////////////////////////////////////
//run methods...

void instruction_out::run(
	run_context& _ctx
) const {

	//TODO There should be an out interface.
	//TODO: and it would be really nice if we could use the logger, but we
	//won't be able to :(. I guess, if we really really wanted to we could
	//write an adapter.

	//TODO: in the meantime. std::cout.
	for(const auto& arg : solve(arguments, _ctx.symbol_table, line_number)) {

		//The overloads are, so far, for debugging purposes so...
		switch(arg.type) {
			case variable::types::boolean: std::cout<<(arg.bool_val ? "true" : "false"); break;
			case variable::types::integer: std::cout<<arg.int_val; break;
			case variable::types::string: std::cout<<arg.str_val; break;
			case variable::types::decimal: std::cout<<arg.double_val; break;
			case variable::types::symbol: 
				throw std::runtime_error("should never happen");
		}
	}

	std::cout<<std::endl;
}

void instruction_fail::run(
	run_context& _ctx
) const {

	_ctx.signal=run_context::signals::sigfail;

	std::stringstream ss;
	for(const auto& arg : arguments) {

		ss<<arg;
	}

	//The error message is passed to the context.
	_ctx.value={ss.str()};
}

void instruction_host_set::run(
	run_context& _ctx
) const {

	//Arg count was checked at parse time.

	auto solved=solve(arguments, _ctx.symbol_table, line_number);
	const auto symbol=solved.at(0);
	const auto value=solved.at(1);

	if(symbol.type!=variable::types::string) {

		error_builder::get()<<"host_set expects first argument to be a string"<<throw_err{line_number, throw_err::types::interpreter};
	}

	if(!_ctx.symbol_table.count(symbol.str_val)) {

		error_builder::get()<<"host_set expects first argument to be a existing variable"<<throw_err{line_number, throw_err::types::interpreter};
	}

	if(_ctx.symbol_table.at(symbol.str_val).type != value.type) {

		error_builder::get()<<"host_set type mismatch"<<throw_err{line_number, throw_err::types::interpreter};
	}

	_ctx.symbol_table.at(symbol.str_val)=value;
}

void instruction_host_add::run(
	run_context& _ctx
) const {

	auto solved=solve(arguments, _ctx.symbol_table, line_number);
	const auto symbol=solved.at(0);
	const auto value=solved.at(1);

	if(symbol.type!=variable::types::string) {

		error_builder::get()<<"host_add expects first argument to be a string"<<throw_err{line_number, throw_err::types::interpreter};
	}

	if(_ctx.symbol_table.count(symbol.str_val)) {

		error_builder::get()<<"host_add expects first argument to be a non-existing variable"<<throw_err{line_number, throw_err::types::interpreter};
	}

	_ctx.symbol_table.insert(std::make_pair(symbol.str_val, value));
}

void instruction_host_do::run(
	run_context&
) const {

//TODO:
}

void instruction_is_equal::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_is_equal::evaluate(
	run_context& _ctx
) const {

	auto solved=solve(arguments, _ctx.symbol_table, line_number);
	const auto& first=solved.front();

	return std::all_of(
		std::begin(solved)+1,
		std::end(solved),
		[&first](const variable& _var) {

			return _var==first;
		}
	);
}

void instruction_is_lesser_than::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_is_lesser_than::evaluate(
	run_context& _ctx
) const {

	auto solved=solve(arguments, _ctx.symbol_table, line_number);

	try {

		const auto first=arguments.front();

		return std::all_of(
			std::begin(arguments)+1,
			std::end(arguments),
			[&first](const variable& _var) {
				return first < _var;
			}
		);
	}
	catch(std::runtime_error& e) {

		error_builder::get()<<e.what()<<throw_err{line_number, throw_err::types::interpreter};
	}

	return false; //Shut up compiler.
}

void instruction_is_greater_than::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_is_greater_than::evaluate(
	run_context& _ctx
) const {

		auto solved=solve(arguments, _ctx.symbol_table, line_number);

	try {

		const auto first=arguments.front();

		return std::all_of(
			std::begin(arguments)+1,
			std::end(arguments),
			[&first](const variable& _var) {
				return first > _var;
			}
		);
	}
	catch(std::runtime_error& e) {

		error_builder::get()<<e.what()<<throw_err{line_number, throw_err::types::interpreter};
	}

	return false; //Shut up compiler.
}

void instruction_add::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_add::evaluate(
	run_context& _ctx
) const {

	auto solved=solve(arguments, _ctx.symbol_table, line_number);

	return std::accumulate(
		std::begin(solved)+1,
		std::end(solved),
		solved.front(),
		[](const variable& _a, const variable& _b) {
			return _a+_b;
		}
	);
}

void instruction_substract::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_substract::evaluate(
	run_context& _ctx
) const {

	auto solved=solve(arguments, _ctx.symbol_table, line_number);

	return std::accumulate(
		std::begin(solved)+1,
		std::end(solved),
		solved.front(),
		[](const variable& _a, const variable& _b) {
			return _a-_b;
		}
	);
}

void instruction_host_has::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_host_has::evaluate(
	run_context& _ctx
) const {

	auto solved=solve(arguments, _ctx.symbol_table, line_number);

	//TODO:

	return {false};
}

void instruction_is_int::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_is_int::evaluate(
	run_context& _ctx
) const {

	const auto solved=solve(arguments, _ctx.symbol_table, line_number);
	return std::all_of(
		std::begin(solved),
		std::end(solved),
		[](const variable& _var) {return _var.type==variable::types::integer;}
	);
}

void instruction_is_bool::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_is_bool::evaluate(
	run_context& _ctx
) const {

	const auto solved=solve(arguments, _ctx.symbol_table, line_number);
	return std::all_of(
		std::begin(solved),
		std::end(solved),
		[](const variable& _var) {return _var.type==variable::types::boolean;}
	);
}

void instruction_is_double::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_is_double::evaluate(
	run_context& _ctx
) const {

	const auto solved=solve(arguments, _ctx.symbol_table, line_number);
	return std::all_of(
		std::begin(solved),
		std::end(solved),
		[](const variable& _var) {return _var.type==variable::types::decimal;}
	);
}

void instruction_is_string::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_is_string::evaluate(
	run_context& _ctx
) const {

	const auto solved=solve(arguments, _ctx.symbol_table, line_number);
	return std::all_of(
		std::begin(solved),
		std::end(solved),
		[](const variable& _var) {return _var.type==variable::types::string;}
	);
}

void instruction_host_get::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_host_get::evaluate(
	run_context& _ctx
) const {

	auto solved=solve(arguments, _ctx.symbol_table, line_number);
	//TODO:
	return {false};
}

void instruction_host_query::run(
	run_context& _ctx
) const {

	_ctx.value=evaluate(_ctx);
}

variable instruction_host_query::evaluate(
	run_context& _ctx
) const {

	auto solved=solve(arguments, _ctx.symbol_table, line_number);
	//TODO:
	return {false};
}

void instruction_declaration_static::run(
	run_context& _ctx
) const {

	if(_ctx.symbol_table.count(identifier)) {

		error_builder::get()<<"identifier already exists for declaration"<<throw_err{line_number, throw_err::types::interpreter};
	}

	_ctx.symbol_table.insert(std::make_pair(identifier, value));
}

void instruction_declaration_dynamic::run(
	run_context& _ctx
) const {

	if(_ctx.symbol_table.count(identifier)) {

		error_builder::get()<<"identifier already exists for declaration"<<throw_err{line_number, throw_err::types::interpreter};
	}

	_ctx.symbol_table.insert(
		std::make_pair(
			identifier, 
			function->evaluate(_ctx)
		)
	);
}

void instruction_assignment_static::run(
	run_context& _ctx
) const {

	if(!_ctx.symbol_table.count(identifier)) {

		error_builder::get()<<"identifier does not exist for assignment"<<throw_err{line_number, throw_err::types::interpreter};
	}

	if(value.type != _ctx.symbol_table.at(identifier).type) {

		error_builder::get()<<"type mismatch for assignment"<<throw_err{line_number, throw_err::types::interpreter};
	}

	_ctx.symbol_table.at(identifier)=value;
}

void instruction_assignment_dynamic::run(
	run_context& _ctx
) const {

	if(!_ctx.symbol_table.count(identifier)) {

		error_builder::get()<<"identifier does not exist for assignment"<<throw_err{line_number, throw_err::types::interpreter};
	}

	auto val=function->evaluate(_ctx);

	if(val.type!=_ctx.symbol_table.at(identifier).type) {

		error_builder::get()<<"type mismatch for assignment"<<throw_err{line_number, throw_err::types::interpreter};
	}

	_ctx.symbol_table.at(identifier)=val;
}

void instruction_return::run(
	run_context& _ctx
) const {

	_ctx.signal=run_context::signals::sigreturn;
}

void instruction_yield::run(
	run_context& _ctx
) const {

	_ctx.signal=run_context::signals::sigyield;
}

void instruction_break::run(
	run_context& _ctx
) const {

	_ctx.signal=run_context::signals::sigbreak;
}

void instruction_conditional_branch::run(
	run_context& _ctx
) const {

	for(const auto& branch : branches) {

		//This is the else...
		if(!branch.function) {

			_ctx.signal=run_context::signals::sigjump;
			_ctx.aux=branch.target_block_index;
			return;
		}

		//This is anything that can be evaluated...
		auto val=branch.function->evaluate(_ctx);
		if(val.type!=variable::types::boolean) {

			error_builder::get()<<"evaluation type mismatch, must be boolean values"<<throw_err{branch.line_number, throw_err::types::interpreter};
		}

		//TODO: is this some kind of XOR?
		if(!branch.negated && val.bool_val) {

			_ctx.signal=run_context::signals::sigjump;
			_ctx.aux=branch.target_block_index;
			return;
		}
		else if(branch.negated && !val.bool_val) {

			_ctx.signal=run_context::signals::sigjump;
			_ctx.aux=branch.target_block_index;
			return;
		}
	}
}

void instruction_loop::run(
	run_context& _ctx
) const {

	_ctx.signal=run_context::signals::sigjump;
	_ctx.aux=target_block_index;
}

////////////////////////////////////////////////////////////////////////////////
//Format out methods...

void instruction_out::format_out(
	std::ostream& _stream
) const {

	_stream<<"out[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_fail::format_out(
	std::ostream& _stream
) const {

	_stream<<"fail[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_host_set::format_out(
	std::ostream& _stream
) const {

	_stream<<"host_set[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_host_add::format_out(
	std::ostream& _stream
) const {

	_stream<<"host_add[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_host_do::format_out(
	std::ostream& _stream
) const {

	_stream<<"host_do[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_is_equal::format_out(
	std::ostream& _stream
) const {

	_stream<<"is_equal[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_is_greater_than::format_out(
	std::ostream& _stream
) const {

	_stream<<"is_greater_than[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_is_lesser_than::format_out(
	std::ostream& _stream
) const {

	_stream<<"is_lesser_than[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_add::format_out(
	std::ostream& _stream
) const {

	_stream<<"add[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_substract::format_out(
	std::ostream& _stream
) const {

	_stream<<"substract[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_is_int::format_out(
	std::ostream& _stream
) const {

	_stream<<"is_int[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_is_bool::format_out(
	std::ostream& _stream
) const {

	_stream<<"is_bool[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_is_double::format_out(
	std::ostream& _stream
) const {

	_stream<<"is_double[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_is_string::format_out(
	std::ostream& _stream
) const {

	_stream<<"is_string[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_host_has::format_out(
	std::ostream& _stream
) const {

	_stream<<
"host_has[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_host_get::format_out(
	std::ostream& _stream
) const {

	_stream<<"host_get[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_host_query::format_out(
	std::ostream& _stream
) const {

	_stream<<"host_query[";
	for(const auto& var : arguments) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_return::format_out(
	std::ostream& _stream
) const {

	_stream<<"return";
}

void instruction_yield::format_out(
	std::ostream& _stream
) const {

	_stream<<"yield";
}

void instruction_break::format_out(
	std::ostream& _stream
) const {

	_stream<<"break";
}

void instruction_declaration_static::format_out(
	std::ostream& _stream
) const {

	_stream<<"variable '"<<identifier<<"' as "<<value;
}

void instruction_declaration_dynamic::format_out(
	std::ostream& _stream
) const {

	_stream<<"variable '"<<identifier<<"' as call to "<<(*function);
}

void instruction_assignment_static::format_out(
	std::ostream& _stream
) const {

	_stream<<"set variable '"<<identifier<<"' to "<<value;
}

void instruction_assignment_dynamic::format_out(
	std::ostream& _stream
) const {

	_stream<<"set variable '"<<identifier<<"' to call to "<<(*function);
}

void instruction_conditional_branch::format_out(
	std::ostream& _stream
) const {

	for(const auto& _branch : branches) {

		_stream<<_branch<<std::endl;
	}
}

void instruction_loop::format_out(
	std::ostream& _stream
) const {

	_stream<<"jump to and loop "<<target_block_index<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// ostream overloads.

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const instruction& _instruction
) {

	_instruction.format_out(_stream);
	return _stream;
}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const block& _block
) {

	switch(_block.type) {
		case block::types::linear:
			_stream<<"linear block: instructions["<<std::endl;
		break;
		case block::types::loop:
			_stream<<"loop block: instructions["<<std::endl;
		break;
	}

	for(const auto& ins : _block.instructions) {

		_stream<<(*ins)<<std::endl;
	}

	_stream<<"]"<<std::endl;
	return _stream;
}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const parameter& _parameter
) {

	_stream<<_parameter.name<<" of ";
	switch(_parameter.type) {
		case parameter::types::integer: _stream<<"integer"; break;
		case parameter::types::decimal: _stream<<"decimal"; break;
		case parameter::types::boolean: _stream<<"boolean"; break;
		case parameter::types::string: _stream<<"string"; break;
		case parameter::types::any: _stream<<"any"; break;
	}

	_stream<<" type"<<std::endl;
	return _stream;
}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const function& _function
) {

	_stream<<"function '"<<_function.name<<"', takes: ["<<std::endl;

	for(const auto& param : _function.parameters) {

		_stream<<param<<",";
	}

	int block_index=0;

	_stream<<"]"<<std::endl<<"blocks:["<<std::endl;
	for(const auto& ctx : _function.blocks) {

		_stream<<"["<<(block_index++)<<"]"<<ctx<<std::endl;
	}

	return _stream;
}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const conditional_path& _branch
) {
	if(nullptr==_branch.function) {
	
		_stream<<"else jump to "<<_branch.target_block_index;
	}
	else {

		_stream<<"if ";
		if(_branch.negated) {
			_stream<<"not ";
		}
		
		_stream<<(*_branch.function)<<" jump to "<<_branch.target_block_index;
	}

	return _stream;
}