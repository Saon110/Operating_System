# CSE314 - Operating Systems Sessional

This repository contains assignments and projects for the CSE314 Operating Systems Sessional course. The coursework covers fundamental operating system concepts including shell scripting, process management, threading, and system calls.

## 📁 Repository Structure

### 🔧 Offline 1: Shell Scripting

- **Location**: `Offline1/`
- **Description**: Shell scripting assignments focusing on file organization and automation
- **Key Files**:
  - `organize.sh` - Main organization script with various options
  - `task_a.sh` - Student submission file organizer by programming language
  - `task_b.sh` - Additional task implementation
  - `task_c.sh` - Additional task implementation
  - `Shell-Scripting-Assignment-Files/` - Test data and workspace

**Features**:

- Automated file organization by programming language (C, C++, Java, Python)
- Student submission processing from ZIP files
- Command-line options for verbose output and execution control
- Target directory structure creation

### 🧵 Offline 2: Process Management

- **Location**: `offline2/`
- **Description**: Advanced process management and scheduling algorithms
- **Files**: Assignment specification and implementation patches

### 🔄 Offline 3: Pthread Programming

- **Location**: `offline3/pthread_materials/`
- **Description**: Multi-threading concepts using POSIX threads
- **Key Components**:
  - `2105110.cpp` - Main threading assignment implementation
  - `poisson_random_number_generator.cpp` - Random number generation
  - `simple_sum_calculation.cpp` - Parallel sum calculation example
  - `student_report_printing.cpp` - Threading simulation with student reports
  - `prod_cons_with_mutex.cpp` - Producer-consumer with synchronization
  - `semaphore.c` - Semaphore implementation examples

**Threading Concepts Covered**:

- Thread creation and joining
- Mutex locks and semaphores
- Producer-consumer problems
- Synchronization mechanisms
- Shared resource management

### 🖥️ xv6 Operating System

- **Location**: `xv6/xv6-riscv/`
- **Description**: xv6 operating system modifications and system call implementations
- **Key Features**:
  - Custom system call implementations
  - Process information tracking (`pstat.h`)
  - History command functionality
  - Process testing utilities

**System Calls Implemented**:

- History tracking system call
- Process information retrieval
- Custom process management utilities

## 🚀 Getting Started

### Prerequisites

- Linux/Unix environment
- GCC compiler
- Make utility
- QEMU (for xv6)
- Bash shell

### Running Shell Scripts (Offline 1)

```bash
cd Offline1/
chmod +x organize.sh task_a.sh task_b.sh task_c.sh

# Run the main organization script
./organize.sh <submission_path> <target_path> <test_path> <answer_path> [-v] [-noexecute]

# Run task A (file organization)
./task_a.sh
```

### Compiling and Running Pthread Programs (Offline 3)

```bash
cd offline3/pthread_materials/2105110/

# Compile the main program
g++ -pthread 2105110.cpp -o assignment

# Run with input file
./assignment < input.txt

# Or use the provided run script
chmod +x run.sh
./run.sh
```

### Building and Running xv6

```bash
cd xv6/xv6-riscv/

# Build the operating system
make

# Run in QEMU
make qemu

# Test custom system calls
history
dummyproc
testprocinfo
```

## 📋 Assignment Details

### Shell Scripting Features

- **File Organization**: Automatically sorts student submissions by programming language
- **Batch Processing**: Handles multiple ZIP files containing student code
- **Error Handling**: Robust error checking and verbose output options
- **Testing Integration**: Supports automated testing frameworks

### Threading Assignment Features

- **Multi-station Simulation**: Simulates operatives working at different stations
- **Semaphore Management**: Uses semaphores for station access control
- **Reader-Writer Problem**: Implements reader-writer synchronization
- **Performance Metrics**: Tracks completion times and throughput

### xv6 System Call Features

- **History Command**: Tracks and displays command history
- **Process Information**: Custom system call for process statistics
- **Process Testing**: Utilities for testing process behavior

## 🎯 Learning Objectives

- **Shell Scripting**: Automation, file processing, and system administration
- **Process Management**: Understanding process lifecycle and scheduling
- **Threading**: Concurrent programming and synchronization
- **System Calls**: Low-level operating system interfaces
- **Operating System Design**: Kernel modification and system programming

## 📊 Grading and Evaluation

Each assignment includes:

- Implementation correctness
- Code quality and documentation
- Performance optimization
- Error handling
- Test case coverage

## 📝 Notes

- All assignments include comprehensive test cases
- Patch files contain the complete diff of changes made
- Student ID: 2105110 (as indicated by file naming)
- Course follows standard OS curriculum with hands-on implementation

## 🔗 Additional Resources

- xv6 Documentation: [MIT 6.1810](https://pdos.csail.mit.edu/6.1810/)
- POSIX Threads Tutorial
- Shell Scripting Best Practices
- Operating Systems Concepts (Silberschatz, Galvin, Gagne)

---

*This repository represents coursework for CSE314 Operating Systems Sessional, demonstrating practical implementation of core operating system concepts.*