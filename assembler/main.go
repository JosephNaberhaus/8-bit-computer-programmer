package main

import (
	"bytes"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"regexp"
	"strconv"
	"strings"
	"unsafe"
)

var inputPath = flag.String("input", "", "input assembly file")
var outputPath = flag.String("output", "", "output binary file")

func repeat(value string, times int) []string {
	result := make([]string, 0, times)
	for i := 0; i < times; i++ {
		result = append(result, value)
	}

	return result
}

func main() {
	flag.Parse()

	if *inputPath == "" {
		panic("no input specified")
	}

	inputFile, err := os.Open(*inputPath)
	if err != nil {
		panic(err)
	}

	input, err := io.ReadAll(inputFile)
	if err != nil {
		panic(err)
	}

	lines := strings.Split(string(input), "\n")
	if len(lines) > 255 {
		panic("too many lines")
	}

	lines = preProcess(lines)
	lines = append(lines, repeat("JMP $i 255", 256-len(lines))...)

	var output bytes.Buffer
	for _, line := range lines {
		result, err := assembleLine(line)
		if err != nil {
			panic("invalid instruction: " + line + "\n" + err.Error())
		}

		if len(result) != 3 {
			panic("unexpected result length")
		}

		for _, b := range result {
			output.WriteString(fmt.Sprintf("0b%s,\n", strconv.FormatInt(int64(b), 2)))
		}
	}

	if *outputPath == "" {
		panic("no output specified")
	}

	err = os.WriteFile(*outputPath, output.Bytes(), os.ModePerm)
	if err != nil {
		panic(err)
	}
}

const register1 = "$1"
const register2 = "$2"
const register3 = "$3"
const register4 = "$4"
const immediate = "$i"

func preProcess(lines []string) []string {
	labelMatcher := regexp.MustCompile(`^\s*([a-zA-Z_]+):\s*$`)

	labelToLine := map[string]int{}

	var result []string
	for len(lines) > 0 {
		curLine := lines[0]
		lines = lines[1:]

		if strings.TrimSpace(curLine) == "" {
			continue
		}

		match := labelMatcher.FindStringSubmatch(curLine)
		if len(match) == 2 {
			labelToLine[match[1]] = len(result)
		} else if !strings.HasPrefix(curLine, "//") {
			result = append(result, curLine)
		}
	}

	for i := range result {
		for label, lineNumber := range labelToLine {
			result[i] = strings.ReplaceAll(result[i], label, strconv.FormatInt(int64(lineNumber), 10))
		}
	}

	return result
}

func assembleLine(line string) ([]byte, error) {
	parts := strings.Split(line, " ")
	if len(parts) == 0 {
		return nil, nil
	}

	switch parts[0] {
	case "SUB":
		return assembleSubtract(parts[1:])
	case "ADD":
		return assembleAdd(parts[1:])
	case "AND":
		return assembleAnd(parts[1:])
	case "OR":
		return assembleOr(parts[1:])
	case "LT":
		return assembleUnsignedLessThan(parts[1:])
	case "SLT":
		return assembleSignedLessThan(parts[1:])
	case "LTEQ":
		return assembleUnsignedLessThanOrEqual(parts[1:])
	case "SLTEQ":
		return assembleSignedLessThanOrEqual(parts[1:])
	case "NEQ":
		return assembleNotEqual(parts[1:])
	case "EQ":
		return assembleEqual(parts[1:])
	case "MV":
		return assembleMove(parts[1:])
	case "NOT":
		return assembleNot(parts[1:])
	case "LOAD":
		return assembleLoad(parts[1:])
	case "SHIFTL":
		return assembleShiftLeft(parts[1:])
	case "SHIFTR":
		return assembleShiftRight(parts[1:])
	case "JMP":
		return assembleJump(parts[1:])
	case "JMPNZ":
		return assembleJumpIFNotZero(parts[1:])
	case "STORE":
		return assembleStore(parts[1:])
	default:
		return nil, errors.New("unkown instruction")
	}
}

func parseImmediate(part string) (byte, error) {
	isSigned := false
	if part[0] == '-' || part[0] == '+' {
		isSigned = true
		part = part[1:]
	}

	var value byte
	if isSigned {
		parsedValue, err := strconv.ParseInt(part, 10, 8)
		if err != nil {
			return 0, err
		}

		value = *(*byte)(unsafe.Pointer(&parsedValue))
	} else {
		parsedValue, err := strconv.ParseUint(part, 10, 8)
		if err != nil {
			return 0, err
		}

		value = byte(parsedValue)
	}

	return value, nil
}

func assembleUnaryIO(parts []string) (byte, byte, error) {
	if len(parts) < 2 || len(parts) > 3 {
		return 0, 0, errors.New("unexpected number of parts")
	}

	var control byte

	// Input register
	switch parts[0] {
	case register1:
		control |= 0b00000001
	case register2:
		control |= 0b00000101
	case register3:
		control |= 0b00001001
	case register4:
		control |= 0b00001101
	case immediate:
		control |= 0b00000000
	default:
		return 0, 0, errors.New("invalid input register selection")
	}

	// Result register
	switch parts[1] {
	case register1:
		control |= 0b00000000
	case register2:
		control |= 0b01000000
	case register3:
		control |= 0b10000000
	case register4:
		control |= 0b11000000
	default:
		return 0, 0, errors.New("invalid result register selection")
	}

	var immediateValue byte
	if len(parts) == 3 {
		value, err := parseImmediate(parts[2])
		if err != nil {
			return 0, 0, err
		}

		immediateValue = value
	}

	return control, immediateValue, nil
}

