package main

import "fmt"

func printBuildEnv(env map[string]string) {
	fmt.Println("env:")
	if len(env) == 0 {
		fmt.Println("  <nil>")
	} else {
		for name, value := range env {
			fmt.Printf("  %s: %s\n", name, value)
		}
	}
}

func ExecListCommand() error {
	plan, err := readBuildTargets("build.claw")
	if err != nil {
		return err
	}

	printBuildEnv(plan.Env)
	fmt.Println()
	fmt.Println("targets:")
	for _, t := range plan.Targets {
		fmt.Printf("  %s\n", t.Name)
	}
	return nil
}
