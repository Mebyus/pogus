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
	plan, err := readBuildTargets("build.claw")
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	fmt.Println(plan.Env)
	for _, t := range plan.Targets {
		fmt.Println(t)
		err = buildTarget(&t, plan.Env)
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

func buildTarget(t *BuildTarget, env map[string]string) error {
	dir := filepath.Join("build", t.Name)
	err := os.MkdirAll(dir, 0o755)
	if err != nil {
		return err
	}

	outpath := filepath.Join(dir, t.Name)
	source := filepath.Join(sourceDir, t.RootSourceFile)

	return compileExe(&CompileExeSpec{
		SourceFile: source,
		OutPath:    outpath,
		Links:      t.Links,
		LinkSearch: env["link.dir"],
	})
}

type CompileExeSpec struct {
	SourceFile string
	OutPath    string
	LinkSearch string

	Links []string
}

func compileExe(spec *CompileExeSpec) error {
	var args []string
	args = append(args, codegenFlags...)
	args = append(args, maxCompilerErrorsFlag)
	args = append(args, warningFlags...)
	args = append(args, otherFlags...)
	args = append(args, "-Og", "-ggdb")

	args = append(args, "-Isrc")

	if spec.LinkSearch != "" {
		args = append(args, "-L"+spec.LinkSearch)
	}

	args = append(args, "-o", spec.OutPath)
	// args = append(args, "-c", source)
	args = append(args, spec.SourceFile)

	for _, link := range spec.Links {
		args = append(args, "-l"+link)
	}

	return invokeCompiler(args)
}

func invokeCompiler(args []string) error {
	cmd := exec.Command(compiler, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
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
		return "", "",  errors.New("bad set format")
	}
	name = fields[1]
	value = fields[3]

	if name == "" {
		return "", "", errors.New("empty env name")
	}
	return name, value, nil
}
