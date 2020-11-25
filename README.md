# QuickSort
The following project parallelises the Quicksort algorithm using the C programming language. The purpose of this project was to determine the most efficient method of parallelising the Quicksort Algorithm.

## Prerequisites
In order to run this project (and its individual files) you must be using a Unix based operating system. Otherwise, you will not be able to run the different versions of quicksort.

## Running the Program on Command Line
As this project has different implementations of the Quicksort Alogrithm, the terminal commands will vary accordingly.

To compile file that does <b>NOT</> implement pthread:
```

cc -O2 filename.c -o filename
```

To complie a file that does implement pthread:
```

cc -O2 filename.c -o filename -pthread
```

Finally, to run the program run the following command (Note: the 1000000 can be adjusted):
```

time ./filename 1000000
```
