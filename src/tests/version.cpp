#include <iostream>
#include "ascript/lib.h"

int main(
	int ,
	char **
) {

	std::cout<<"ascript library version "<<ascript::get_lib_version()<<std::endl;
	return 0;
}
