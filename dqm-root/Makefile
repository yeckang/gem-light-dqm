#
# Makefile for gemtreewriter package
#
BUILD_HOME:=$(shell pwd)/../..

CC=g++-4.9.3
CCFLAGS=-O2 -Wall -fPIC -pthread -m64
ADDFLAGS=-g -std=c++11
RC=rootcint
LBL=-shared -O2 -m64
LS=ls -lartF

# ROOT Config
ROOTCFLAGS=$(shell root-config --cflags)
ROOTLIBS  =$(shell root-config --libs) 
ROOTGLIBS =$(shell root-config --glibs) 

ROOTLIBS += -lRHTTP

PYTHONCFLAGS=$(shell python2.7-config --cflags) 
PYTHONLIBS=$(shell python2.7-config --ldflags)
MYSQLLIBS=$(shell mysql_config --libs)


#Project=gem
Project=gem-light-dqm
Package=dqm-root
LongPackage=dqm-root
ShortPackage=dqm-root
PackageName=dqm-root

IncludeDirs = /usr/include/root
IncludeDirs+= /usr/local/include/root
IncludeDirs+= $(BUILD_HOME)/$(Project)
IncludeDirs+= $(BUILD_HOME)/$(Project)/$(Package)
IncludeDirs+= $(BUILD_HOME)/$(Project)/$(Package)/src/common
IncludeDirs+= $(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+= $(BUILD_HOME)/$(Project)/gemtreewriter/include
IncludeDirs+= ${BUILD_HOME}/${GEM_OS_PROJECT}/gemreadout/include
IncludeDirs+= /usr/local/include/python2.7
#IncludeDirs+= /usr/local/lib/python2.7


INC=$(IncludeDirs:%=-I%)

SRC=$(BUILD_HOME)/$(Project)/$(Package)/src/common
OBJ=$(BUILD_HOME)/$(Project)/$(Package)/src/obj
DIC=$(BUILD_HOME)/$(Project)/$(Package)/src/dic
BIN=$(BUILD_HOME)/$(Project)/$(Package)/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)

LIB=$(BUILD_HOME)/$(Project)/gemtreewriter/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
all:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/histogramLayeringExample.cxx $(LIB)/libEvent.so -o $(BIN)/layer
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/gtprinterMain.cxx $(LIB)/libEvent.so -o $(BIN)/gtprinter
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/onlinePrinterMain.cxx $(LIB)/libEvent.so -o $(BIN)/onlineprinter
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/dqmMain.cxx $(LIB)/libEvent.so -o $(BIN)/dqm $(PYTHONLIBS) $(MYSQLLIBS)
	$(LS) $(BIN)

dqm:  	#CURRENT VERSION
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/dqmMain.cxx $(LIB)/libEvent.so -o $(BIN)/dqm $(PYTHONLIBS) $(MYSQLLIBS) -lProof -lProofPlayer

merger:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/mergeCleanTrees.cxx $(LIB)/libEvent.so -o $(BIN)/treemerge

layer:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/histogramLayeringExample.cxx $(LIB)/libEvent.so -o $(BIN)/layer
	$(LS) $(BIN)

printer:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/gtprinterMain.cxx $(LIB)/libEvent.so -o $(BIN)/gtprinter
	$(LS) $(BIN)


onlineprinter:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/onlinePrinterMain.cxx $(LIB)/libEvent.so -o $(BIN)/onlineprinter
	$(LS) $(BIN)


clean:
	rm -rf $(BIN)
print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo INC           $(INC)

