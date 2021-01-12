#pragma once

#include <string>
#include <vector>

namespace ascript {

struct variable;


//!This interface defines what a host should be able to do.
/**
* A host is an entity with which ascript can interact. Scripts can read and
* write from a symbol table (to be implemented by the host) and can ask it to
* perform implementation-defined complex operations (query to ask about a value
* that the host must compute, do to ask the host to perform an action that 
* returns no results.
* This interface defines what a host should be able to do.
**/ 
class host {

	public:

	//!Must return true if the symbol is defined in the host.
	virtual bool                host_has(const std::string) const =0;

	//!Must deletel the symbol is defined in the host, throw if not.
	virtual void                host_delete(const std::string) =0;

	//!Must return the symbol defined in the host or throw a host_error if the
	//!symbol does not exist..
	virtual variable            host_get(const std::string) const =0;

	//!Must send a query to the host about its state that can be solved to a variable.
	virtual variable            host_query(const std::vector<variable>&) const =0;

	//!Must add the given value to the symbol table or throw host_error if the
	//!symbol is already defined.
	virtual void                host_add(const std::string&, variable)=0;

	//!Must set the given value to the symbol table or throw host_error if the
	//!symbol is not defined. The syntax and semantics or the message are 
	//!implementation-defined.
	virtual void                host_set(const std::string&, variable)=0;

	//!Must send a procedure message to the host. The syntax and semantics of
	//!the message are implementation-defined.
	virtual void                host_do(const std::vector<variable>&)=0;
};

}
