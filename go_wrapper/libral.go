package libral

/*
#cgo LDFLAGS: -L/Users/john.mccabe/workspaces/github/libral/build/lib -lral -lstdc++ /usr/local/lib/leatherman_json_container.a /usr/local/lib/leatherman_execution.a /usr/local/lib/leatherman_logging.a /usr/local/lib/leatherman_locale.a /usr/local/lib/libboost_locale-mt.dylib /usr/local/lib/libboost_system-mt.dylib /usr/local/lib/libboost_log-mt.dylib /usr/local/lib/libboost_log_setup-mt.dylib /usr/local/lib/libboost_thread-mt.dylib /usr/local/lib/libboost_date_time-mt.dylib /usr/local/lib/libboost_filesystem-mt.dylib /usr/local/lib/libboost_chrono-mt.dylib /usr/local/lib/libboost_regex-mt.dylib /usr/local/lib/libboost_atomic-mt.dylib /usr/local/lib/leatherman_util.a /usr/local/lib/leatherman_file_util.a /usr/local/lib/libyaml-cpp.dylib /usr/local/lib/libboost_program_options-mt.dylib /usr/local/lib/libaugeas.dylib
#cgo CFLAGS: -I/Users/john.mccabe/workspaces/github/libral/lib/inc

#include "libral/cwrapper.hpp"

// extern char* get_all(char* type_name_c);
// extern uint8_t get_all_with_err(char** resource, char* type_name);
// extern outcome get_all_outcome(char* type_name_c);
*/
import "C"

import "fmt"

//"unsafe"

// Outcome TODO
type Outcome struct {
	Result    string
	ErrorCode int
}

// OutcomeC TODO
// type OutcomeC C.struct_outcome

func convertOutcome(in C.struct_outcome) Outcome {
	out := new(Outcome)
	out.Result = C.GoString(in.result)
	out.ErrorCode = int(in.error_code)
	return *out
}

// GetAllOutcome TODO
func GetAllOutcome(typeName string) (string, error) {
	typeNameC := C.CString(typeName)
	// defer C.free(unsafe.Pointer(typeNameC))
	outC := C.get_all_outcome(typeNameC)
	out := convertOutcome(outC)
	if out.ErrorCode != 0 {
		return "", fmt.Errorf("Error thrown calling native get_all_outcome: %d", out.ErrorCode)
	}
	return out.Result, nil
}

// GetAll invokes the get_all(type_name) function in the wrapped libral
// library converting the output from a char* to native golang string
func GetAll(typeName string) string {
	var hosts string
	typeNameC := C.CString(typeName)
	// defer C.free(unsafe.Pointer(typeNameC))
	hosts = C.GoString(C.get_all(typeNameC))
	return hosts
}

// GetAll invokes the get_all_with_err(&resource, type_name) function in
// the wrapped libral
// func GetAllWithErr(typeName string) (string, error) {
// 	//var resources string
// 	var r *C.char
// 	var resources unsafe.Pointer(r)
// 	defer C.Free(resources)
// 	var err error
// 	errorCode := C.get_all_with_err(r, C.CString(typeName))
// 	if errorCode != 0 {
// 		err = fmt.Errorf("### It didn't work! Error code: %d", errorCode)
// 		return "", err
// 	}

// 	return C.GoString(*r), nil
// }
