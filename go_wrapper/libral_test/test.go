package main

import (
	"fmt"
	"libral"
)

func main() {
	fmt.Println("### We're here!")
	queryType := "host"
	hosts, err := libral.GetAllOutcome(queryType)
	if err != nil {
		fmt.Printf("Error throw calling libral.GetAllOutcome(%s): %v", queryType, err)
		return
	}
	fmt.Printf("Successfully called libral.GetAllOutcome(%s)", queryType)
	fmt.Printf("### Hosts %s", hosts)

	// fmt.Println("\n### Let's get more!!!")
	// r, e := libral.GetAllWithErr("")
	// fmt.Printf("### Hosts %s", r)

	// if e != nil {
	// 	fmt.Printf("### Oh shit: %v", e)
	// }
}
