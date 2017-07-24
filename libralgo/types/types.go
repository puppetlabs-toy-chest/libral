package types

import (
	"encoding/json"
)

// Attribute describes the structure of an attribute returned by a provider
type Attribute struct {
	Name string `json:"name"`
	Desc string `json:"desc"`
	Type string `json:"type"`
	Kind string `json:"kind"`
}

// RAL represents the provider which has generated a resource
type RAL struct {
	Type     string `json:"type"`
	Provider string `json:"provider"`
}

// Provider represents a provider returned from libral
type Provider struct {
	Name       string      `json:"name"`
	Type       string      `json:"type"`
	Source     string      `json:"source"`
	Desc       string      `json:"desc"`
	Suitable   bool        `json:"suitable"`
	Attributes []Attribute `json:"attributes"`
}

// Resource represents a resource returned from libral
//
//  - RAL provider which exposed the resource
//  - Attributes map of attributes exposed by the provider
//  - Raw JSON representation of the resource as a []byte
type Resource struct {
	Name       string                 `json:"name"`
	RAL        RAL                    `json:"ral"`
	Attributes map[string]interface{} `json:"attributes"`
	Raw        []byte                 `json:"raw"`
}

// ProvidersResult represents a result from a get_providers call
type ProvidersResult struct {
	Providers []Provider `json:"providers"`
}

// ResourcesResult represents a result from a get_resources call
type ResourcesResult struct {
	Resources []json.RawMessage `json:"resources"`
}

// ResourceResult represents a result from a get_resource call
type ResourceResult struct {
	Resource json.RawMessage `json:"resource"`
}
