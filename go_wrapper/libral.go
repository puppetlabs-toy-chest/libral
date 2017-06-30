package libral

/*
#cgo LDFLAGS: -L${SRCDIR}/../build/lib -lral -lstdc++ /usr/local/lib/leatherman_json_container.a /usr/local/lib/leatherman_execution.a /usr/local/lib/leatherman_logging.a /usr/local/lib/leatherman_locale.a /usr/local/lib/libboost_locale-mt.dylib /usr/local/lib/libboost_system-mt.dylib /usr/local/lib/libboost_log-mt.dylib /usr/local/lib/libboost_log_setup-mt.dylib /usr/local/lib/libboost_thread-mt.dylib /usr/local/lib/libboost_date_time-mt.dylib /usr/local/lib/libboost_filesystem-mt.dylib /usr/local/lib/libboost_chrono-mt.dylib /usr/local/lib/libboost_regex-mt.dylib /usr/local/lib/libboost_atomic-mt.dylib /usr/local/lib/leatherman_util.a /usr/local/lib/leatherman_file_util.a /usr/local/lib/libyaml-cpp.dylib /usr/local/lib/libboost_program_options-mt.dylib /usr/local/lib/libaugeas.dylib
#cgo CFLAGS: -I${SRCDIR}/../lib/inc

#include "libral/cwrapper.hpp"
#include <stdlib.h>
*/
import "C"

import (
	"fmt"
	"unsafe"
)

// GetTypes TODO
func GetProviders() (string, error) {
	var resultC *C.char
	defer C.free(unsafe.Pointer(resultC))

	ok := C.get_providers(&resultC)
	if ok != 0 {
		return "", fmt.Errorf("Error thrown calling get_providers: %d", ok)
	}
	result := C.GoString(resultC)
	return result, nil
}

// GetResources TODO
func GetResources(typeName string) (string, error) {
	var resultC *C.char
	defer C.free(unsafe.Pointer(resultC))
	typeNameC := C.CString(typeName)
	defer C.free(unsafe.Pointer(typeNameC))

	ok := C.get_resources(&resultC, typeNameC)
	if ok != 0 {
		return "", fmt.Errorf("Error thrown calling get_all_resources: %d", ok)
	}
	result := C.GoString(resultC)
	return result, nil
}

// GetResource TODO
func GetResource(typeName, resourceName string) (string, error) {
	var resultC *C.char
	defer C.free(unsafe.Pointer(resultC))
	typeNameC := C.CString(typeName)
	defer C.free(unsafe.Pointer(typeNameC))
	resourceNameC := C.CString(resourceName)
	defer C.free(unsafe.Pointer(resourceNameC))

	ok := C.get_resource(&resultC, typeNameC, resourceNameC)
	if ok != 0 {
		return "", fmt.Errorf("Error thrown calling get_all_resources: %d", ok)
	}
	result := C.GoString(resultC)
	return result, nil
}
