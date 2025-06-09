package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strings"
)

func main() {
	if len(os.Args) < 2 {
		err := ExecDefaultBuildCommand()
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		return
	}

	command := os.Args[1]
	var err error
	switch command {
	case "build":
		var buildKind BuildKind
		var target string
		if len(os.Args) >= 3 {
			// if strings.HasPrefix()
			// switch os.Args[2] {
			// case "--test":
			// 	buildKind = BuildTest
			// case "--release":
			// 	buildKind = BuildRelease
			// default:
			// 	buildKind = BuildDebug
			// }
			target = os.Args[len(os.Args)-1]
		}

		err = ExecBuildCommand(&BuildCommand{
			Target:    target,
			BuildKind: buildKind,
		})
	case "list":
		err = ExecListCommand()
	case "clean":
		err = ExecCleanCommand()
	default:
		err = fmt.Errorf("unknown command: %s", command)
	}
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

type BuildPlan struct {
	Targets []BuildTarget

	Env map[string]string
}

type BuildTarget struct {
	Links []string

	Name string

	RootSourceFile string
}

func readBuildTargets(path string) (*BuildPlan, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}

	env := make(map[string]string)
	var targets []BuildTarget
	scanner := bufio.NewScanner(f)
	var target BuildTarget // current build target
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		if line == "}" {
			targets = append(targets, target)
			target = BuildTarget{}
			continue
		}

		if strings.HasPrefix(line, "#set") {
			name, value, err := parseSetEnv(line)
			if err != nil {
				return nil, err
			}
			env[name] = value
			continue
		}

		if strings.HasPrefix(line, "#build") {
			name, err := parseBuildName(line)
			if err != nil {
				return nil, err
			}
			target.Name = name
			continue
		}

		if strings.HasPrefix(line, "#root") {
			root, err := parseBuildRoot(line)
			if err != nil {
				return nil, err
			}
			target.RootSourceFile = root
			continue
		}

		if strings.HasPrefix(line, "#link") {
			link, err := parseBuildLink(line)
			if err != nil {
				return nil, err
			}
			target.Links = append(target.Links, link)
			continue
		}

		return nil, fmt.Errorf("bad line: \"%s\"", line)
	}
	err = scanner.Err()
	if err != nil {
		return nil, err
	}

	return &BuildPlan{
		Targets: targets,
		Env:     env,
	}, nil
}

func parseBuildName(line string) (string, error) {
	fields := strings.Fields(line)
	if len(fields) < 2 {
		return "", errors.New("bad build format")
	}
	name := fields[1]
	if name == "" {
		return "", errors.New("empty build name")
	}

	return name, nil
}

func parseBuildRoot(line string) (string, error) {
	fields := strings.Fields(line)
	if len(fields) < 2 {
		return "", errors.New("bad root format")
	}
	root := fields[1]
	if root == "" {
		return "", errors.New("empty root name")
	}

	return root, nil
}

func parseBuildLink(line string) (string, error) {
	fields := strings.Fields(line)
	if len(fields) < 2 {
		return "", errors.New("bad link format")
	}
	link := fields[1]
	if link == "" {
		return "", errors.New("empty link name")
	}

	return link, nil
}

func parseSetEnv(line string) (name string, value string, err error) {
	fields := strings.Fields(line)
	if len(fields) < 4 {
		return "", "", errors.New("bad set format")
	}
	name = fields[1]
	value = fields[3]

	if name == "" {
		return "", "", errors.New("empty env name")
	}
	return name, value, nil
}
