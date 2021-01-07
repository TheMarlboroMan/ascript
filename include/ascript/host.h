#pragma once

namespace ascript {

struct variable;

//!Defines what a host should be able to do...
class host {

	public:

	virtual bool                host_has() const =0;
	virtual bool                host_query() const =0;
	virtual variable            host_get() const =0;
	virtual void                host_add(const std::string&, variable)=0;
	virtual void                host_set(const std::string&, variable)=0;
	virtual void                host_do()=0;
};

}
