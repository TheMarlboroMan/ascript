#include "ascript/run_context.h"

using namespace ascript;

run_context::run_context(
	host* _host,
	out_interface * _out
): 
	host_ptr{_host},
	out_facility{_out}
{}

void run_context::reset() {

	signal=signals::none;
	value={false};
	arguments.clear();
}
