#pragma once

#include "ascript/out_interface.h"
#include "ascript/variable.h"

namespace ascript {

//!convenience implementation of the out_interface that uses std::out.
class stdout_out:public out_interface {

	//!out implementation.
	virtual void            out(const variable&);
	//!flush implementation.
	virtual void            flush();
};

}
