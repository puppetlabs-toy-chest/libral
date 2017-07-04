package main

import (
	"fmt"
	"libral"
)

func main() {
	fmt.Println("### We're here!")
	r := libral.GetAll("host")
	fmt.Printf("### Hosts %s", r)

	fmt.Println("\n### Let's get more!!!")
	r, e := libral.GetAllWithErr("")
	fmt.Printf("### Hosts %s", r)

	if e != nil {
		fmt.Printf("### Oh shit: %v", e)
	}
}
