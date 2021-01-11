#pragma once

#include "ascript/out_interface.h"
#include "ascript/variable.h"

namespace ascript {

class stdout_out:public out_interface {

	virtual void            out(const variable&);
	virtual void            flush();
};

}
