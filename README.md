# clusterer_lite

A liteweight single-threaded implementation of the event-building for data driven hybrid pixel detectors based on the https://github.com/TomSpeedy/clusterer repository.

## target platform: zynq7000


# Prerequisities

* C++ compiler compatible with C++17 standard
* CMake version >= 2.8

# Installation

* Create the build directory and navigate to it, for instance:

  ```mkdir build && cd build```
* Create the generator files (e.g. Makefile):
  
  ```cmake ..```
* Compile and link
  
  ```cmake --build .```
* Use the executable located in build/bin directory

# Usage

* Run from terminal with three mandatory arguments:
  * processing option either -t or -b (for processing either burdaman text files or raw binary files respectively)   
  * path to the data file
  * path to the calibration folder containing the following configuration files:
    * a.txt
    * b.txt
    * c.txt
    * t.txt
* Example call:
  ```/path/to/executable/clusterer -b /path/to/data/file /path/to/calibration/folder```
* The output is written to the original file location
* The name of the output file is created in the following steps
  * Any suffix after '.' symbol is removed
  * New suffix '_clustered' is added
  * Three files in the MM data file format are created by adding '.ini', '_cl.txt' and '_px.txt' suffixes
