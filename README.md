# PhilosophersTeaTime

## Overview
PhilosophersTeaTime is a multithreaded C program simulating a variation of the classic Dining Philosophers problem, adapted to tea drinking. It showcases synchronization techniques using semaphores and pthreads in a Unix environment.

## Features
- **Multithreaded Simulation**: Simulates philosophers' actions including thinking, getting thirsty, pouring, and drinking tea.
- **Semaphore Synchronization**: Utilizes semaphores for managing access to shared resources like tea cups and sugar.
- **Dynamic Philosopher Count**: Supports a variable number of philosophers specified at runtime.

## Prerequisites
- Unix-like operating system
- GCC compiler with pthread support

## Compilation
Compile the program with:
```
gcc -o PhilosophersTeaTime df.c -lpthread
```

## Running the Program
Run the program by specifying the number of philosophers:
```
./PhilosophersTeaTime [number_of_philosophers]
```

## Contributing
Contributions to enhance the simulation or improve synchronization are welcome. Please adhere to standard coding practices.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
