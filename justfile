assemble input:
    go run assembler/main.go --input {{input}} --output components/program/assembled.txt

build:
    idf.py build

clean:
    idf.py fullclean

flash: build
    idf.py flash

monitor:
    idf.py monitor

