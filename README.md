Description
=======
The code for Differentially Private Set Intersection (DiPSI) implemented in HELib (version from January 2019). 
This code implements and benchmarks the PSI as described in the Paper (link: tbd).

Our contributed source code can be found in the following files:

----

Our Parameters: src/params.h

Client/Server: src/PSIClient.*, src/PSIServer.*

Hashing: src/FixedBinningHash.*, src/HashingStrategy.*

PSI helper functions: /src/PSI.*

Main: src/PSImain.*

----




Setup for HELib:
=======

Get Anaconda (https://www.anaconda.com/distribution/#linux) and set up a conda environment, install Armadillo (https://anaconda.org/conda-forge/armadillo) and NTL 10.3.0 (https://anaconda.org/conda-forge/ntl)

0. Clone this repository 
``git clone https://github.com/PSISubmission/PSISubmission``
1. In the ``./CMakeLists.txt`` change the path of the NTL include and lib library to your paths i.e. the path given 
by the conda environment.
2. Run the cmake 
command in the PSI_HELib folder to compile ``./CMakeLists.txt``
3. (Optional) CHange parameters in the ``src/params.h`` file
3. Run ``cd src && make`` command, which creates an executable in the ``bin``
folder. 
4. Run the benchmarks ``bin/do_benchmarks.sh`` (may take long time)

