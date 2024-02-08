#include <ascript/lib.h>
#include <sstream>

using namespace ascript;

std::string ascript::get_lib_version() {

	std::stringstream ss;
	ss<<MAJOR_VERSION<<"."<<MINOR_VERSION<<"."<<PATCH_VERSION<<"-"<<LIB_VERSION<<"-"<<RELEASE_VERSION;
	return ss.str();
}
