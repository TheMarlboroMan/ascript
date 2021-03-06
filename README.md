#ascript

Short for annoying script. An annoying script language...

## fun stuff

- To allow for yielding there's no recursive interpreter... this opens the door for all sorts of unpleasantries.
- When calling interpreter::run with a script is imperative that the script object is not gone out of scope. This could be fixed, but it wouldn't be annoying.
- Scripts can't be copied. That's right. There's an unique_ptr down there, could be fixed but again, would not be annoying.

##BUGS:

- out ["something", var, ", ", othervar]; causes the parser to explode, it cannot fathom a comma following a quote.

##TODO:

- Check for memory leaks.
- Cleanup arguments / parameters name, they should not be interchangeable in a readable codebase.
- Check TODOs.
- As an interesting exercise, try separating "variable" into "type", which would have subtypes for int, bool and such.
	- An alternative is to leave variable to exist, but have separate storage classes for each type and have the class point to them when retrieving data.
- Study homogeneous arrays, with a syntax like let crap be {1,2,3}; and then array_size[crap], array_get[crap, 1], array_set[crap, 2, 33], and so on.
- Study structures (symbol tables on themselves, custom types). they would be... interesting, let crap be packed type [member:value, member:value, member:value];

##manual

ascript is an script language implemented in C++ as a library. Its purpose is to act as a simple script language for other applications, mainly games, which is the main purpose why I wrote it.

ascript stands for "annoying script". There are small annoyances but it's mainly for fun.

ascript needs to be run from a C++ program. There are a few example programs bundled.

Of course, ascript is for my own amusement. There are better, more stable and time-tested alternatives out there.

###the input files

Input files are plain text files that must contain ascript functions. There's no official extension, I use ".ann".

###the host

The host is not exactly the program from within ascript runs, but a facility inside such program that is designed to interact with it. On its own, ascript supports only very simple arithmetic and logic but through a host interface it can access host calculations and instruct the host to perform any implementation-defined action.

The advantage against implementing the same functionality in the program itself is that behaviour implemented in ascript is evaluated at run-time and needs not to be recompiled. In other words, it should allow me to add non-hardcoded functionality to my games.

###the output interface

The output interface is whatever exists in the calling environment that allows ascript to output stuff. It must implement "out_interface".

For convenience, a stdout_out class is provided with ascript.

###general syntax notes

ascript supports a very simple syntax with very few concessions... every single instruction must end with a ; (be it a function declaration or a more complex equality test), in layman's terms, there should be a ; at the end of every line.

ascript is case sensitive and all of its keywords and functions are lowercase.

Valid language tokens are separated by a space, function parameters are separated by commas and put between brackets. You must separate the brackets from the function name by whitespace, but may put the parameters next to commas and brackets.

Comments are lines starting with #. I don't think a # symbol at the end of a line will be anything but a syntax error.

Line breaks and tabs mean nothing.

ascript does not adhere to BNF form, that is, it does not support token resolution a la "callfunction [1, 2, otherfunction [3, 4]]". The inner function must be resolved separatedly.

In the same way, boolean logic is performed through functions. It follows that a valid statement is "if func [1, 2]". However, "let val be func [1,2]; if val" is not a valid form, even if the redundant "let val be func [1,2]; if is_same [val, true]" is valid. There is a "not" keyword that allows for "if not is_equal [true, false]" but there are not "and", or "or" keywords (again, no BNF). Given that is_* functions accept multiple arguments, simple cases can be covered ("if is_equal[true, val1, val2, val3, val4]") but compound statements will be neccesary for others.

Built-in functions cannot be found outside assignments and boolean statements. Built-in procedures (functions that return no values), however can only be found outside these. User-defined functions can appear in both places.

###types

ascript is typed. Declaring a variable will automatically set its type to integer, boolean, double or string. Its type must be the same through all its lifetime. Resetting the variable to another type will cause an error.

Function parameters are also typed, even if the keyword "any" allows an argument to be of any type (basically an "I really don't care about anything in life" case). Oddly enough, a function can return different types.

There are functions to identify each type (is_int, is_string...).

####a note on type mismatches

ascript is annoyingly typed. A call to "is_lesser_than [integer, double]" will cause a type mismatch, as "is_equal [true, 1, 1.0]" will. 

###declaring functions

Functions are the entrypoint to ascript from the calling environment. They are declared on the following form:

beginfunction *functionname* [*paramname* as *type*, *paramname* as *type*];

	statements;

endfunction;

The parameter list is optional, it does not need to appear if a function takes no parameters. A function can accept any number of parameters, whose names should not be repeated. The types can be "int", "bool", "string", "double" and "any" (just in case you are feeling funky).

