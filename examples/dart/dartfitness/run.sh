cd /home/cody/taas/pladapt/examples/dart/dartfitness

LD_LIBRARY_PATH=/usr/local/lib

LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/cody/taas/pladapt/examples/dart/dartam/build/src/.libs
export LD_LIBRARY_PATH

/home/cody/taas/pladapt/examples/dart/dartfitness/dartfitness $*
