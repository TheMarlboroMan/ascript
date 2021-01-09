#pragma once

#include "ascript/host.h"
#include "ascript/variable.h"

#include <map>

namespace ascript {

struct run_context {

	enum class signals {none, sigbreak, sigreturn, sigyield, sigjump, sigfail, sigcall};

	                                run_context(host*);

	//Clears signals and values.
	void                            reset();

	std::map<std::string, variable> symbol_table;
	host *                          host_ptr{nullptr};
	signals                         signal{signals::none};
	variable                        value{false}; //A value produced by some function or the index of a block.
	std::vector<variable>           arguments; //Vector of arguments to be passed to a call from sigcall.

};

}
