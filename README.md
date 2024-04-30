# Interprocess Variables (C++)

IPV stands for Interprocess Variable. 
It is a library that allows you to share variables between different processes. 
Written in C++, it supports both Linux and Windows platforms thanks to the Boost library.

## License
The library is released under the  Mozilla Public License 2.0 (MPL 2.0). 
https://www.mozilla.org/en-US/MPL/2.0/

## Rationale

In monolithic programs, global variables can be used to share data between functions.
However, this becomes challenging in microservices, parallel programming, or when dealing with multiple DLLs or shared libraries.

There are scenarios where accessing or modifying a variable from a different process or command line is necessary, such as retrieving statistics from a running process or altering a configuration parameter.

The ipvar library provides a simple way to share variables between processes.

## How it works
IPV uses Boost's shared memory to create a shared memory segment and provides macros to simplify the creation of variables in this segment. An ipv::variable can be accessed by dereferencing its pointer.

~~~
// Create a shared variable of type std::atomic<int> named myCounter.
decl_ipv_variable(std::atomic<int>, myCounter);

// Assign a value to the variable
*myCounter = 5;

// Access the value of the variable
std::cout << *myCounter << std::endl;

// Increment the value of the variable
(*myCounter)++;

// Check if the variable has been created by the current thread
bool isMine = myCounter.isMine();
~~~


## Few words about ipv::variable

The variable is stored in shared memory, allowing access by different processes. A thread can determine if it created the variable.
This can be usefull for performing a specific action on variable creation or for designating the current thread/process as the master, with others as consumers.

By default, IPV variables are non-persistent; they are destroyed when the last instance is destroyed. To make a variable persistent, set the last parameter of the ipv::variable constructor to true or use the decl_pipv_variable macro.


~~~
	// Create a persistent shared variable of type std::atomic<int> named myCounter and intialize it to 25.
	decl_pipv_variable_3(std::atomic<int>, myCounter, "Incrementing counter", 25);
~~~



IPV supports any variable type. However, handling pointers or variables requiring memory allocation (like std::string, std::vector, std::map) is the user's responsibility. The SharedMemoryManager singleton provides methods for memory allocation and deallocation in the shared memory segment.
The segment manager is also exposed. You can use it to create the allocators for complex data structures.
Use the following functions to allocate and deallocate memory in the shared memory segment, and to get access to the shared_memory_segment.

~~~
void* ipvar::allocate_shared_memory(std::size_t size);
void ipvar::deallocate_shared_memory(void* ptr);
_shared_memory_ * ipvar::get_shared_memory_segment();

// _shared_memory_  will be a boost::interprocess::managed_shared_memory object or a boost::interprocess::managed_windows_shared_memory object depending on the platform.
~~~


## IPV lifetime
Non-persistent IPV variables are created when their first instance is created and are destroyed when the last instance is destroyed.

consider this example:
~~~
{
  {
    decl_ipv_variable(std::atomic<int>, myCounter);
    decl_pipv_variable(std::atomic<int>, myPersistentCounter);
    *myCounter = 5;
    *myPersistentCounter = 10;
  }
  // myCounter is destroyed
  // myPersistentCounter is destroyed, but the value is saved in the shared memory segment
  decl_ipv_variable(std::atomic<int>, myCounter);
  decl_pipv_variable(std::atomic<int>, myPersistentCounter);
  std::cout << "myCounter = " << *myCounter << " myPersistentCounter = " << *myPersistentCounter << std::endl;
}

This will output:
myCounter = 0 myPersistentCounter = 10

As you can see, the value of myPersistentCounter is saved in the shared memory segment, and is available in the next instance of the variable.

~~~

Persistent IPV variables are created when the first instance of the variable is created, and are destroyed when system is reset (reboot / poweroff) on linux.
On windows, persistent variables will remain until the last program that uses an ipv::variable is terminated.

You can query if a variable is persistent by calling the isPersistent() method of the ipv::variable class.

