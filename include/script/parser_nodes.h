#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ostream>

namespace script {

struct variable {

	                        variable(bool);
	                        variable(int);
	                        variable(const std::string);
	enum class              types{boolean, integer, string} type{types::integer};
	bool                    bool_val{false};
	int                     int_val{0};
	std::string             str_val;
};

struct instruction {

	virtual                 ~instruction(){}
	virtual void            format_out(std::ostream&) const=0;
};

struct instruction_out:instruction {

	                        instruction_out(std::vector<variable>&);
	std::vector<variable>   parameters;

	void                    format_out(std::ostream&) const;
};

struct instruction_fail:instruction {

	                        instruction_fail(std::vector<variable>&);
	std::vector<variable>   parameters;

	void                    format_out(std::ostream&) const;
};

struct instruction_return:instruction {

	void                    format_out(std::ostream&) const;
};

struct context {

	enum class types {linear, loop}             type;
	std::vector<std::unique_ptr<instruction>>  instructions;
};

struct script {

	std::string                                 name;
	std::vector<context>                        contexts;
};

std::ostream& operator<<(std::ostream&, const variable&);
std::ostream& operator<<(std::ostream&, const instruction&);
std::ostream& operator<<(std::ostream&, const context&);
std::ostream& operator<<(std::ostream&, const script&);



}