All parameters are copies, there are no reference parameters.

Functions can also call other functions. Recursion is theoretically supported.

###variables

Variables can be declared to hold statically defined values in the following form:

let *variablename* be *variablevalue*;

The variable value determines its type: integers are simple numbers, strings use double quotes, booleans use the literals "true" and "false" and doubles expect to have a dot somewhere for the decimal part. Anything else is supposed to be the name of another variable, which copies the value.

There are no reference types.

Variables can be declared to hold the return value of a function:

let *variablename* be built_in_fn [param, param];
let *variablename* be custom_function [param, param];

Variables can be reset with the form:

set *variablename* to *variablevalue*;
set *variablename* to built_in_fn [param, param];
set *variablename* to custom_function [param, param];

Variables do not share their space with host symbol tables, but do share it with function parameters.

####variable scope

A variable will exist for as long as its block does. That is, everything declared after beginfunction, if, elseif, else or loop will exist until its corresponding closing statement.

###returning values from functions

A function can exit by using the "return statement". If a function must return a (single) value, if must do it like this:

return [value];

##running ascript from C++:

First, load up a tokenizer and instruct it to decompose a program:
	ascript::tokenizer tk;
	const auto tokens=tk.from_file(_argv[1]);

Next, load a parser and generate functions from it:
	ascript::parser p;
	const auto functions=p.parse(tokens);

These functions must exist for as long as any interpreter wants to make use of them!!! If they go out of scope first, it's boom time.

Implement the "host" and "out_interface" interfaces. Create instances of them, plus an interpreter.
	script_host sh;
	ascript::interpreter i;
	ascript::stdout_out outfacility;

Load the functions into the interpreter. Yes, this is a neccesary step.
	for(const auto& f : functions) {
		i.add_function(f);
	}

Finally run whatever function you want. The two last parameters are the function name and a vector of parameters (of "ascript::variable" type).

	auto result=i.run(sh, outfacility, funcname, {});

###accessing return values from the calling environment

Once the interpreter ends, it returns a "return_value" type which might hold:

- A real value.
- A "yield" value, meaning that the function has yielded and no return value is yet available.
- A "nothing" value, meaning that the function returns nothing.

	if(result) {

		//result.get() will return a "variable" type.
	}
	else if(result.is_yield()) {

		//the function is not done and can be resumed with interpreter::resume()
	}
	else if(result.is_nothing()) {

		//no return value.
	}

If a yielded value is returned the interpreter can be queried to whether or not it is blocked by a timed yield and how many milliseconds are left with get_yield_ms_left A return value of 0 (most likely) means a non timed yield. Positive values are for functions whose yield timer has not expired, and negative values are for those who expired.

###handling errors on the calling enviroment:

All failures will throw an exception whose base type is "ascript_error".

###control flow

The following control flow mechanisms are available:

####conditionals

All conditionals follow of the following form:

if (not) *evaluation_function*;
	statements;
(elseif *evaluation_function*);
	statements;
(elseif *evaluation_function*);
	statements;
(elseif *evaluation_function*);
	statements;
(else)
	statements;
endif;

It follows that this the shortest valid if statement:

if fn [];
	statements;
endif;

Evaluation functions can either be built-ins or user-defined functions.

if is_equal [true, true];
	statements;
endif;

if funcname [true, true];
	statements;
endif;

A user-defined function that returns no value will cause a runtime error if used within an if statement:

beginfunction test;
	out ["fail"];
endfunction;

if test[];
	out ["will never get here"];
endif;
out ["nor here"];

####loops

All loops are of the following form:

loop;
	statements;
endloop;

Loops can be exited by:

	- return
	- break
	- fail

ascript runs in a single thread: an infinite loop will freeze the calling environment unless "yield" is used!.

There's no for or while loops. All loops expect to have some if clause that breaks (or yield and loop forever).

####yielding functions

The keyword "yield" causes the execution of a function to be halted with no return value and returns control to the point where "run" was called. The interpreter can resume the function with a call to "resume" and the method "is_finished" can be used to query its state.

The purpose of non-timed "yield" is to be able to run infinite loops without freezing the calling environment. It could be argued that the same functionality can be obtained with repeated "run" calls, but I wanted "yield" to exist anyhow.

Yield can also be used to halt execution for at least a number of milliseconds using:

	yield for *variable_solving_to_milliseconds*;
	yield for 1000;

This form prevents the interpreter from resuming until the time has elapsed. Execution does not resume automatically: the interpreter must be queried about the time left (if any) before calling "resume". Calling "resume" on a time-yielding interpreter will return the yield value.

###calling built-in and user-defined functions

Calling other functions is done using the function identifier and brackets for the parameter lists:

