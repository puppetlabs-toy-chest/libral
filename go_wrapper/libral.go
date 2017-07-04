package libral

/*
#cgo LDFLAGS: -L/Users/john.mccabe/workspaces/github/libral/build/lib -lral -lstdc++ /usr/local/lib/leatherman_json_container.a /usr/local/lib/leatherman_execution.a /usr/local/lib/leatherman_logging.a /usr/local/lib/leatherman_locale.a /usr/local/lib/libboost_locale-mt.dylib /usr/local/lib/libboost_system-mt.dylib /usr/local/lib/libboost_log-mt.dylib /usr/local/lib/libboost_log_setup-mt.dylib /usr/local/lib/libboost_thread-mt.dylib /usr/local/lib/libboost_date_time-mt.dylib /usr/local/lib/libboost_filesystem-mt.dylib /usr/local/lib/libboost_chrono-mt.dylib /usr/local/lib/libboost_regex-mt.dylib /usr/local/lib/libboost_atomic-mt.dylib /usr/local/lib/leatherman_util.a /usr/local/lib/leatherman_file_util.a /usr/local/lib/libyaml-cpp.dylib /usr/local/lib/libboost_program_options-mt.dylib /usr/local/lib/libaugeas.dylib
#cgo CFLAGS: -I/Users/john.mccabe/workspaces/github/libral/lib/inc

#include "libral/cwrapper.hpp"

// extern char* get_all(char* type_name_c);
*/
import "C"

// GetAll invokes the get_all(type_name) function in the wrapped libral
// library converting the output from a char* to native golang string
func GetAll(typeName string) string {
	var hosts string
	hosts = C.GoString(C.get_all(C.CString(typeName)))
	return hosts
}
