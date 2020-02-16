package main

import (
	"errors"
	"fmt"
	"strings"
)

type state struct {
	batteryUsed bool
	batteryLow bool
	mainsUp bool
	mainsDown bool
}

func (state state) String() string {
	retval := fmt.Sprintf("\tOn Battery: %t\n", state.batteryUsed)
	retval += fmt.Sprintf("\tBattery Low: %t\n", state.batteryLow)
	retval += fmt.Sprintf("\tMains Up: %t\n", state.mainsUp)
	retval += fmt.Sprintf("\tMains Low: %t\n", state.mainsDown)
	return retval
}

func (state state) Equal(other state) bool {
	return state.batteryLow == other.batteryLow &&
		state.batteryUsed == other.batteryUsed &&
		state.mainsUp == other.mainsUp &&
		state.mainsDown == other.mainsDown
}

type MonitorCommand struct {
}

var monitorCommand MonitorCommand

func init() {
	_, err := parser.AddCommand(
		"monitor",
		"Start Monitoring",
		"Start UPS monitoring process",
		&monitorCommand)
	if err != nil {
		panic(err)
	}
}

func updateStatus(line string) (newState state, err error) {
	sections := strings.Split(line, ",")
	if len(sections) != 4 {
		return newState, errors.New("unable to parse state, not long enough")
	}

	for index := range sections {
		parts := strings.Split(sections[index], ":")
		if len(parts) != 2 {
			return newState, errors.New("unable to parse state, invalid part")
		}
		key := strings.ToLower(parts[0])
		value := strings.ToLower(parts[1]) == "yes"
		switch key {
		case "batt":
				newState.batteryUsed = value
				break
		case "battlow":
			newState.batteryLow = value
			break
		case "mainsdwn":
			newState.mainsDown = value
			break
		case "mainsup":
			newState.mainsUp = value
			break
		}
	}

	return newState, nil
}

func issueShutdown() {
	fmt.Printf("Shutdown Now!\n")
}

func (x *MonitorCommand)Execute(args[]string) error {
	inBuffer := make([]byte, 50)
	lineBuffer := ""
	currentState := state{}
	nextState := state{}

	for {
		num, err := serialIO.Read(inBuffer)
		if err != nil {
			return err
		}

		if num == 0 {
			continue
		}

		startIndex := 0
		for index := 0; index < num; index++ {
			if inBuffer[index] == '\n' {
				lineBuffer += string(inBuffer[startIndex:index])
				nextState, err = updateStatus(lineBuffer)
				if err != nil {
					return err
				}
				lineBuffer = ""
				startIndex = index + 1
			}
		}

		if startIndex < num {
			lineBuffer += string(inBuffer[startIndex:num])
		}

		if !nextState.Equal(currentState) {
			currentState = nextState
			if (currentState.batteryLow) {
				issueShutdown()
			} else if (currentState.mainsDown && configFile.UPS.ShutdownOnNoMains) {
				issueShutdown()
			}
		}
	}
}