the decl_ macros have two versions:  decl_ipv_... and decl_pipv_... for non persistent and persistent variables respectively.

the decl_ipv_variable_3 and decl_pipv_variable_3 macros allow you to set the initial value of the variable.


## Security considerations
Exposing variables to the outside world can pose a security risk. The library does not provide any security mechanism. 
You should use the library in a secure environment.

You're strongly advised to use your own segment name. This can be done defining IPV_SHARED_MEMORY_NAME in the compiler options.


## Dependencies
The library has a dependency on the boost library.  Tested with boost 1.81, but should work with previous versions. 
It is recommended to use the latest version of boost.

# Usage 

To use ipv, you must include the header file ipvar.h in your cpp file. 
The additional ipv_utils.h file is optional, and provides some utility functions and macros.
It eases the creation of variables.


The macros for creating variables are defined in ipv_macros.h
decl_ipv_variable (_type, _name) : creates a non persistent variable  of type _type named _name
decl_pipv_variable (_type, _name) : creates a persistent variable of type _type named _name

decl_ipv_variable_2(_type, _name, _descr) : Add a description to the non persistent variable. 
decl_pipv_variable_2(_type, _name, _descr) : Add a description to the persistent variable. 

decl_ipv_variable_3(_type, _name, _descr, _init) : Stores the initial value into the created variable. 
decl_pipv_variable_3(_type, _name, _descr, _init) : Stores the initial value into the created persistent variable. 


## Example
These examples show how to use a shared variables for controling a process from another one.
The first program simulates different logging messages, and loops until oredered to stop.
The second program changes the logging level of the first program or instructs it to stop by setting the stopRunning boolean.

~~~
// Program 1
#include <thread>
#include <iostream>
#include <chrono>
#include "ipvar.h"
#include "ipvar_util.h"

int main(int, char**)
{
    decl_ipv_variable_3(std::atomic<int>, loggerLevel, "Logger level (TRACE=0.. ERROR=4)", 0);
    decl_ipv_variable_3(bool, stopRunning, "Stop Running the demo", false);

    while (!*stopRunning)
    {
        if (*loggerLevel <= 0) std::cout << "TRACE: This is a trace message" << std::endl;
        if (*loggerLevel <= 1) std::cout << "DEBUG: This is a debug message" << std::endl;
        if (*loggerLevel <= 2) std::cout << "INFO: This is an info message" << std::endl;
        if (*loggerLevel <= 3) std::cout << "WARN: This is a warning message" << std::endl;
        if (*loggerLevel <= 4) std::cout << "ERROR: This is an error message" << std::endl;
        std::cout << "-----------------------------------\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Stop instruction received" << std::endl;
    return 0;
}
~~~


~~~
// Program 2
#include "ipvar.h"
#include <thread>
#include <iostream>
#include <chrono>
int main(int argc,char ** argv)
{
  decl_ipv_variable_3(int, loggerLevel,"Logger level (TRACE=0.. ERROR=4)",0);
  decl_ipv_variable_3(bool, stopRunning,"Stop Running the demo",false);

  if (argc !=2)   {
	std::cout << "Usage: " << argv[0] << " <logger level>  to change logging level (0: TRACE .. 4:ERROR)" << std::endl;
    std::cout << "Usage: " << argv[0] << " stop" << std::endl;
	return 1;
  }
  // set the logger level
  if (std::string(argv[1]) == "stop")  { 
     *stopRunning = true;
     std::cout << "Stopping the logger" << std::endl;
  } else {
    std::cout << "Setting logger level to " << argv[1] << std::endl;
    *loggerLevel = std::stoi(argv[1]);
  }

  return 0;
}
~~~

## Installation
The library is header only, so you can simply copy the header files in your project, and include them in your source files.
Alternatively, you can install the library in a specific folder and add that folder to your include directories. Ensure the Boost library is installed on your system.

## Contact
For any question, please leave a message in the github forum.