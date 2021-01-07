#include "ascript/parser_nodes.h"
#include "ascript/run_context.h"

//TODO: Remove.
#include <iostream>

using namespace ascript;


instruction_declaration_static::instruction_declaration_static(
	const std::string& _identifier, 
	const variable& _value
):
	identifier{_identifier},
	value{_value}
{}

instruction_declaration_dynamic::instruction_declaration_dynamic(
	const std::string& _identifier, 
	std::unique_ptr<instruction_function>& _fn
):
	identifier(_identifier),
	function{std::move(_fn)}
{}

instruction_loop::instruction_loop(
	int _target_block_index
):
	target_block_index{_target_block_index}
{}

////////////////////////////////////////////////////////////////////////////////
//run methods...

void instruction_out::run(
	run_context&
) const {

	//TODO There should be an out interface.
	//TODO: and it would be really nice if we could use the logger, but we
	//won't be able to :(. I guess, if we really really wanted to we could
	//write an adapter.

	//TODO: in the meantime. std::cout.
	for(const auto& arg : arguments) {

		std::cout<<arg;
	}

	std::cout<<std::endl;
}

void instruction_fail::run(
	run_context& _ctx
) const {

	_ctx.signal=run_context::signals::sigfail;
	//TODO: output the message???? Put is somewhere special in the context???
	//I say that one.
}

void instruction_host_set::run(
	run_context& _ctx
) const {

	//TODO: Check syntax on the parser: two parameters.

	//TODO: Did we check if the first value is a str????
	//TODO: 

	const auto symbol=arguments.at(0);
	const auto value=arguments.at(1);

	if(!_ctx.symbol_table.count(symbol.str_val)) {

		//TODO: Should throw...
	}

	//TODO: Check the type
	if(_ctx.symbol_table.at(symbol.str_val).type != value.type) {

		//TODO: throw type mismatch.
	}

	_ctx.symbol_table.at(symbol.str_val)=value;
}

void instruction_host_add::run(
	run_context&
) const {

	//TODO: Check syntax on the parser: two parameters.

	//TODO: Did we check if the first value is a str????

	const auto symbol=arguments.at(0);
	const auto value=arguments.at(1);

	if(_ctx.symbol_table.count(symbol.str_val)) {

		//TODO: Should throw...
	}

	_ctx.symbol_table.insert(std::make_pair(symbol.str_val, value));
}

void instruction_host_do::run(
	run_context&
) const {
//TODO:

}

void instruction_is_equal::run(
	run_context&
) const {

	//TODO: Some nice iterators, sec eq first, third eq first...

	//TODO:

}

void instruction_is_lesser_than::run(
	run_context&
) const {

//TODO: Fail if not numeric.
//TODO: Do.

}

void instruction_is_greater_than::run(
	run_context&
) const {

//TODO: Fail if not numeric.
//TODO: Do.

}

void instruction_host_has::run(
	run_context&
) const {

	
//TODO:

}

void instruction_is_int::run(
	run_context& _cxt
) const {

	_ctx.aux=0;

	for(const auto& arg : arguments) {

		if(arg.type!=variable::types::integer) {
			return;
		}
	} 

	_ctx.aux=1;
}

void instruction_is_bool::run(
	run_context&
) const {

	_ctx.aux=0;

	for(const auto& arg : arguments) {

		if(arg.type!=variable::types::boolean) {
			return;
		}
	} 

	_ctx.aux=1;
}

void instruction_is_double::run(
	run_context&
) const {

	_ctx.aux=0;

	for(const auto& arg : arguments) {

		if(arg.type!=variable::types::decimal) {
			return;
		}
	} 

	_ctx.aux=1;
}

void instruction_is_string::run(
	run_context&
) const {

	_ctx.aux=0;

	for(const auto& arg : arguments) {

		if(arg.type!=variable::types::string) {
			return;
		}
	} 

	_ctx.aux=1;
}

void instruction_host_get::run(
	run_context&
) const {

//TODO:

}

void instruction_host_query::run(
	run_context&
) const {

//TODO:

}

void instruction_declaration_static::run(
	run_context& _ctx
) const {

	//TODO: Check if it's not already set!!!
	_ctx.symbol_table.insert(std::make_pair(identifier, value));
}

void instruction_declaration_dynamic::run(
	run_context&
) const {

	//TODO: Produce the value, then run the same shit as above.
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
	run_context&
) const {

	//TODO...
	//TODO...
	//TODO...
	//TODO...
	//TODO...
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
	const function& _function
) {

	_stream<<"function '"<<_function.name<<"', takes: ["<<std::endl;

	for(const auto& paramname : _function.parameter_names) {

		_stream<<paramname<<",";
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
