package main

import (
	"github.com/jessevdk/go-flags"
	"github.com/pkg/errors"
	"runtime"

	"github.com/lordmortis/DumbUPS/config"
)

type Options struct {
	ConfigFile string `long:"configFile" description:"path to config.json file" default:"../config.json"`
}

var options Options
var parser = flags.NewParser(&options, flags.Default)
var configFile *config.Config

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())

	parser.CommandHandler = func(command flags.Commander, args []string) error {
		var err error

		configFile, err = config.Load(&options.ConfigFile)
		if err != nil {
			return errors.Wrap(err, "Unable to parse config file")
		}

		return command.Execute(args)
	}
}