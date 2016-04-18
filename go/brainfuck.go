package main

import (
	"bufio"
	"fmt"
	"io"
	"io/ioutil"
	"os"
)

func eval(r io.Reader, i io.Reader, w io.Writer) error {
	prog, err := ioutil.ReadAll(r)
	if err != nil {
		return err
	}
	input := bufio.NewReader(i) // buffered reader for `,` requests

	var (
		fpos uint   = 0                  // file position
		dpos uint   = 0                  // data position
		dpth uint   = 1                  // scope depth - for `[` and `]`
		size uint   = 30000              // size of data card
		plen uint   = uint(len(prog))    // programme length
		data []byte = make([]byte, size) // data card with `size` items
	)

	for fpos < plen {
		switch prog[fpos] {
		case '+': // increment at current position
			data[dpos]++
		case '-': // decrement at current position
			data[dpos]--
		case '>': // move to next position
			if dpos == size-1 {
				dpos = 0
			} else {
				dpos++
			}
		case '<': // move to previous position
			if dpos == 0 {
				dpos = size - 1
			} else {
				dpos--
			}
		case '.': // output value of current position
			fmt.Fprintf(w, "%c", data[dpos])
		case ',': // read value into current position
			if data[dpos], err = input.ReadByte(); err != nil {
				os.Exit(0)
			}
		case '[': // if current position is false, skip to ]
			if data[dpos] == 0 {
				for { // skip forward until as same scope depth
					fpos++
					if prog[fpos] == '[' {
						dpth++
					} else if prog[fpos] == ']' {
						dpth--
					}
					if dpth == 0 {
						break
					}
				}
				dpth = 1 // reset scope depth
			}
		case ']': // if at current position true, return to [
			if data[dpos] != 0 {
				for { // move back until at same scope depth
					fpos--
					if prog[fpos] == ']' {
						dpth++
					} else if prog[fpos] == '[' {
						dpth--
					}
					if dpth == 0 {
						break
					}
				}
			}
			dpth = 1 // reset scope depth
		}
		fpos++
	}
	return nil
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "usage: %s [file.bf]\n", os.Args[0])
		os.Exit(3)
	}

	r, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		os.Exit(2)
	}

	err = eval(r, os.Stdin, os.Stdout)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		os.Exit(1)
	}
}
