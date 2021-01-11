#pragma once

namespace ascript {

struct variable;

//!interface for out facilities, decides how variables are printed out when 
//!using the "out" instruction.

struct out_interface {

	virtual void            out(const variable&)=0;
	virtual void            flush()=0;
};

}
