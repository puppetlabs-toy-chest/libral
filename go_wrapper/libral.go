package libral

/*
#cgo LDFLAGS: -fPIC -L${SRCDIR}/ -lstdc++ /usr/local/lib/libaugeas.dylib /usr/local/lib/libral.a  /usr/local/lib/libcpp-hocon.a /usr/local/lib/libyaml-cpp.a /usr/local/lib/leatherman_execution.a /usr/local/lib/leatherman_logging.a /usr/local/lib/leatherman_locale.a /usr/local/lib/leatherman_ruby.a /usr/local/lib/leatherman_dynamic_library.a /usr/local/lib/leatherman_util.a /usr/local/lib/leatherman_file_util.a /usr/local/lib/leatherman_json_container.a /usr/local/lib/libboost_system-mt.a /usr/local/lib/libboost_log-mt.a /usr/local/lib/libboost_log_setup-mt.a /usr/local/lib/libboost_thread-mt.a /usr/local/lib/libboost_date_time.a /usr/local/lib/libboost_filesystem-mt.a /usr/local/lib/libboost_chrono-mt.a /usr/local/lib/libboost_regex-mt.a /usr/local/lib/libboost_atomic-mt.a /usr/local/lib/libboost_program_options-mt.a /usr/local/lib/libboost_locale-mt.a /usr/local/lib/libiconv.a
#cgo CFLAGS: -I${SRCDIR}/../lib/inc

#include "libral/cwrapper.hpp"
#include <stdlib.h>
*/
import "C"

import (
	"encoding/json"
	"fmt"
	"libral/types"
	"unsafe"
)

// GetProviders returns the resource providers available to libral.
func GetProviders() ([]types.Provider, error) {
	rawProviders, err := getProvidersRaw()
	if err != nil {
		return nil, err
	}

	var providersResult types.ProvidersResult

	if err := json.Unmarshal([]byte(rawProviders), &providersResult); err != nil {
		return nil, fmt.Errorf("Error unmarshalling provider JSON: %v", err)
	}

	return providersResult.Providers, nil
}

// GetResources returns all resources of the specified provider type.
func GetResources(typeName string) ([]types.Resource, error) {
	var result []types.Resource
	rawResources, err := getResourcesRaw(typeName)
	if err != nil {
		return nil, err
	}

	var resourcesResult types.ResourcesResult

	if err := json.Unmarshal([]byte(rawResources), &resourcesResult); err != nil {
		return nil, fmt.Errorf("Error unmarshalling resources JSON: %v", err)
	}

	for _, rawResource := range resourcesResult.Resources {
		var resource types.Resource
		if err := json.Unmarshal([]byte(rawResource), &resource); err != nil {
			return nil, fmt.Errorf("Error unmarshalling resource JSON: %v", err)
		}
		resource.Raw = rawResource
		var attributes map[string]interface{}
		if err := json.Unmarshal([]byte(rawResource), &attributes); err != nil {
			return nil, fmt.Errorf("Error unmarshalling resource attributes JSON: %v", err)
		}
		_, ok := attributes["ral"]
		if ok {
			delete(attributes, "ral")
		}
		resource.Attributes = attributes
		result = append(result, resource)
	}

	return result, nil
}

// GetResource returns a Resource of the specified provider type with matching resource name,
// or an empty resource if no match is found, if more than one resource exists with the same
// name an error is thrown.
func GetResource(typeName, resourceName string) (types.Resource, error) {
	var result types.Resource
	rawResource, err := getResourceRaw(typeName, resourceName)
	if err != nil {
		return types.Resource{}, err
	}

	var resourceResult types.ResourceResult
	if err := json.Unmarshal([]byte(rawResource), &resourceResult); err != nil {
		return types.Resource{}, fmt.Errorf("Error unmarshalling resource JSON: %v", err)
	}

	if err := json.Unmarshal([]byte(resourceResult.Resource), &result); err != nil {
		return types.Resource{}, fmt.Errorf("Error unmarshalling resource details JSON: %v", err)
	}
	result.Raw = resourceResult.Resource
	var attributes map[string]interface{}
	if err := json.Unmarshal([]byte(resourceResult.Resource), &attributes); err != nil {
		return types.Resource{}, fmt.Errorf("Error unmarshalling resource attributes JSON: %v", err)
	}
	_, ok := attributes["ral"]
	if ok {
		delete(attributes, "ral")
	}
	result.Attributes = attributes

	return result, nil
}

// getProvidersRaw returns a JSON string containing a list of providers known to libral.
//
// For example:
//
//   {
//       "providers": [
//           {
//               "name": "file::posix",
//               "type": "file",
//               "source": "builtin",
//               "desc": "A provider to manage POSIX files.\n",
//               "suitable": true,
//               "attributes": [
//                   {
//                       "name": "checksum",
//                       "desc": "(missing description)",
//                       "type": "enum[md5, md5lite, sha256, sha256lite, mtime, ctime, none]",
//                       "kind": "r"
//                   },
//                   ...
//               ]
//           },
//           {
//               "name": "host::aug",
//               "type": "host",
//               ...
//           }
//       ]
//   }
func getProvidersRaw() (string, error) {
	var resultC *C.char
	defer C.free(unsafe.Pointer(resultC))

	ok := C.get_providers(&resultC)
	if ok != 0 {
		return "", fmt.Errorf("Error thrown calling get_providers: %d", ok)
	}
	result := C.GoString(resultC)
	return result, nil
}

// getResourcesRaw returns a JSON string containing a list all resources of the specified type
// found by libral.
//
// For example, querying the `host` type:
//
//   {
//       "resources": [
//           {
//               "name": "localhost",
//               "ensure": "present",
//               "ip": "127.0.0.1",
//               "target": "/etc/hosts",
//               "ral": {
//                   "type": "host",
//                   "provider": "host::aug"
//               }
//           },
//           {
//               "name": "broadcasthost",
//               "ensure": "present",
//               "ip": "255.255.255.255",
//               "target": "/etc/hosts",
//               "ral": {
//                   "type": "host",
//                   "provider": "host::aug"
//               }
//           }
//       ]
//   }
func getResourcesRaw(typeName string) (string, error) {
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

// getResourceRaw returns a JSON string containing the matching resources of the specified type
// found by libral. It will throw an error if multiple resources are found with the same name.
//
// For example, querying the `host` type `broadcasthost`:
//
//   {
//       "resources": [
//           {
//               "name": "broadcasthost",
//               "ensure": "present",
//               "ip": "255.255.255.255",
//               "target": "/etc/hosts",
//               "ral": {
//                   "type": "host",
//                   "provider": "host::aug"
//               }
//           }
//       ]
//   }
func getResourceRaw(typeName, resourceName string) (string, error) {
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
