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
	return compileExe(source, outpath)
}

func compileExe(source string, outpath string) error {
	var args []string
	args = append(args, codegenFlags...)
	args = append(args, maxCompilerErrorsFlag)
	args = append(args, warningFlags...)
	args = append(args, otherFlags...)
	args = append(args, "-Og", "-ggdb")

	args = append(args, "-o", outpath)
	// args = append(args, "-c", source)
	args = append(args, source)

	return invokeCompiler(args)
}

func invokeCompiler(args []string) error {
	cmd := exec.Command(compiler, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

type BuildTarget struct {
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
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}

		if strings.HasPrefix(line, "#set") {
			// TODO: implement setting variables
			continue
		}

		if strings.HasPrefix(line, "#build") {
			t, err := parseBuildTarget(line)
			if err != nil {
				return nil, err
			}
			targets = append(targets, t)
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

func parseBuildTarget(line string) (BuildTarget, error) {
	s := strings.TrimPrefix(line, "#build")
	name, src, ok := strings.Cut(s, "=")
	if !ok {
		return BuildTarget{}, errors.New("bad build format")
	}
	name = strings.TrimSpace(name)
	if name == "" {
		return BuildTarget{}, errors.New("empty build name")
	}
	src = strings.TrimSpace(src)
	if src == "" {
		return BuildTarget{}, errors.New("empty source file name")
	}

	return BuildTarget{
		Name:           name,
		RootSourceFile: src,
	}, nil
}
