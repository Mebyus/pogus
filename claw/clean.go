package main

import "os"

func ExecCleanCommand() error {
	return os.RemoveAll("build")
}
