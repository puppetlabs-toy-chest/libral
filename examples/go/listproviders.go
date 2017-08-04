package main

import (
	"fmt"
	"log"
	"os"

	libral "github.com/puppetlabs/libral/libralgo"
)

func main() {
	debug := log.New(os.Stderr, "debug: ", log.Ldate|log.Ltime|log.Lshortfile)

	providers, err := libral.GetProviders()
	if err != nil {
		debug.Fatalf("Error thrown when getting providers [%v], exiting...", err)
	}

	if len(providers) == 0 {
		debug.Fatalln("No providers found, exiting...")
	}

	fmt.Println("Found Providers:")
	for _, provider := range providers {
		fmt.Printf("  %s\n", provider.Name)
	}
}
