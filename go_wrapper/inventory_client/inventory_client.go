package main

// See - https://github.com/puppetlabs/puppetlabs-inventory for information on
// puppet-inventory

import (
	"encoding/json"
	"fmt"
	"libral"
	"log"
	"os"
	"strings"
)

// PuppetInventory is a list of resources gathered via libral
type PuppetInventory struct {
	Resources []map[string]interface{} `json:"resources"`
}

func main() {
	var inventory PuppetInventory
	debug := log.New(os.Stderr, "debug: ", log.Ldate|log.Ltime|log.Lshortfile)

	providers, err := libral.GetProviders()
	if err != nil {
		debug.Fatalf("Error thrown when getting providers [%v], exiting...", err)
	}

	for _, provider := range providers {
		debug.Printf("Parsing provider: %s\n", provider.Name)
		r, p := parseProviderName(provider.Name) // Could iterate over a defined list here instead of getting all
		resources, err := libral.GetResources(r)
		if err != nil {
			debug.Fatalf("Error thrown when getting [%s] resources [%v], exiting...", r, err)
		}
		if len(resources) == 0 {
			debug.Println("  - No resources found")
			continue
		}
		for _, resource := range resources {
			debug.Printf("  - Parsing resource: %s\n", resource.Name)
			inventoryResource := resource.Attributes
			if _, ok := inventoryResource["title"]; !ok {
				inventoryResource["title"] = resource.Attributes["name"]
			}
			inventoryResource["resource"] = r
			inventoryResource["provider"] = p
			inventory.Resources = append(inventory.Resources, inventoryResource)
		}
	}

	inventoryJSON, err := json.MarshalIndent(inventory, "", "  ")
	if err != nil {
		debug.Fatalf("Error thrown when marshalling inventory [%v], exiting...", err)
	}
	fmt.Println(string(inventoryJSON))

}

// parseProviderName splits the provider name (`resource::provider`) into
// Puppet inventory read strings `resource` and `:provider`
func parseProviderName(providerName string) (resource, provider string) {
	s := strings.SplitN(providerName, ":", 2)
	return s[0], s[1]
}
