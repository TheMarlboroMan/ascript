#include "ascript/environment.h"
#include "ascript/tokenizer.h"
#include "ascript/parser.h"
#include "ascript/error.h"

using namespace ascript;

environment::environment(
	host& _host, 
	out_interface& _out
):
	host_instance{_host},
	outfacility{_out}
{

}

void environment::load(
	const std::string& _filename
) {

	tokenizer tk;
	const auto tokens=tk.from_file(_filename);

	parser p;
	auto scripts=p.parse(tokens);
	for(auto& s : scripts) {

		const std::string funcname=s.name;
		
		if(functions.count(funcname)) {

			error_builder::get()<<"a function named '"<<funcname<<"' is already loaded"<<throw_err{0, throw_err::types::user};
		}

		functions.emplace(std::make_pair(s.name, std::move(s)));
	}
}

void environment::load(
	function& _function
) {

	std::string funcname=_function.name;

	if(functions.count(funcname)) {

		error_builder::get()<<"a function named '"<<funcname<<"' is already loaded"<<throw_err{0, throw_err::types::user};
	}

	functions.emplace(std::make_pair(funcname, std::move(_function)));
}

void environment::unload(
	const std::string& _function_name
) {

	if(!functions.count(_function_name)) {

		error_builder::get()<<"function '"<<_function_name<<"' is not loaded"<<throw_err{0, throw_err::types::user};
	}

	functions.erase(_function_name);
}

return_value environment::run(
	const std::string& _function_name, 
	const std::vector<variable>& _arguments
) {

	interpreter interpreter;
	for(const auto& pair: functions) {
		interpreter.add_function(pair.second);
	}

	interpreters.push_back({
		++counter,
		_function_name,
		std::move(interpreter)
	});

	auto result=interpreters.back().interpreter.run(host_instance, outfacility, _function_name, _arguments);
	if(!result.is_yield()) {

		erase(interpreters.back().id);
	}

	return result;
}

return_value environment::run(
	const std::string& _function_name, 
	const std::vector<variable>& _arguments, 
	std::size_t& _id
) {

	interpreter interpreter;
	for(const auto& pair: functions) {
		interpreter.add_function(pair.second);
	}

	interpreters.push_back({
		++counter,
		_function_name,
		std::move(interpreter)
	});

	_id=interpreters.back().id;
	auto result=interpreters.back().interpreter.run(host_instance, outfacility, _function_name, _arguments);
	if(!result.is_yield()) {

		erase(interpreters.back().id);
	}

	return result;
}

return_value environment::resume(
	std::size_t _id
) {

	auto it=std::find_if(
		std::begin(interpreters),
		std::end(interpreters),
		[_id](const pack& _pack) {
			return _pack.id==_id;
		}
	);

	if(it==std::end(interpreters)) {

		error_builder::get()<<"no interpreter with id '"<<_id<<throw_err{0, throw_err::types::user};
	}

	auto& interpreter=it->interpreter;

	if(interpreter.is_failed()) {

		error_builder::get()<<"cannot resume failed interpreter"<<throw_err{0, throw_err::types::user};
	}

	auto result=interpreter.resume();

	if(!result.is_yield()) {

		erase(_id);
	}

	return result;
}

void environment::erase(
	std::size_t _id
) {

	auto it=std::find_if(
		std::begin(interpreters),
		std::end(interpreters),
		[_id](const pack& _pack) {
			return _pack.id==_id;
		}
	);

	interpreters.erase(it);
} 

std::vector<std::size_t> environment::get_yield_ids() const {

	std::vector<std::size_t> result;

	std::transform(
		std::begin(interpreters),
		std::end(interpreters),
		std::back_inserter(result),
		[](const pack& _pack) {
			return _pack.id;
		}
	);

	return result;
}

int environment::get_yield_time(
	std::size_t _id
) const {

	const auto it=std::find_if(
		std::begin(interpreters),
		std::end(interpreters),
		[_id](const pack& _pack) {
			return _pack.id==_id;
		}
	);

	if(it==std::end(interpreters)) {

		error_builder::get()<<"no interpreter with id '"<<_id<<throw_err{0, throw_err::types::user};
	}

	const auto& interpreter=it->interpreter;
	return interpreter.get_yield_ms_left();
}

interpreter& environment::get_interpreter(
	std::size_t _id
) {

	const auto it=std::find_if(
		std::begin(interpreters),
		std::end(interpreters),
		[_id](const pack& _pack) {
			return _pack.id==_id;
		}
	);

	if(it==std::end(interpreters)) {

		error_builder::get()<<"no interpreter with id '"<<_id<<throw_err{0, throw_err::types::user};
	}

	return it->interpreter;
}

void environment::pause() {

	for(auto& _pack : interpreters) {

		if(_pack.interpreter.is_timed_yield() && !_pack.interpreter.is_paused()) {

			_pack.interpreter.pause();
		}
	}
}

void environment::unpause() {

	for(auto& _pack : interpreters) {

		if(_pack.interpreter.is_timed_yield() && _pack.interpreter.is_paused()) {

			_pack.interpreter.unpause();
		}
	}
}
