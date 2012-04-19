UNAME=$(shell uname)

ifeq (${UNAME},Darwin)
  BUILD=buildclang
else
  BUILD=buildgcc
endif

.PHONY: all

all: build

build: ${BUILD}

buildgcc:
	g++ -std=c++0x -Wall -Wextra main.cc -o main

buildclang:
	clang++ -std=c++11 -stdlib=libc++ -U__STRICT_ANSI__ -Wall -Wextra main.cc -o main

run:
	@./main
