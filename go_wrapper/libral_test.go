package libral

import "testing"

var getProviderNameTests = []struct {
	title        string
	providerType string
	expectedName string
}{
	{
		title:        "Found file provider",
		providerType: "file",
		expectedName: "file::posix",
	},
	{
		title:        "Found host provider",
		providerType: "host",
		expectedName: "host::aug",
	},
}

func Test_GetProviders(t *testing.T) {
	for _, test := range getProviderNameTests {
		t.Run(test.title, func(t *testing.T) {
			providers, err := GetProviders()
			if err != nil {
				t.Fatalf("Unxpected error thrown: %v", err)
			}

			for _, provider := range providers {
				if provider.Name == test.expectedName {
					t.Logf("Expected provider [%s] found", provider.Name)
					return
				}
			}
			t.Fatalf("Expected provider [%s] not found", test.expectedName)
		})
	}
}

var getResourcesTests = []struct {
	title             string
	providerType      string
	expectedName      string
	expectedAttr      string
	expectedAttrValue string
	expectError       bool
}{
	{
		title:             "Get host type resources",
		providerType:      "host",
		expectedName:      "localhost",
		expectedAttr:      "ip",
		expectedAttrValue: "127.0.0.1",
		expectError:       false,
	},
	{
		title:        "Get invalid type resources",
		providerType: "this_is_not_a_valid_type",
		expectError:  true,
	},
}

func Test_GetResources(t *testing.T) {
	for _, test := range getResourcesTests {
		t.Run(test.title, func(t *testing.T) {
			resources, err := GetResources(test.providerType)
			if err != nil {
				if !test.expectError {
					t.Fatalf("Unxpected error thrown: %v", err)
				} else {
					t.Logf("Expected error thrown: %v", err)
					return
				}
			}

			for _, resource := range resources {
				if resource.Name == test.expectedName {
					t.Logf("Expected resource name [%s] found", resource.Name)
					if resource.Attributes[test.expectedAttr] == test.expectedAttrValue {
						t.Logf("Expected resource attribute [%s] value [%s] found", test.expectedAttr, test.expectedAttrValue)
						return
					}
					t.Fatalf("Expected resource attribute [%s] value [%s] not found", test.expectedAttr, test.expectedAttrValue)
				}
			}
			t.Fatalf("Expected resource [%s] not found", test.expectedName)
		})
	}
}

var getResourceTests = []struct {
	title             string
	providerType      string
	resourceInstance  string
	expectedAttr      string
	expectedAttrValue string
	expectError       bool
}{
	{
		title:             "Get file resource",
		providerType:      "file",
		resourceInstance:  "/etc/hosts",
		expectedAttr:      `ensure`,
		expectedAttrValue: "file",
		expectError:       false,
	},
	{
		title:            "Get resource with invalid type",
		providerType:     "this_is_not_a_valid_type",
		resourceInstance: "/etc/hosts",
		expectError:      true,
	},
	{
		title:            "Get resource with vaild type and invalid name",
		providerType:     "file",
		resourceInstance: "this_is_not_a_valid_resource_name",
		expectError:      true,
	},
}

func Test_GetResource(t *testing.T) {
	for _, test := range getResourceTests {
		t.Run(test.title, func(t *testing.T) {
			resource, err := GetResource(test.providerType, test.resourceInstance)
			if err != nil {
				if !test.expectError {
					t.Fatalf("Unxpected error thrown: %v", err)
				} else {
					t.Logf("Expected error thrown: %v", err)
					return
				}
			}

			if resource.Name == test.resourceInstance {
				t.Logf("Expected resource name [%s] found", resource.Name)
				if resource.Attributes[test.expectedAttr] == test.expectedAttrValue {
					t.Logf("Expected resource attribute [%s] value [%s] found", test.expectedAttr, test.expectedAttrValue)
					return
				}
				t.Fatalf("Expected resource attribute [%s] value [%s] not found", test.expectedAttr, test.expectedAttrValue)
			}
			t.Fatalf("Resource name [%s] does not match expected value [%s]", resource.Name, test.resourceInstance)

		})
	}
}
