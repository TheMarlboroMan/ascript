#include "ascript/run_context.h"

using namespace ascript;

void run_context::reset() {

	aux=0;
	signal=signals::none;
	value={false};
}
