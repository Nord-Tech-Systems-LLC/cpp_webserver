#!/usr/bin/env make

SrcDir=src
IncludeDir=$(SrcDir)
BuildDir = build
BuildBinDir = $(BuildDir)/bin
BuildObjectsDir = $(BuildDir)/objects

CC = g++
CFlags = -I $(IncludeDir) -g


all: $(BuildBinDir)/main

prerequisites:
	@ mkdir -p $(BuildBinDir)
	@ mkdir -p $(BuildObjectsDir)

./build/bin/main: \
		$(BuildObjectsDir)/main.o \
		$(BuildObjectsDir)/http_server.o \
		$(BuildObjectsDir)/server_logging.o | prerequisites
		# $(BuildObjectsDir)/syntax_validation.o | prerequisites
	@ echo Building $@ from $^
	@ $(CC) -o $@ $^


$(BuildObjectsDir)/%.o: $(SrcDir)/%.cpp | prerequisites
	@ echo Building $@ from $<
	@ $(CC) $(CFlags) -c -o $@ $<
