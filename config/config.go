package config

import (
	"github.com/pelletier/go-toml"
	"io/ioutil"
	"strings"
)

type SerialConfig struct {
	PortName string
	BaudRate uint
	DataBits uint
	StopBits uint
}

type UPSConfig struct {
	ShutdownString string `toml:"shutdown"`
	ShutdownOnNoMains bool
}

type Config struct {
	Serial SerialConfig
	UPS UPSConfig
}

func defaultConfig() Config {
	var config = Config{}
	config.Serial.PortName = "/dev/ttyUSBS0"
	config.Serial.BaudRate = 9600
	config.Serial.DataBits = 8
	config.Serial.StopBits = 1
	config.UPS.ShutdownString = "immediate"
	return config
}

func (c *Config)setup() error {
	// setup shutdown string parsing
	c.UPS.ShutdownString = strings.ToLower(c.UPS.ShutdownString)
	switch c.UPS.ShutdownString {
	case "lowbattery":
		c.UPS.ShutdownOnNoMains = false
		break
	case "immediate":
		c.UPS.ShutdownOnNoMains = true
		break

	}
	return nil
}

func Load(filename *string) (*Config, error) {
	fileString, err := ioutil.ReadFile(*filename)
	if err != nil {
		return nil, err
	}

	var config = defaultConfig()

	err = toml.Unmarshal(fileString, &config)
	if err != nil {
		return nil, err
	}

	err = config.setup()

	return &config, nil
}