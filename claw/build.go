package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
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
	// "-Wnull-dereference",
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
	// "-fanalyzer",
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

type BuildKind uint8

const (
	BuildDebug BuildKind = iota
	BuildTest
	BuildRelease
)

type BuildCommand struct {
	Target    string
	BuildKind BuildKind
}

func printBuildKind(kind BuildKind) {
	var s string
	switch kind {
	case BuildDebug:
		s = "debug"
	case BuildTest:
		s = "test"
	case BuildRelease:
		s = "release"
	}

	fmt.Printf("make [%s] build\n", s)
}

func ExecDefaultBuildCommand() error {
	return ExecBuildCommand(&BuildCommand{})
}

func ExecBuildCommand(c *BuildCommand) error {
	plan, err := readBuildTargets("build.claw")
	if err != nil {
		return err
	}
	printBuildEnv(plan.Env)

	fmt.Println()
	printBuildKind(c.BuildKind)

	fmt.Println()
	if c.Target != "" {
		return buildTargetByName(plan, c)
	}
	for _, t := range plan.Targets {
		err = buildTarget(&t, plan.Env, c.BuildKind)
		if err != nil {
			return fmt.Errorf("build \"%s\" target: %s", t.Name, err)
		}
	}
	return nil
}

func buildTargetByName(plan *BuildPlan, c *BuildCommand) error {
	for _, t := range plan.Targets {
		if t.Name == c.Target {
			return buildTarget(&t, plan.Env, c.BuildKind)
		}
	}
	return fmt.Errorf("unknown build target: %s", c.Target)
}

func buildTarget(t *BuildTarget, env map[string]string, buildKind BuildKind) error {
	fmt.Printf("build [%s]: (link %v)\n", t.Name, t.Links)

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
		BuildKind:  buildKind,
	})
}

type CompileExeSpec struct {
	SourceFile string
	OutPath    string
	LinkSearch string

	Links []string

	BuildKind BuildKind
}

func compileExe(spec *CompileExeSpec) error {
	var args []string
	args = append(args, codegenFlags...)
	args = append(args, maxCompilerErrorsFlag)
	args = append(args, warningFlags...)
	args = append(args, otherFlags...)

	if spec.BuildKind == BuildRelease {
		args = append(args, "-O2")
	} else {
		// default build is debug
		args = append(args, "-Og", "-ggdb")
	}

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