func assembleUnaryI(parts []string) (byte, byte, error) {
	if len(parts) < 1 || len(parts) > 2 {
		return 0, 0, errors.New("unexpected number of parts")
	}

	var control byte

	// Input register
	switch parts[0] {
	case register1:
		control |= 0b00000010
	case register2:
		control |= 0b00000110
	case register3:
		control |= 0b00001010
	case register4:
		control |= 0b00001110
	case immediate:
		control |= 0b00000000
	default:
		return 0, 0, errors.New("invalid input register selection")
	}

	var immediateValue byte
	if len(parts) == 2 {
		value, err := parseImmediate(parts[1])
		if err != nil {
			return 0, 0, err
		}

		immediateValue = value
	}

	return control, immediateValue, nil
}

func assembleBinaryIO(parts []string) (byte, byte, error) {
	if len(parts) < 3 || len(parts) > 4 {
		return 0, 0, errors.New("unexpected number of parts")
	}

	var control byte

	// First register
	switch parts[0] {
	case register1:
		control |= 0b00000001
	case register2:
		control |= 0b00000101
	case register3:
		control |= 0b00001001
	case register4:
		control |= 0b00001101
	case immediate:
		control |= 0b00000000
	default:
		return 0, 0, errors.New("invalid first input register selection")
	}

	// Second register
	switch parts[1] {
	case register1:
		control |= 0b00000010
	case register2:
		control |= 0b00010010
	case register3:
		control |= 0b00100010
	case register4:
		control |= 0b00110010
	case immediate:
		control |= 0b00000000
	default:
		return 0, 0, errors.New("invalid second input register selection")
	}

	// Result register
	switch parts[2] {
	case register1:
		control |= 0b00000000
	case register2:
		control |= 0b01000000
	case register3:
		control |= 0b10000000
	case register4:
		control |= 0b11000000
	default:
		return 0, 0, errors.New("invalid result register selection")
	}

	var immediateValue byte
	if len(parts) == 4 {
		value, err := parseImmediate(parts[3])
		if err != nil {
			return 0, 0, err
		}

		immediateValue = value
	}

	return control, immediateValue, nil
}

func assembleBinaryI(parts []string) (byte, byte, error) {
	if len(parts) < 2 || len(parts) > 3 {
		return 0, 0, errors.New("unexpected number of parts")
	}

	var control byte

	// First register
	switch parts[0] {
	case register1:
		control |= 0b00000001
	case register2:
		control |= 0b00000101
	case register3:
		control |= 0b00001001
	case register4:
		control |= 0b00001101
	case immediate:
		control |= 0b00000000
	default:
		return 0, 0, errors.New("invalid first input register selection")
	}

	// Second register
	switch parts[1] {
	case register1:
		control |= 0b00000010
	case register2:
		control |= 0b00010010
	case register3:
		control |= 0b00100010
	case register4:
		control |= 0b00110010
	case immediate:
		control |= 0b00000000
	default:
		return 0, 0, errors.New("invalid second input register selection")
	}

	var immediateValue byte
	if len(parts) == 3 {
		value, err := parseImmediate(parts[2])
		if err != nil {
			return 0, 0, err
		}

		immediateValue = value
	}

	return control, immediateValue, nil
}

func assembleSubtract(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b10000100, control, immediate}, nil
}

func assembleAdd(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b11000100, control, immediate}, nil
}

func assembleAnd(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b10100100, control, immediate}, nil
}

func assembleOr(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b11100100, control, immediate}, nil
}

func assembleUnsignedLessThan(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b10010100, control, immediate}, nil
}

func assembleSignedLessThan(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b10011100, control, immediate}, nil
}

func assembleUnsignedLessThanOrEqual(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b11010100, control, immediate}, nil
}

func assembleSignedLessThanOrEqual(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b11011100, control, immediate}, nil
}

func assembleNotEqual(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b10110100, control, immediate}, nil
}

func assembleEqual(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b11110100, control, immediate}, nil
}

func assembleMove(parts []string) ([]byte, error) {
	control, immediate, err := assembleUnaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b10000010, control, immediate}, nil
}

func assembleNot(parts []string) ([]byte, error) {
	control, immediate, err := assembleUnaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b11000010, control, immediate}, nil
}

func assembleLoad(parts []string) ([]byte, error) {
	control, immediate, err := assembleUnaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b10000000, control, immediate}, nil
}

func assembleShiftLeft(parts []string) ([]byte, error) {
	control, immediate, err := assembleUnaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b10100010, control, immediate}, nil
}

func assembleShiftRight(parts []string) ([]byte, error) {
	control, immediate, err := assembleUnaryIO(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b11100010, control, immediate}, nil
}

func assembleJump(parts []string) ([]byte, error) {
	control, immediate, err := assembleUnaryI(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b00000001, control, immediate}, nil
}

func assembleJumpIFNotZero(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryI(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b01000001, control, immediate}, nil
}

func assembleStore(parts []string) ([]byte, error) {
	control, immediate, err := assembleBinaryI(parts)
	if err != nil {
		return nil, err
	}

	return []byte{0b00010000, control, immediate}, nil
}
