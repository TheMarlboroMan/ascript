#pragma once

#include "ascript/interpreter.h"
#include "ascript/host.h"
#include "ascript/out_interface.h"

#include <vector>
#include <string>

namespace ascript {

//!Pre-packed environment for loading functions and running scripts.
/**
* Supports running several scripts at once (in case they yield). Interpreters
* are removed as soon as they stop executing.
*/
class environment {

	public:

	//!Class constructor.
	                            environment(host&, out_interface&);

	//!Returns the number of current (yielding) interpreters.
	std::size_t                 size() const {return interpreters.size();}

	//!returns true if a function with that name is loaded.
	bool                        has_function(const std::string& _funcname) {return functions.count(_funcname);}

	//!Removes all pending interpreters and resets the id counter. Does not remove functions.
	void                        clear() {interpreters.clear(); counter=0;}

	//!Loads functions from a file.
	void                        load(const std::string&);

	//!Loads a function, moves it so the parameter becomes useless.
	void                        load(function&);

	//!Unloads a function by name.
	void                        unload(const std::string&);

	//!Pauses all pausable interpreters. Will not throw.
	void                        pause();

	//!Un pauses all pausable interpreters. Will not throw.
	void                        unpause();

	//!Runs a function.
	return_value                run(const std::string&, const std::vector<variable>&);

	//!Runs a function, stores the id of the executing interpreter.
	return_value                run(const std::string&, const std::vector<variable>&, std::size_t&);

	//!Resumes the interpreter with the given id.
	return_value                resume(std::size_t);

	//!Returns a vector with the identifiers of yielding interpreters.
	std::vector<std::size_t>    get_yield_ids() const;

	//!Returns a reference to the interpreter with the given id, for finer control.
	interpreter&                get_interpreter(std::size_t);

	//!Returns the remaining yield time in milliseconds for the interpreter with the given id. Throws if no interpreter with that id is yielding.
	int                         get_yield_time(std::size_t) const;

	private: 

	struct pack {

		std::size_t             id;
		std::string             function;
		ascript::interpreter    interpreter;
	};

	void                        erase(std::size_t); 

	using function_table=std::map<std::string, ascript::function>;

	host&                       host_instance;
	out_interface&              outfacility;

	std::size_t                 counter{0};
	function_table              functions;
	std::vector<pack>           interpreters;
};

}


