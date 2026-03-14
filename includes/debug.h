/*

* This header file is used to control the debug output of the project. It defines a macro DEBUG that can be set to 1 to enable debug output or 0 to disable it.

* If the compiler does not already know what DEBUG is, the header defines it as 0.
* If the compiler already received DEBUG, the header leaves it alone.

* debug.h is just a fallback default. 
* The actual debug output is controlled by the CMakeLists.txt file, which can set the DEBUG macro to 1 or 0 when compiling the project. 
* This allows for flexible control over debug output without modifying the source code directly.

*/

#pragma once

#ifndef DEBUG
#define DEBUG 0 // Set to 1 to enable debug output, 0 to disable
#endif