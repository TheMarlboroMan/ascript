#include "parser_nodes.h"

using namespace script;

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
	const std::string _val
):
	type{types::string},
	bool_val{false},
	int_val{0},
	str_val{_val}
{

}

std::ostream& script::operator<<(
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
	}
	
	return _stream;
}

std::ostream& script::operator<<(
	std::ostream& _stream, 
	const instruction& _instruction
) {

	_instruction.format_out(_stream);
	return _stream;
}

std::ostream& script::operator<<(
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

std::ostream& script::operator<<(
	std::ostream& _stream, 
	const script& _script
) {

	_stream<<"script '"<<_script.name<<"', contexts:["<<std::endl;
	for(const auto& ctx : _script.contexts) {

		_stream<<ctx<<std::endl;
	}

	return _stream;
}

instruction_out::instruction_out(
	std::vector<variable>& _params
):
	parameters{_params}
{
}

void instruction_out::format_out(
	std::ostream& _stream
) const {

	_stream<<"out[";
	for(const auto& var : parameters) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

instruction_fail::instruction_fail(
	std::vector<variable>& _params
):
	parameters{_params}
{
}

void instruction_fail::format_out(
	std::ostream& _stream
) const {

	_stream<<"fail[";
	for(const auto& var : parameters) {
		_stream<<var<<",";
	}
	_stream<<"]";
}

void instruction_return::format_out(
	std::ostream& _stream
) const {

	_stream<<"return";
}

