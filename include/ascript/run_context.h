#pragma once

#include "ascript/host.h"
#include "ascript/variable.h"
#include "ascript/out_interface.h"

#include <map>
#include <optional>

namespace ascript {

struct run_context {

	enum class signals {none, sigbreak, sigreturn, sigreturnval, sigyield, sigjump, sigfail, sigcall, sigexit};

	                                run_context(host*, out_interface*);

	//Clears signals and values.
	void                            reset();

	std::map<std::string, variable> symbol_table;
	host *                          host_ptr{nullptr};
	out_interface *                 out_facility{nullptr};
	signals                         signal{signals::none};
	variable                        value{false}; //A value produced by some function or the index of a block.
	std::optional<variable>         return_register; //!The register where returned values are stored.
	std::vector<variable>           arguments; //Vector of arguments to be passed to a call from sigcall.

};

}
