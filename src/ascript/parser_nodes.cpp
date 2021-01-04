#include "ascript/parser_nodes.h"

using namespace ascript;

variable::variable(
	bool _val
):
	type{types::boolean},
	bool_val{_val},
	int_val{0}
{

}

variable::variable(
	int _val
):
	type{types::integer},
	bool_val{false},
	int_val{_val}
 {

}

variable::variable(
	const std::string& _val
):
	type{types::string},
	bool_val{false},
	int_val{0},
	str_val{_val}
{

}

variable::variable(
	const std::string& _identifier,
	types /*_unused*/
):
	type{types::symbol},
	bool_val{false},
	int_val{0},
	str_val{_identifier}
{

}

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
	int _target_context_index
):
	target_context_index{_target_context_index}
{}

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

void instruction_host_has::format_out(
	std::ostream& _stream
) const {

	_stream<<"host_has[";
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

	_stream<<"jump to and loop "<<target_context_index<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// ostream overloads.

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const variable& _var
) {
	
	switch(_var.type) {

		case variable::types::integer:
			_stream<<"integer:"<<_var.int_val;
			return _stream;
		case variable::types::boolean:
			_stream<<"boolean:"<<_var.bool_val;
			return _stream;
		case variable::types::string:
			_stream<<"string:"<<_var.str_val;
			return _stream;
		case variable::types::symbol:
			_stream<<"symbol:"<<_var.str_val;
			return _stream;
	}
	
	return _stream;
}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const instruction& _instruction
) {

	_instruction.format_out(_stream);
	return _stream;
}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const context& _context
) {

	switch(_context.type) {
		case context::types::linear:
			_stream<<"linear context: instructions["<<std::endl;
		break;
		case context::types::loop:
			_stream<<"loop context: instructions["<<std::endl;
		break;
	}

	for(const auto& ins : _context.instructions) {

		_stream<<(*ins)<<std::endl;
	}

	_stream<<"]"<<std::endl;
	return _stream;
}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const script& _script
) {

	_stream<<"script '"<<_script.name<<"', takes: ["<<std::endl;

	for(const auto& paramname : _script.parameter_names) {

		_stream<<paramname<<",";
	}

	int context_index=0;

	_stream<<"]"<<std::endl<<"contexts:["<<std::endl;
	for(const auto& ctx : _script.contexts) {

		_stream<<"["<<(context_index++)<<"]"<<ctx<<std::endl;
	}

	return _stream;
}

std::ostream& ascript::operator<<(
	std::ostream& _stream, 
	const conditional_path& _branch
) {
	if(nullptr==_branch.function) {
	
		_stream<<"else jump to "<<_branch.target_context_index;
	}
	else {

		_stream<<"if ";
		if(_branch.negated) {
			_stream<<"not ";
		}
		
		_stream<<(*_branch.function)<<" jump to "<<_branch.target_context_index;
	}

	return _stream;
}
