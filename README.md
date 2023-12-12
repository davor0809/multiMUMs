# multiMUMs

This repository includes programs used in a bachelor-thesis.

# Prerequisites

To run the code in this repository, one must install the [sdsl-lite](https://github.com/simongog/sdsl-lite)-library.
The path to the sdsl-library must then be set in the [Makefile](/programs/Makefile) by setting `SDSL = $PATH_TO_LIBRARY`.

# Datasets

All datasets lie in [this](/programs/data) folder.

# Running a program

In order to run a program, one must first comment out the algorithm he wants to use in [comparer.cpp](/programs/comparer.cpp).
Beware that only one algorithm should be commented out at a time.
To compile the program, simply enter:
```bash
make comparer
```
The program expects the dataset to lie in /programs/input, therefore you should first copy the dataset you want to use to /programs/input.
To run the program:
```bash
./comparer
```
