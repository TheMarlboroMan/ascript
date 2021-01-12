#ascript

Short for annoying script. An annoying script language...

## fun stuff

- To allow for yielding there's no recursive interpreter... this opens the door for all sorts of unpleasantries.
- When calling interpreter::run with a script is imperative that the script object is not gone out of scope. This could be fixed, but it wouldn't be annoying.
- Scripts can't be copied. That's right. There's an unique_ptr down there, could be fixed but again, would not be annoying.

##BUGS:

- out ["something", var, ", ", othervar]; causes the parser to explode, it cannot fathom a comma following a quote.

##TODO:

- Cleanup arguments / parameters name, they should not be interchangeable in a readable codebase.
- Check TODOs.
- As an interesting exercise, try separating "variable" into "type", which would have subtypes for int, bool and such.
- Study homogeneous arrays, with a syntax like let crap be {1,2,3}; and then array_size[crap], array_get[crap, 1], array_set[crap, 2, 33], and so on.
- Study structures (symbol tables on themselves, custom types). they would be... interesting, let crap be packed type [member:value, member:value, member:value];
- Document classes.
- Write small manual.
- Write tests of all the language features so we can know what to expect.
- Do a char by char tokenizer, the one we have has slight bugs.
- Interpreters could get a hold of the final return value of a function, if any.
	- Sure, if not... what do they return?????

##manual

###the host

###general syntax notes

###types

###declaring functions

###variables

###control flow

###loops

###calling other functions

###built in functions

###built int procedures 

## Changelog
