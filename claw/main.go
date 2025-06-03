package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

var warningFlags = []string{
	"-Wall",
	"-Wextra",
	"-Wconversion",
	"-Wunreachable-code",
	"-Wshadow",
	"-Wundef",
	"-Wfloat-equal",
	"-Wformat=0",
	"-Wpointer-arith",
	"-Winit-self",
	"-Wuninitialized",
	"-Wduplicated-branches",
	"-Wduplicated-cond",
	"-Wdouble-promotion",
	"-Wnull-dereference",
	"-Wstrict-prototypes",
	// "-Wvla",
	"-Wpointer-sign",
	"-Wswitch-default",
	"-Wshadow=local",

	"-Wno-main",
	"-Wno-shadow",
	"-Wno-unused-parameter",
	"-Wno-unused-function",
	"-Wno-unused-const-variable",
}

var otherFlags = []string{
	"-Werror",
	"-pipe",
	"-fanalyzer",
}

var codegenFlags = []string{
	"-fwrapv",
	"-funsigned-char",
	"-fno-asynchronous-unwind-tables",
	"-fomit-frame-pointer",
	// "-fno-stack-protector",
}

const maxCompilerErrorsFlag = "-fmax-errors=1"

const compiler = "cc"
const sourceDir = "src"

func main() {
	targets, err := readBuildTargets("build.claw")
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	for _, t := range targets {
		fmt.Println(t)
		err = buildTarget(&t)
		if err != nil {
			fmt.Fprintf(os.Stderr, "build \"%s\" target: %s\n", t.Name, err)
			os.Exit(1)
		}
	}

	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

func buildTarget(t *BuildTarget) error {
	dir := filepath.Join("build", t.Name)
	err := os.MkdirAll(dir, 0o755)
	if err != nil {
		return err
	}

	outpath := filepath.Join(dir, t.Name)
	source := filepath.Join(sourceDir, t.RootSourceFile)
	return compileExe(source, outpath, t.Links)
}

func compileExe(source string, outpath string, links []string) error {
	var args []string
	args = append(args, codegenFlags...)
	args = append(args, maxCompilerErrorsFlag)
	args = append(args, warningFlags...)
	args = append(args, otherFlags...)
	args = append(args, "-Og", "-ggdb")

	args = append(args, "-o", outpath)
	// args = append(args, "-c", source)
	args = append(args, source)

	for _, link := range links {
		args = append(args, "-l" + link)
	}

	return invokeCompiler(args)
}

func invokeCompiler(args []string) error {
	cmd := exec.Command(compiler, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

type BuildTarget struct {
	Links []string

	Name string

	RootSourceFile string
}

func readBuildTargets(path string) ([]BuildTarget, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}

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
			// TODO: implement setting variables
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

	return targets, nil
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
