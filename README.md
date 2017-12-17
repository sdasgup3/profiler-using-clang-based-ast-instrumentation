# About
These projects concern about collecting profiles (run-times and execution
  frequencies) on C/C++ programs.

# Projects
## instrumentor
 Clang based AST reformatting tool used for injecting instrumentation code in
 C/C++ programs.
### Directory Structure
#### [tool](instrumentor/tool)
The actual tool.
#### [lib](instrumentor/lib) & [include](instrumentor/include)
The library code that need to be linked to the instrumented code and the
corresponding include files.
#### [unitTests](instrumentor/unitTests)
Some representative test-cases.

### Build the instrumentor
```
cd instrumentor
make
```

### Instrument C++ code
- For a single file
```
cd instrumentor
./build/instrumentor unitTests/Test2/prime.cpp -- > unitTests/Test2/prime_instr.cpp
```
- For a batch of C++ files in a project.
```
cd <The directory containing the C++ files>
scripts/batchinstrument.sh
```

### Build & Run the instrumented code
```
g++ -I ./instrumentor/include -L ./instrumentor/build unitTests/Test2/prime_instr.cpp -linstrumentor
./a.out
```
### Example Profile Output
For the [C++ code](unitTests/Test2/fibonacci.cpp), we have the following
profiles collected.
```
                Name      Self Time (ns)     Child Time (ns)
              ----------------------------------------------
     fibonacci_iter         2719.000000            0.000000
      fibonacci_rec      4285253.000000            0.000000
frequency_of_primes   1106886635.000000            0.000000
              main       8765207.000000   1111174607.000000
______________________________________________________________
```

### Run Tests

```
cd unitTests  
make
make clean
```

### Current Limitations (Could be avoided)
  - Not thread safe.
  - Works on  C++ code.
  - The collected run-times are wall-clock times, as apposed to user time.
