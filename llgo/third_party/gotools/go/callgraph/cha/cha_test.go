// Copyright 2014 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package cha_test

import (
	"bytes"
	"fmt"
	"go/ast"
	"go/parser"
	"go/token"
	"io/ioutil"
	"sort"
	"strings"
	"testing"

	"llvm.org/llgo/third_party/gotools/go/callgraph"
	"llvm.org/llgo/third_party/gotools/go/callgraph/cha"
	"llvm.org/llgo/third_party/gotools/go/loader"
	"llvm.org/llgo/third_party/gotools/go/ssa"
	"llvm.org/llgo/third_party/gotools/go/types"
)

var inputs = []string{
	"testdata/func.go",
	"testdata/iface.go",
	"testdata/recv.go",
}

func expectation(f *ast.File) (string, token.Pos) {
	for _, c := range f.Comments {
		text := strings.TrimSpace(c.Text())
		if t := strings.TrimPrefix(text, "WANT:\n"); t != text {
			return t, c.Pos()
		}
	}
	return "", token.NoPos
}

// TestCHA runs CHA on each file in inputs, prints the dynamic edges of
// the call graph, and compares it with the golden results embedded in
// the WANT comment at the end of the file.
//
func TestCHA(t *testing.T) {
	for _, filename := range inputs {
		content, err := ioutil.ReadFile(filename)
		if err != nil {
			t.Errorf("couldn't read file '%s': %s", filename, err)
			continue
		}

		conf := loader.Config{
			SourceImports: true,
			ParserMode:    parser.ParseComments,
		}
		f, err := conf.ParseFile(filename, content)
		if err != nil {
			t.Error(err)
			continue
		}

		want, pos := expectation(f)
		if pos == token.NoPos {
			t.Errorf("No WANT: comment in %s", filename)
			continue
		}

		conf.CreateFromFiles("main", f)
		iprog, err := conf.Load()
		if err != nil {
			t.Error(err)
			continue
		}

		prog := ssa.Create(iprog, 0)
		mainPkg := prog.Package(iprog.Created[0].Pkg)
		prog.BuildAll()

		cg := cha.CallGraph(prog)

		if got := printGraph(cg, mainPkg.Object); got != want {
			t.Errorf("%s: got:\n%s\nwant:\n%s",
				prog.Fset.Position(pos), got, want)
		}
	}
}

func printGraph(cg *callgraph.Graph, from *types.Package) string {
	var edges []string
	callgraph.GraphVisitEdges(cg, func(e *callgraph.Edge) error {
		if strings.Contains(e.Description(), "dynamic") {
			edges = append(edges, fmt.Sprintf("%s --> %s",
				e.Caller.Func.RelString(from),
				e.Callee.Func.RelString(from)))
		}
		return nil
	})
	sort.Strings(edges)

	var buf bytes.Buffer
	buf.WriteString("Dynamic calls\n")
	for _, edge := range edges {
		fmt.Fprintf(&buf, "  %s\n", edge)
	}
	return strings.TrimSpace(buf.String())
}
