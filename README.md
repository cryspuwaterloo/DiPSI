This is the official repository  for our EuroSP 2020 paper on Differentially Private Set Intersection (DiPSI):
Paper Link: TBD

Abstract
======
Private set intersection (PSI) allows two parties to
compute the intersection of their data without revealing the
data they possess that is outside of the intersection. However,
in many cases of joint data analysis, the intersection is
also sensitive. We define differentially private set intersection
and we propose new protocols using (leveled) homomorphic
encryption where the result is differentially private. Our
circuit-based approach has an adaptability that allows us
to achieve differential privacy, as well as to compute predicates over the intersection such as cardinality. Furthermore,
our protocol produces differentially private output for set
intersection and set intersection cardinality that is optimal
in terms of communication and computation complexity. For
a client set of size m and a server set of size n, where m
is smaller than n, our communication complexity is O(m)
while previous circuit-based protocols only achieve O(n+m)
communication complexity. In addition to our asymptotic
optimizations which include new analysis for using nested
cuckoo hashing for PSI, we demonstrate the practicality
of our protocol through an implementation that shows the
feasibility of computing the differentially private intersection
for large data sets containing millions of elements.

Description
=======
The source code implements the for Differentially Private Set Intersection (DiPSI) using HELib (version from January 2019).

The most important files are the following

----

src/params.h: A list of all parameters for the DiPSI

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

