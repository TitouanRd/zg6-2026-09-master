* [STM32 PROJECTS](#projects)
	* [Connexion to the target](#connect)
	* [Compiling the C project](#compile)
	* [Upload the program on the target and run/debug](#debug)
* [LIBC AND SEMIHOSTING](#libc_semihost)
	* [The C Library: libc](#libc)
	* [Semihosting](#semihosting)
* [SEGGER REAL TIME TRANSFER (RTT)](#rtt)
* [TROUBLE SHOOTNG](#trouble)

<a id="projects"></a>
# STM32 PROJECTS

The project structure is

```
 +- config/            linker script, openocd script
 |   +- ocd_st_nucleo_f411.cfg   openocd configuration script
 |   +- stm32f411re_flash.ld     linker script
 +- docs/              some doc
 +- include/
 |   +- cmsis/         ARM CMSIS core
 |   +- board.h        global peripheral pointers
 |   +- config.h       global project IO pin configuration
 |   +- stm32f411xe.h  peripheral register structure definition
 +- lib/               microcontroller peripheral drivers
 +- libext/            external libraries
 |   +- SEGGER/          SEGGER RTT Target support
 +- src/               Application src
 +- startup/           Startup code ()
 +- Makefile
 +- .dbginit           gdb configuration
 +- README.md
```

<a id="connect"></a>
## Connexion to the target

	ocd &

then choose the config file for the target, here,

	ocd_st_nucleo_f411.cfg
	
the push the "Connect to target" button. 

<a id="compile"></a>
## Compiling the C project

Compilation is automated though the `Makefile` description which lists the source files to be compiled, the different actions to be completed to get the final executable file `main.elf`.

	make clean
	make

You have to recompile the project each time the source files are modified. `make` reads the Makefile description to perform the update of modified files.

The provided `Makefile` supports several stage of development:

* debug stage: the project is in course of development. An IO terminal attached to the debugger is provided throug [semihosting](#semihosting). The code is not optimised.

		make dbg
		
	or
	
		make
		
* debug with optimisation. The IO feature is still there. The code is optimised. It is a usefull stage to make sure the program runs correctly (see the C keyword `volatile`).

		make opt
		
* release stage. No more IO terminal is provided. The code is optimised. The program is no more debuggable. The program must be ready for production at this stage.

		make release

<a id="debug"></a>
## Upload the program on the target and run/debug

	dbg main.elf
	

<a id="libc_semihost"></a>
# LIBC AND SEMIHOSTING

<a id="libc"></a>
## The C Library: libc

A library that provides standard functions to deal with differents aspects not supported by the C language itself

- print and get messages (print, scanf),
- file support (open, read, write, close),
- time functions (times, gettimeofday),
- string functions (strlen, ...)
- dynamic memory (malloc, realloc, free)
- ...

Two versions of the libc come alongside the compiler when you install the ARM GNU tools

- newlib: full standard C library
- newlib-nano: trimmed downed C library. Since the use of floating-point is not systematic in embedded applications, it provides by default no support for it in the printf and scanf functions. It is though possible to add the support

The choice between newlib and newlib-nano is made in the Makefile

	USE_NEWLIB_NANO = 1
	USE_NEWLIB_NANO_FLOAT_IO = 1

If the newlib-nano is chosen, it is then possible to add the support for floating-point.

<a id="semihosting"></a>
## Semihosting

By default, an embedded target doesn't have access to an Input/Output terminal. It is up to the developer to add support for one in the final application if it is needed.

It is though useful to have access a IO terminal while developing and degugging. The Arm GNU toolchain provides that support with the `--specs=rdimon.specs` argument added to the linker command and the `initialise_monitor_handles()` initialise function. They set up the low level functions to deal with file access that can interact with the debugger to offer to the developer the ability to use IO operations.

A little more here: [https://interrupt.memfault.com/blog/arm-semihosting](https://interrupt.memfault.com/blog/arm-semihosting)
 
Terminal IO takes place in the debugger log window for output, and through the GDB command line for input.

This feature is set in the Makefile

	USE_SEMIHOST=1

The feature can be deactivated with the value 0. The file `startup/libc_syscalls.c`, in which you have the low level functions for file access, is then added to the project. They provide minimal support to be able to use the full libc.

Note that **this feature is inhibited** when making the **release** version of the code. The support for needed IO must have been developped through the functions in `startup/lic_syscalls.c`.

<a id="rtt"></a>
# SEGGER REAL TIME TRANSFER

## Playing with RTT channels

In **OpenOCD**

	# setup rtt to find the Control Block in the first 2kb of RAM
	rtt setup 0x20000000 2048 "SEGGER RTT"
	
	# after the SEGGER_RTT_Init() was executed by the target
	rtt start
	
	# get some information
	rtt channels
	rtt channellist
	
	# setup the tcp server (port 5555) for the channel 0
	rtt server start 5555 0
	rtt server start 5556 1
	
In a **terminal**

	# run the tcp client
	tclsh cons.tcl
	
In **dbg**

	# run the code on the target ...
	continue

In **OpenOCD**

	# stop the server on port 5555, and the rtt service
	rtt server stop 5555
	rtt server stop 5556
	rtt stop
	
	
	
<a id="trouble"></a>
# TROUBLE SHOOTING

