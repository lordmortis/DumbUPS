// +build linux
package main

import (
	"fmt"
	"os/exec"
)

func issueShutdown() {
	fmt.Printf("Shutdown")
	if err := exec.Command("shutdown now").Run(); err != nil {
		fmt.Println("Failed to initiate shutdown:", err)
	}
}
