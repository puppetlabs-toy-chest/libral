package libral

import (
	"testing"

	"github.com/tidwall/gjson"
)

var getProviderTests = []struct {
	title    string
	jsonPath string
	expected string
}{
	{
		title:    "Found file provider",
		jsonPath: `providers.#[type=="file"].name`,
		expected: "file::posix",
	},
	{
		title:    "Found host provider",
		jsonPath: `providers.#[type=="host"].name`,
		expected: "host::aug",
	},
}

func Test_GetProviders(t *testing.T) {
	for _, test := range getProviderTests {
		t.Run(test.title, func(t *testing.T) {
			providers, err := GetProviders()
			if err != nil {
				t.Fatalf("Unxpected error thrown: %v", err)
			}

			value := gjson.Get(providers, test.jsonPath)

			if value.String() != test.expected {
				t.Fatalf("Unxpected name returned [%s], expected [%s]", value, test.expected)
			}
		})
	}
}

var getResourcesTests = []struct {
	title        string
	resourceType string
	jsonPath     string
	expected     string
	expectError  bool
}{
	{
		title:        "Get host type resources",
		resourceType: "host",
		jsonPath:     `resources.#[ip=="127.0.0.1"].name`,
		expected:     "localhost",
		expectError:  false,
	},
	{
		title:        "Get invalid type resources",
		resourceType: "this_is_not_a_valid_type",
		expectError:  true,
	},
}

func Test_GetResources(t *testing.T) {
	for _, test := range getResourcesTests {
		t.Run(test.title, func(t *testing.T) {
			resources, err := GetResources(test.resourceType)
			if err != nil {
				if !test.expectError {
					t.Fatalf("Unxpected error thrown: %v", err)
				} else {
					t.Logf("Expected error thrown: %v", err)
					return
				}
			}

			value := gjson.Get(resources, test.jsonPath)

			if value.String() != test.expected {
				t.Fatalf("Unxpected name returned [%s], expected [%s]", value, test.expected)
			}
		})
	}
}

var getResourceTests = []struct {
	title            string
	resourceType     string
	resourceInstance string
	jsonPath         string
	expected         string
	expectError      bool
}{
	{
		title:            "Get file resource",
		resourceType:     "file",
		resourceInstance: "/etc/hosts",
		jsonPath:         `resource.ensure`,
		expected:         "file",
		expectError:      false,
	},
	{
		title:            "Get resource with invalid type",
		resourceType:     "this_is_not_a_valid_type",
		resourceInstance: "/etc/hosts",
		expectError:      true,
	},
	{
		title:            "Get resource with vaild type and invalid name",
		resourceType:     "file",
		resourceInstance: "this_is_not_a_valid_resource_name",
		expectError:      true,
	},
}

func Test_GetResource(t *testing.T) {
	for _, test := range getResourceTests {
		t.Run(test.title, func(t *testing.T) {
			resource, err := GetResource(test.resourceType, test.resourceInstance)
			if err != nil {
				if !test.expectError {
					t.Fatalf("Unxpected error thrown: %v", err)
				} else {
					t.Logf("Expected error thrown: %v", err)
					return
				}
			}

			value := gjson.Get(resource, test.jsonPath)

			if value.String() != test.expected {
				t.Fatalf("Unxpected name returned [%s], expected [%s]", value, test.expected)
			}
		})
	}
}
