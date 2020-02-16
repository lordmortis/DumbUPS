package config

import (
	"encoding/json"
	"io/ioutil"
)

type Config struct {
	SerialPort string
}

func defaultConfig() Config {
	var config = Config{}
	config.SerialPort = "/dev/ttyUSBS0"
	return config
}

func Load(filename *string) (*Config, error) {
	fileString, err := ioutil.ReadFile(*filename)
	if err != nil {
		return nil, err
	}

	var config = defaultConfig()

	err = json.Unmarshal(fileString, &config)
	if err != nil {
		return nil, err
	}

	return &config, nil
}