functioname [param, param];

If a function needs no parameters, ascript still needs the brackets for disambiguation purposes.

functionname [];

Should the function return a value, it can be captured with:

let *variablename* be functioname [param, param];
set *variablename* to functioname [param, param];

Function names cannot be captured as strings, thus this:

let a be "myfunc";
let b be a [true];

Will fail unless there's a "a" function defined.

###built in functions

These are the built-in functions. All these functions can appear at variable assignment or declaration. Those returning boolean values can also appear in branch statements.

####is_equal

Returns true if all of the parameters passed to it are of the same type and hold the same value.

let a be 1;
let b be 1;
if is_equal [1, a, b];
	out ["ok"];
endif;

####is_greater_than

Returns true if the first parameter is of numeric type and is greater than the rest, which must be of the same numeric type:

let a be 1;
let b be 2;
if is_greater [100, a, b];
	out ["ok"];
endif;

####is_lesser_than

The opposite of "is_greater_than".

####add

Returns a numeric value that results of adding all of its parameters, which must be of the same numeric type.

let a be 1;
let b be 2;
let c be add [a, b, 3];
if is_equal [c, 6];
	out ["ok"];
endif;

####substract

Like add, but with substractions and using the first parameter as a base.

####concatenate

Add for strings, annoyingly typed.

####is_int

Returns true if all the given parameters are of integer type.

####is_bool

Returns true if all the given parameters are of boolean type.

####is_double

Returns true if all the given parameters are of double type.

####is_string

Returns true if all the given parameters are of string type.

####host_has

Returns true if the host has all given names on its symbol table. All names must be expressed as strings or as variables that solve to strings.

####host_get

Takes a single value (a string or a variable that solves to one) and returns the value that exits in the host's symbol table with that name. If the name does not exist, it should throw (the host behaviour is actually implementation-defined).

####host_query

Takes any number of values to make the host produce a single variable. Everything about this function is implementation-defined. Semantics imply that this function must be used to query the host for information, whereas host_do must be used to ask the host do change its state, for example:

beginfunction check_for_mistic_key [keytype as string];

	let result be host_query ["check_key", keytype];
	return [result];
endfunction;

beginfunction check_for_opened_door [door_id as int];

	let result be host_query ["is_door_open", door_id];
	return [result];
endfunction;

beginfunction open_door [keytype as string, door_id as int];

	if check_for_opened_door [door_id];

		return;
	endif;

	if not check_for_mistic_key [keytype];

		host_do ["show_message", "key needed", keytype];
	else;

		host_do ["open_door", door_id];
	endif;
endfunction;

In the host program, a door object can be programmed to hold its id and the keytype as a string and, on interaction, fire up an interpreter and call "open door";

	interpreter.run(host, outfacility, "opendoor", {keytype, id]);

The host function could be implemented as follows:

	variable host_query(const std::vector<variable>& _arguments) const {

		//type and size checking is skipped for brevity.

		std::string message=_arguments[0].str_val;

		if(message=="check_key") {

			return player_instance.has_key(_arguments[1].str_val);
		}
		else if(message=="is_door_open") {

			return current_map.get_door(_arguments[1].int_val).is_open();
		}

		//throw something...
	}

	void host_do(const std::vector<variable> _arguments) {

		//type and size checking is skipped for brevity.

		std::string message=_arguments[0].str_val;

		if(message=="open_door") {
			
			current_map.get_door(_arguments[1].int_val).open();
		}
		else if(message=="show_message") {

			//blah blah blah
		}
	}

###built in procedures 

These are the built-in procedures. None of them return any value and they can only appear on their own, never as part of any other statement.

####host_delete

Deletes the given symbols (as strings or variables that solve to strings) in the host. Should throw if the symbols do not exist, but that's implementation-defined.

####host_set

Takes two parameters, asking the host to set the symbol on the first parameter (a string or a variable that solves as one) to the value on the second. Semantics imply that the host should throw if the symbol does not exist, but nothing forces that. Language conventions also imply that the type of the symbol should not change, but ultimately that's defined by the host implementation.

####host_add

Takes two parameters, asking the host to add a new the symbol (the first parameter, a string or a variable that solves as one) with the value of the second. Semantics imply that the host should throw if the symbol already exists, but nothing forces that. 

####host_do

Takes any number of parameters to ask the host to perform completely implementation-defined actions. Semantics imply that the host is able to change its state as a consequence of a call to host_do.

####out

Uses the out_interface to output whatever values it is passed (integers, strings, booleans and doubles). Takes any number of parameters.

####fail

Halts the execution and throws an user error, whose message is the concatenation of all parameters passed (of any type).
