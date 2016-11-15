cd $GEM_DQM_ROOT_BASE
source bin/thisroot.sh
cd -
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/gcc/4.9.3/lib64
export PATH=$PATH:$BUILD_HOME/gem-light-dqm/dqm-root/bin/linux/x86_64_slc6:$BUILD_HOME/gem-light-dqm/gemtreewriter/bin/linux/x86_64_slc6
