#pragma once

namespace ascript {

struct variable;

//!interface for out facilities, decides how variables are printed out when 
//!using the "out" instruction.
struct out_interface {

	//!must print out a variable.
	virtual void            out(const variable&)=0;
	//!must commit (if needed) the contents to whatever underlying system there is (file, log, output stream...).
	virtual void            flush()=0;
};

}
