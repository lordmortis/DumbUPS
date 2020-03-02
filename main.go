package main

import (
	"github.com/jacobsa/go-serial/serial"
	"github.com/jessevdk/go-flags"
	"github.com/pkg/errors"
	"io"
	"log"
	"os"
	"runtime"

	"github.com/lordmortis/DumbUPS/config"
)

type Options struct {
	ConfigFile string `long:"configFile" description:"path to config.json file" default:"/etc/dumbups.conf"`
}

var options Options
var parser = flags.NewParser(&options, flags.Default)
var configFile *config.Config
var serialIO io.ReadWriteCloser

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())

	parser.CommandHandler = func(command flags.Commander, args []string) error {
		var err error

		configFile, err = config.Load(&options.ConfigFile)
		if err != nil {
			return errors.Wrap(err, "Unable to parse config file")
		}

		options := serial.OpenOptions{
			PortName: configFile.Serial.PortName,
			BaudRate: configFile.Serial.BaudRate,
			DataBits: configFile.Serial.DataBits,
			StopBits: configFile.Serial.StopBits,
			InterCharacterTimeout: 100,
			MinimumReadSize: 0,
		}

		serialIO, err = serial.Open(options)
		if err != nil {
			log.Fatalf("serial.Open: %v", err)
		}

		// Make sure to close it later.
		defer serialIO.Close()

		return command.Execute(args)
	}

	if _, err := parser.Parse(); err != nil {
		if flagsErr, ok := err.(*flags.Error); ok && flagsErr.Type == flags.ErrHelp {
			os.Exit(0)
		} else {
			os.Exit(1)
		}
	}
}