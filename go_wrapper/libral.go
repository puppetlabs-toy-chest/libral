package libral

/*
#cgo LDFLAGS: -L/Users/alessandro/code/libral/build/lib -lral -lstdc++ /usr/local/lib/leatherman_json_container.a /usr/local/lib/leatherman_execution.a /usr/local/lib/leatherman_logging.a /usr/local/lib/leatherman_locale.a /usr/local/lib/libboost_locale-mt.dylib /usr/local/lib/libboost_system-mt.dylib /usr/local/lib/libboost_log-mt.dylib /usr/local/lib/libboost_log_setup-mt.dylib /usr/local/lib/libboost_thread-mt.dylib /usr/local/lib/libboost_date_time-mt.dylib /usr/local/lib/libboost_filesystem-mt.dylib /usr/local/lib/libboost_chrono-mt.dylib /usr/local/lib/libboost_regex-mt.dylib /usr/local/lib/libboost_atomic-mt.dylib /usr/local/lib/leatherman_util.a /usr/local/lib/leatherman_file_util.a /usr/local/lib/libyaml-cpp.dylib /usr/local/lib/libboost_program_options-mt.dylib /usr/local/lib/libaugeas.dylib
#cgo CFLAGS: -I/Users/alessandro/code/libral/lib/inc

#include "libral/cwrapper.hpp"
#include "stdint.h"

// extern char* get_all(char* type_name_c);
// extern uint8_t get_all_with_err(char* resource, char* type_name);
*/
import "C"
import (
	//"unsafe"
	"fmt"
)

// GetAll invokes the get_all(type_name) function in the wrapped libral
// library converting the output from a char* to native golang string
func GetAll(typeName string) string {
	var hosts string
	hosts = C.GoString(C.get_all(C.CString(typeName)))

	return hosts
}


// GetAll invokes the get_all_with_err(&resource, type_name) function in
// the wrapped libral
func GetAllWithErr(typeName string) (string, error) {
	//var resources string
	var r *C.char
	//var resources_p unsafe.Pointer(r)
	//defer C.Free(resources)
	var err error
	errorCode := C.get_all_with_err(r, C.CString(typeName))
	if errorCode != 0 {
		err = fmt.Errorf("### It didn't work! Error code: %d",  errorCode)
		return "", err
	}

	return C.GoString(r), nil
}
