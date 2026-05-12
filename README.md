# Multithreaded Producer–Consumer System with Deadlock Detection

## Operating Systems Course Project

Sakarya University – Software Engineering Department

### Students

* Faruk Turnalı – B231202060
* Ahmet Hilmi Güler – B231202013
* Hüseyin Yılmaz – B231202023

---------------------------------------------------------------------------------------------------------------------------------------------------

# Project Overview

This project implements a configurable multithreaded producer–consumer system using POSIX Threads (pthreads) in C.

The system simulates concurrent producer and consumer operations over shared bounded buffers while supporting synchronization, deadlock detection and performance evaluation.

The project was developed for the Operating Systems course project requirements. 

---------------------------------------------------------------------------------------------------------------------------------------------------

# Features

* Multithreaded producer–consumer architecture
* Shared bounded buffers
* Mutex and condition variable synchronization
* Deadlock detection using circular wait analysis
* Configurable runtime using configuration files
* Logging system
* Performance metric collection
* Multi-buffer pipeline support
* Workload experiment system

---------------------------------------------------------------------------------------------------------------------------------------------------

# Technologies Used

* C Programming Language
* POSIX Threads (pthreads)
* GCC Compiler
* Makefile

---------------------------------------------------------------------------------------------------------------------------------------------------

# Project Structure

OS-PRODUCER-CONSUMER-PROJECT/
├── Makefile
├── README.md
├── .gitignore
├── configs/
│   ├── config.txt
│   ├── experiment1_low_load.txt
│   ├── experiment2_high_load.txt
│   ├── experiment3_deadlock.txt
│   ├── experiment4_bottleneck.txt
│   └── experiment5_circular_dependency.txt
├── docs/
│   └── Report.pdf
└── src/
    ├── common/
    │   ├── utils.c
    │   └── utils.h
    ├── main.c
    ├── producer.c
    ├── producer.h
    ├── consumer.c
    ├── consumer.h
    ├── buffer.c
    ├── buffer.h
    ├── config.c
    └── config.h

---------------------------------------------------------------------------------------------------------------------------------------------------

# Compilation

Compile the project using:

make

Clean executable and object files:

make clean

---------------------------------------------------------------------------------------------------------------------------------------------------

# Running the Program

Run the application with a configuration file:

./main.exe configs/config.txt

Example:

./main.exe configs/experiment1_lowload.txt

---------------------------------------------------------------------------------------------------------------------------------------------------

# Configuration File Format

Example configuration:

A[22]
B[12]
t:60

P1>A
P2>A
P3>B

A>C1
B>C2
B>C3>A

P1:2
P2:3
P3:2

C1:2
C2:3
C3:2

---------------------------------------------------------------------------------------------------------------------------------------------------

# Experiments

The project includes five workload experiments:

| Experiment   | Description         |
| ------------ | ------------------- |
| Experiment 1 | Low workload        |
| Experiment 2 | High workload       |
| Experiment 3 | Artificial deadlock |
| Experiment 4 | Bottleneck scenario |
| Experiment 5 | Circular dependency |

---------------------------------------------------------------------------------------------------------------------------------------------------

# Performance Metrics

The system measures:

* Throughput
* Average waiting times
* Thread blocking times
* Deadlock frequency

---------------------------------------------------------------------------------------------------------------------------------------------------

# Deadlock Detection

The project includes a deadlock monitoring system based on circular wait detection.

Experiment 3 intentionally creates a deadlock scenario between Buffer A and Buffer B in order to test the detection mechanism. 

---------------------------------------------------------------------------------------------------------------------------------------------------

# References

1. Operating System Concepts – Silberschatz, Galvin, Gagne
2. POSIX Threads Programming Documentation
3. Linux pthread Manual Pages
4. Course lecture notes
