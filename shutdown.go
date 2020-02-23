package main

import "fmt"

type ShutdownCommand struct {
}

var shutdownCommand ShutdownCommand

func init() {
	_, err := parser.AddCommand(
		"shutdown",
		"Turn off the UPS",
		"Turn off the UPS",
		&shutdownCommand)
	if err != nil {
		panic(err)
	}
}

func (x *ShutdownCommand)Execute(args[]string) error {
	fmt.Printf("Triggering UPS Shutdown\n")
	_, err := serialIO.Write([]byte("shut\n"))
	if err != nil {
		return err
	}

	return nil
}