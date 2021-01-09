#include "ascript/run_context.h"

using namespace ascript;

run_context::run_context(
	host* _host
): 
	host_ptr{_host}
{}

void run_context::reset() {

	aux=0;
	signal=signals::none;
	value={false};
}
