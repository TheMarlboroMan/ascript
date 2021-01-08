#pragma once

#include "ascript/host.h"
#include "ascript/variable.h"

#include <map>

namespace ascript {

struct run_context {

	enum class signals {none, sigbreak, sigreturn, sigyield, sigjump, sigfail};

	void                            reset();
	void                            resume();

	//TODO: The symbol table should be added too...

	std::map<std::string, variable> symbol_table;
	host *                          host_ptr{nullptr};
	signals                         signal{signals::none};
	int                             aux{0}; //Whatever this may represent, usually the block to jump to.
	variable                        value{false}; //A value produced by some function.

};

}
