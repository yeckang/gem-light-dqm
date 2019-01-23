# gem-light-dqm
GEM light DQM code

To compile: 
```bash
> cd /path/to/gem-light-dqm
> export BUILD_HOME=$PWD/../
> sh compile.sh
```
Binary executables will be produced here:
```bash
./gemtreewriter/bin/${XDAQ_OS}/${XDAQ_PLATFORM}/unpacker
./dqm-root/bin/${XDAQ_OS}/${XDAQ_PLATFORM}/dqm
```
Guaranteed to work at CC7 CERN PCs
