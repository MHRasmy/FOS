# FOS

## Overview

FOS is an open-source operating system project that focuses on efficient memory management and CPU scheduling to optimize user-kernel interactions and process execution. It is designed to provide a flexible and extensible platform for various computing environments.

## Features

1. **Memory Management:** FOS employs advanced memory management techniques to efficiently allocate and deallocate memory resources. It ensures optimal utilization of the available memory while preventing memory leaks and fragmentation.

2. **CPU Scheduling:** The operating system implements a robust CPU scheduling algorithm to enhance the overall system performance. It prioritizes and allocates CPU time to processes based on their priority, ensuring fair and efficient utilization of the CPU.

3. **User-Kernel Interactions:** FOS aims to provide a seamless and optimized interface between user applications and the kernel. This enhances the overall user experience and system responsiveness.

4. **Process Execution:** FOS focuses on optimizing the execution of processes, balancing resource allocation, and ensuring timely execution. It includes mechanisms to handle process creation, termination, and synchronization efficiently.

## Getting Started

### Prerequisites

- Make sure you have a compatible development environment set up, including a C compiler and build tools.

### Building FOS

1. Clone the repository:

   ```bash
   git clone https://github.com/MHRasmy/FOS.git
2. Change directory to the project folder:

   ```bash
   cd FOS/boot

3. Build the operating system:

   ```bash
   gcc main.c -o fos

4. Run the FOS:
   
   ```bash
   ./fos

## Contributing

Contributions to the Simple Shell project are welcome! To contribute, please follow these steps:

1. Fork the repository.
2. Create a new branch with your changes: `git checkout -b <branch-name>`
3. Make your changes and test them thoroughly.
4. Commit your changes: `git commit -m "my commit message"`
5. Push your changes to your fork: `git push origin <branch-name>`
6. Create a pull request for your changes.
