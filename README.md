# Serial Wire Debug Open Library
[![Build Status](https://travis-ci.org/cederom/LibSWD.svg?branch=master)](https://travis-ci.org/cederom/LibSWD)

Welcome to http://libswd.com / https://GitHub.com/CeDeROM/LibSWD :-)

**HAVE YOU SEEN [THE ARM MBED DAPLink PROJECT](https://github.com/mbedmicro/DAPLink)? ITS AN OPEN-SOURCE FIRMWARE THAT CAN CONVERT TINY ARM MCU INTO USB HARDWARE DEBUGGER FOR RAPID DRAG-N-DROP FLASHING, HID/CMSIS-DAP DEBUG, AND VIRTUAL COM PORT UART CONNECTED TO YOUR TARGET! I AM DEVELOPING THAT PROJECT AS WELL, PLEASE GIVE IT A TRY, YOU WILL GET EFFICIENT AND EASY SOLUTION TO DEVELOP YOUR DESING! :-)**

## Table of Contents
 * [About](#about)
 * [Credits](#credits)
 * [Releases](#releases)
 * [Source Code](#source-code-repository)
 * [Documentation](#documentation)
 * [References](#references)
  * [Resources](#resources)
  * [Software](#software)
  * [Papers](#papers)
 * [History](#history)
 * [Support](#support)
 * [Developer's Handbook](#developers-handbook)
  * [How to build and install](#how-to-build-and-install)
    * [Building from a GIT repository](#building-from-a-git-repository)
    * [Building a Release](#building-a-release)
    * [Run and Debug without installing](#run-and-debug-without-installing)
    * [Configure parameters](#configure-parameters)
    * [Build dependencies](#build-dependencies)
    * [Doxygen documentation](#doxygen-documentation)
    * [Install, Deinstall, Package](#install-deinstall-package)
  * [Include LibSWD Source Code into your own application](#include-libswd-source-code-into-your-own-application)
  * [LibSWD API](#libswd-api)
    * [Introduction](#introduction)
    * [Overview](#overview)
    * [How it works](#how-it-works)
      * [SWD Context](#swd-context)
      * [Functions](#functions)
      * [Commands](#commands)
      * [Drivers](#drivers)
      * [Error and Retry handling](#error-and-retry-handling)
    * [Example](#example)

## About

This is an official website of the [LibSWD Project](https://github.com/cederom/LibSWD) - Free and Open solution for accessing the SW-DP (Serial Wire Debug Port) on [ARM-Cortex CPU](http://en.wikipedia.org/wiki/ARM_architecture) based embedded systems. Source code is released under ["new" 3 clause BSD License](http://www.opensource.org/licenses/bsd-license.php). **Please note that on April 2015 project has been moved from SourceForce to GitHub.**

LibSWD was started in 2010 by [Tomasz Bolesław CEDRO](http://www.tomek.cedro.info) in order to create Generic, Free, Open, Hardware and Platform Independent implementation of Serial Wire Debug access to 
[ARM-Cortex CPU cores](http://en.wikipedia.org/wiki/ARM_architecture). SWD is an alternative to JTAG method for accessing the On-Chip Debug Access Port that allows low-level access to system resources such as system bus, memory, IO, even single stepping the code execution - a must-have for an Embedded Systems Developer and/or Analyst.

SWD works on both simple and advanced devices that belong to the ARM Cortex family. By default Cortex CPU or FPGA'a IP Core is both JTAG and SWD capable, so the common name for this transport method is SWJ (Serial Wire and JTAG). It is the application requirement and designer choice if a final product supports JTAG, SWD, or both (SWJ). [Here](http://www.arm.com/files/pdf/Low_Pin-Count_Debug_Interfaces_for_Multi-device_Systems.pdf) you can read short and interesting paper on how SWD overcomes JTAG limitations.

LibSWD can be considered generic and fully featured solution that contains simple standalone commandline application, but it also can be easily integrated in a form of dynamic library into third party software with minimal impact on existing program structure. LibSWD implements all utilities necessary to generate bit stream and queue bus operations on both high (operation) and low (bit) abstraction level. High level functions can be called from external application to generate queue that represent bus operations or simply produce a bitstream that can be later flushed into real hardware with simple set of functions (these functions can utilise existing cable drivers already implemented in external software).

Initial research and practical tests were performed back in 2010 on STM32 family ARM Cortex-M3 based devices using [UrJTAG](http://urjtag.sf.net) and [OpenOCD](http://openocd.sf.net) utilities with [FT2232](http://www.ftdichip.com/Products/ICs/FT2232D.htm) based hardware interfaces. At this moment I am using [KT-LINK](http://shop.kristech.pl/p/24/257/kt-link-.html) interface for communication with various Targets, trying to integrate LibSWD into my [iCeDeROM](https://github.com/cederom/icederom) project that would replace simple commandline application with fully featured extensible GUI environment for low-level development and analysis of embedded electronics and computer systems.

## Credits

Various parts of this research project were thankfully conducted, contributed and supported by:
* [Tomasz Bolesław CEDRO aka CeDeROM](http://www.tomek.cedro.info) (me) that is Inventor, Developer and Maintainer of LibSWD.
* [Orange Labs Warsaw](http://www.orange.com/en_EN/group/global_footprint/countries/poland/poland-lab.jsp) and [Orange Labs Paris](http://www.orange.com/en_EN/group/global_footprint/countries/france/france-lab.jsp) that I worked for as R&D Architect.
* [Cybernetic Research Student Group](http://cyber.ise.pw.edu.pl/) that I am founder, at [Warsaw University of Technology](http://www.pw.edu.pl).
* [Polish Interdisciplinary NEUROSCIENCE Group](http://www.neuroscience.pl) that I am founder.
* Krzysztof Kajstura of [Kristech](http://kristech.eu/).
* Andras Ketskes of [BodyTrace Inc](http://www.bodytrace.com/).
* Andrew Parlane of [Carallon Ltd](http://www.carallon.com/).
* [Stephen Groat](https://www.stephengroat.com/) of [Qualcomm Inc](https://www.qualcomm.com/).
* Individuals from all around the world, including [Freddie Chopin](http://www.freddiechopin.info), [Akos Vandra](https://github.com/axos88), [Evan Foss](https://sites.google.com/site/evanfoss).
* My so called "free" time ;-)


## Releases

### 2017-02-15: [libswd-0.7.tar.gz](https://github.com/cederom/LibSWD/releases/download/libswd-0.7/libswd-0.7.tar.gz) / [libswd-0.7.pdf](https://github.com/cederom/LibSWD/releases/download/libswd-0.7/libswd-0.7.pdf)

### 2013-10-20: [libswd-0.6.tar.gz](https://github.com/cederom/LibSWD/releases/download/libswd-0.6/libswd-0.6.tar.gz) / [libswd-0.6.pdf](https://github.com/cederom/LibSWD/releases/download/libswd-0.6/libswd-0.6.pdf)

### 2012-12-06: [libswd-0.5.tar.gz](https://github.com/cederom/LibSWD/releases/download/libswd-0.5/libswd-0.5.tar.gz) / [libswd-0.5.pdf](https://github.com/cederom/LibSWD/releases/download/libswd-0.5/libswd-0.5.pdf)

### 2012-10-14: [libswd-0.4.tar.gz](https://github.com/cederom/LibSWD/releases/download/libswd-0.4/libswd-0.4.tar.gz) / [libswd-0.4.pdf](https://github.com/cederom/LibSWD/releases/download/libswd-0.4/libswd-0.4.pdf)

### 2011-11-11: [libswd-0.3.tar.gz](https://github.com/cederom/LibSWD/releases/download/libswd-0.3/libswd-0.3.tar.gz) / [libswd-0.3.pdf](https://github.com/cederom/LibSWD/releases/download/libswd-0.3/libswd-0.3.pdf)

### 2011-10-31: [libswd-0.2.tar.gz](https://github.com/cederom/LibSWD/releases/download/libswd-0.2/libswd-0.2.tar.gz) / [libswd-0.2.pdf](https://github.com/cederom/LibSWD/releases/download/libswd-0.2/libswd-0.2.pdf)

### 2011-04-02: [libswd-0.1.tar.gz](https://github.com/cederom/LibSWD/releases/download/libswd-0.1/libswd-0.1.tar.gz) / [libswd-0.1.pdf](https://github.com/cederom/LibSWD/releases/download/libswd-0.1/libswd-0.1.pdf)


## Source Code Repository

At first LibSWD was small piece of experimental software stored on my hard drive. Then I decided to share it with SVN. As GIT showed up and turned out to be a very very nice tool for remote source code management, I have moved to GIT. Then GitHub showed up with incredibly efficient and intuitive features for collaborative development, so I have moved everything to GitHub.

In order to clone Upstream repository, run:

 `git clone https://github.com/cederom/LibSWD.git`

If you want to send a patch to Upstream, create a your own fork at GitHub, commit your changes to the fork, then create a [Pull Request](https://help.github.com/articles/using-pull-requests/) to the Upstream. As simple as that! :-)


## Documentation

LibSWD is documented using Doxygen. Documentation body is included within the source code and then generated with doxygen utility to obtain html/pdf output files (run `make doxygen-doc` command), therefore documentation sources and its result is integral part of the source code repository. All necessary documents are frozen upon release and delivered within the package to match code version (see Releases section). 


## References

### Resources 

* [Project History](https://github.com/cederom/LibSWD/wiki/Project-History) - historical document on from-scratch implementation and verification of LibSWD on ARM-Cortex devices (using Stm32Primer2 DevelKit, KT-LINK Interface, UrJTAG and OpenOCD software utilities, FreeBSD OS).
* [ARM Info Center](http://infocenter.arm.com/help/) - every information you need to get about ARM product is there (including CoreSight On-Chip Trace and Debug).
* [ARM SWD Website](http://www.arm.com/products/system-ip/debug-trace/coresight-soc-components/serial-wire-debug.php) - Official ARM website with SWD description. 
* [STM32 SWD Website](http://www.arm.com/products/system-ip/debug-trace/coresight-soc-components/serial-wire-debug.php) - Official STM32 with SWD description. 

### Software

* [iCeDeROM](http://www.github.com/cederom/icederom) - My new software utility that will do In-Circuit Evaluate Debug and Edit for Research on Microelectronics.
* [UrJTAG](http://urjtag.sf.net/) - Open-Source utility to perform low-level TAP operations on various hardware targets. 
* [OpenOCD](http://openocd.sf.net/) - Open-Source utility to (re)program target's memory and code debug. 

### Papers

* ["LibSWD serial wire debug open framework for low-level embedded systems access"](https://www.researchgate.net/publication/261079643_LibSWD_serial_wire_debug_open_framework_for_low-level_embedded_systems_access),  T.Cedro, M.Kuzia, A.Grzanka, Computer Science and Information Systems (FedCSIS), 2012 Federated Conference on.
* ["A Bits' Life"](https://www.researchgate.net/publication/269094704_A_Bits%27_Life), T.Cedro, M.Kuzia, Hakin9 Mobile Security 02/2012(3), ISSN: 1733-7186 02/2012.
* ["Low Pin-count Debug Interfaces for Multi-device Systems"](http://www.arm.com/files/pdf/Low_Pin-Count_Debug_Interfaces_for_Multi-device_Systems.pdf), Michael Williams, ARM Limited, 110 Fulbourn Road, Cambridge, England.


## History

### LibSWD-0.8-DEVEL:
 * Added Travis CI build test automation integrated with GitHub repository.

### 2017-02-16:
* LibSWD project gets dedicated internet domain so you can reach it quickly at http://libswd.com :-)

### 2017-02-15:
* LibSWD 0.7 release.
* ARM Cortex-M0 and Cortex-M4 support.
* AP read fix (ABORT after READ).
* MEMAP support for 16-bit non-packed writes.
* LibSWD Application CLI and CPUID improvements.
* Fixed byte-laning and memory alignment issues.
* Source code cleanup.
* Special thanks to Andrew Parlane of [Carallon Ltd.](http://www.carallon.com/)! :-)

### 2015-04-10:
* LibSWD Project moves out from SourceForge to GitHub and is now available at https://github.com/cederom/LibSWD.

### 2013-10-20:

* LibSWD 0.6 release.
* Introducing CLI (Command Line Interface) parser framework. Updated error defines.
* Major fixes in LOG functions. Introducing vprintf() like libswd_log_internal_va().
* Added DEFAULT defines for LogLevel and AutoFixErrors libswdctx->config values.
* Introducing MEM-AP routines.
* Introducing Standalone LibSWD Aplication.
* Introducing initial support for Debug and Flash access.
* Special thanks to Andras Ketskes of [BodyTrace Inc.](http://www.bodytrace.com/)! :-)

### 012-12-06:

* LibSWD 0.5 release.
* All functions and defines were renamed to use prefix libswd_ and LIBSWD_.
* This is a first and official candidate for release with OpenOCD master.

### 2012-10-14:

* LibSWD 0.4 is out, bugfix and functional verification release that includes:
* Implemented necessary data phase on ACK={WAIT,FAULT} reply from Target.
* Unknown ACK response and Protocol Error Sequence detection.
* swd_cmd_t was added *errors to hold queue for error handling in ACK element.
* automatic error handling on queue was not included in this release yet.
* fixed swd_cmdq_free_tail()
* swd_cmdq_flush() updates swdctx->cmdq upon execution.
* swd_drv_transmit() updates swdctx->log values only on successful transmit.
* DP and AP operations including MEM-AP now works.
* OpenOCD integration is almost done, flashing works (1938B/18.6s with FT2232).
* By the way, I express my admiration to Felix Baumgartner and the whole Redbull STRATOS team for their pioneer achievements and today aerospace/skydive records!! :-)

### 2011-11-11

* LibSWD 0.3 comes out, a major bugfix release:
* Fixed critical issue with ACK/DATA bitswap due erratic ARM documentation.
* Fixes and improvements in libswd.h.
* Debug hint function in Request packet decoposition (libswd_drv).
* Fixed OpenOCD drivers.
* Introduced SWD_LOGLEVEL_PAYLOAD even more verbose than debug.
* All transfers are now LSB-First.
* Major bugfixes.

### 2011-11-1
 
* Code repository now use only a GIT system.
* SVN only stores doxygen documentation in trunk for easier web browsing, including documentation and code for releases.
* Synchronization between svn and git is a mess, we don't need that, git is far better for code development :-)

### 2011-10-31

* LibSWD 0.2 release.
* Source code reorganization (split into different files).
* Build system fixes and documentation updates.
* Integration with OpenOCD.
* Bugfixes and improvements.
* Created function set for easier work on DAP.
* Error detection in swd_drv_transmit() - cmdq is truncated on error.

### 2011-04-02

* LibSWD initial version 0.1 released to the public.
* Basic functionality of transport, queue and drivers for ADIv5.0.
* Automated SW-DP activation/reset and IDCODE read.
* Tested and verified functionality on UrJTAG.
* Source code documented using Doxygen.
* Autotools integration for standarized build.

### 2011-03-23

* Integration with UrJTAG complete. Source code sent to developers.
* Functionality verification complete. Updated work in progress description at http://stm32primer2swd.sf.net/
* Started integration with OpenOCD...

### 2011-02-09

* Remote source code repository (SVN) started.

### 2010-12-29

* Official LibSWD project website launched at http://libswd.sf.net

### 2010-11-23

* Decision to implement SWD API as standalone library.

### 2010-10-21

* Work to create open swd implementation has begun.
* Website with progress log launched at http://stm32primer2swd.sourceforge.net/.


## Support

Please use GitHub goodies for support:
 * [Issues](https://github.com/cederom/libswd/issues) - to discuss any problems that you encounter.
 * [Pull Requests](https://github.com/cederom/libswd/pulls) - to to send patches.

Well tested patches are highly welcome and appreciated! :-)

## Developer's Handbook

### How to build and install

#### Building from a GIT repository

To stay in sync with development use GIT. This is also good if you want to test new features that are not yet in a release:

```
 git clone https://github.com/cederom/LibSWD.git
 cd LibSWD
 ./autogen.sh
 ./configure <optional parameters>
 make
```

If new commits show up, just pull the repo (`git pull`) and rebuild the new code.

If you intend to add new features, fix bugs, etc - go to GitHub, create your own fork of LibSWD, make changes, create commits, then create a [Pull Request](https://help.github.com/articles/using-pull-requests/). 

#### Building a Release

To obtain the latest release visit project website at https://github.com/cederom/LibSWD. LibSWD use Autotools, so the build and install procedure is pretty straightforward:

```
 tar xzvf libswd-<version>.tar.gz
 cd libswd-<version>
 ./configure <optional parameters>
 make
```

#### Run and Debug without installing

For development purposes it is very convenient to simply fetch the source code, build, and debug without installing. Remember to enable debug in `configure` before compilation.

 `libtool --mode=execute gdb src/libswd`

#### Configure parameters

To know more build options that can be set before compilation:

 `./configure --help`

Important configure parameters:
* `--enable-debug` will build the binary with debug symbols (useful for development and bug-tracking, disabled by default).
* `--enable-application` will also build the commandline application (disabled by default).
* `CFLAGS=...` will pass important flags to the compiler (i.e. `CFLAGS="-I/usr/local/include/libftdi1"` on FreeBSD or `CFLAGS="-I/opt/local/include/libftdi1"` on MacOSX may come handy when includes are missing during compilation).
* `LDFLAGS=...` will pass important flags to the linker (i.e. `LDFLAGS="-L/usr/local/lib"` on FreeBSD or `LDFLAGS="-L/opt/local/lib"` on MacOSX may come handy when required libraries are found missing during configure or compilation).

#### Build Dependencies

In order to build the Library and the Application you need: `gcc`, `autogen`, `automake`, `autoconf`, `libtool`, `libftdi`, `libreadline` development packages installed on your system.

#### Doxygen Documentation

It is possible to build the code documentation in `html` and `pdf` format. Note that you need to have [Doxygen](http://www.doxygen.org) installed for this to work. Additionally you need to have `LaTeX` installed to build the `PDF` output. In order to build the complete documentation, run: 

 `make doxygen-doc`

#### Install, Deinstall, Package

To create and verify package file for distribution:

 `make distcheck`

To clean the build, but keep configure and makefiles:
 `make clean`

To clean the build and revert to clean package state:
 `make distclean`

To install and uninstall the library:

`make install`

`make uninstall`

To specify alternative install location:

 `./configure --prefix=<your path>`
 
### Include LibSWD Source Code into your own application

You can include LibSWD source code into your own application using [GIT SUBMODULE](http://git-scm.com/docs/git-submodule) feature that allows you to fetch directly from the upstream repository as a part of build process. See [OpenOCD+LibSWD](http://repo.or.cz/w/openocd/cederom.git) integration as an example.

### LibSWD API

This section is taken from the Doxygen documentation of the LibSWD source code. Please review source code documentation file if you need more information..

#### Introduction

LibSWD is an Open-Source framework to deal with with Serial Wire Debug Port in accordance to ADI (Arm Debug Interface, version 5.0 at the moment) specification. It is released under 3-clause BSD license. For more information please visit project website at https://github.com/cederom/LibSWD


#### Overview

Serial Wire Debug is an alternative to JTAG (IEEE1149.1) transport layer for accessing the Debug Access Port in ARM-Cortex based devices. LibSWD provides methods for bitstream generation on the wire using simple but flexible API that can reuse capabilities of existing applications for easier integration.

Every bus operation such as control, request, turnaround, acknowledge, data and parity packet is named a "command" represented by a `libswd_cmd_t` data type that builds up the queue that later can be flushed into real hardware using standard set of (application-specific) driver functions.

This way LibSWD is almost standalone and can be easily integrated into existing utilities for low-level access and only requires in return to define driver bridge that controls the physical interface interconnecting host and target.

Drivers and other application-specific functions are `extern` and located in external file crafted for that application and its hardware. LibSWD is therefore best way to make your application SWD aware.


#### How it works

##### SWD Context

The most important data type in LibSWD is `libswd_ctx_t` structure, a context that represents logical entity of the swd bus (transport layer between host and target) with all its parameters, configuration and command queue. Context is being created with `libswd_init()` function that returns pointer to allocated virgin structure, and it can be destroyed with `libswd_deinit()` function taking the pointer as argument. Context can be set only for one interface-target pair, but there might be many different contexts in use if necessary, so amount of devices in use is not limited. 

##### Functions

All functions in general operates on pointer type and returns number of processed elements on success or negative value with `libswd_error_code_t` on failure. Functions are grouped by functionality that is denoted by function name prefix (ie. `libswd_bin*` are for binary operations, `libswd_cmdq*` deals with command queue, `libswd_cmd_enqueue*` deals with creating commands and attaching them to queue, `libswd_bus*` performs operation on the swd transport system, `libswd_drv*` are the interface drivers, etc). Because programs using libswd for transport can queue multiple operations and don't handle errors of each transaction apropriately, `libswd_drv_transmit()` function verifies the ACK and PARITY operation results directly after execution (read from target) and return error code if necessary. When error is detected and there were some pending perations enqueued for execution, they are discarded and removed from the queue (they would not be accepted by the target anyway), the queue is then again ready to accept new transactions (i.e. error handling operations).

Standard end-users are encouraged to only use high level functions (`libswd_bus*`, `libswd_dap*`, `libswd_dp*`) to perform operations on the `SWD` transport layer and the target's `DAP` (Debug Access Port) and its components such as `DP` (Debug Port) and the `AP` (Access Port). More advanced users however may use low level functions (`libswd_cmd*`, `libswd_cmdq*`) to group them into new high-level functions that automates some tasks (such as high-level functions do). Functions of type `extern` are the ones to implement in external file by developers that want to incorporate LibSWD into their application. Context structure also has void pointer in the `libswd_driver_t` structure that can hold address of the external driver structure to be passed into internal swd drivers (`extern libswd_drv*` functions) that wouldn't be accessible otherwise.

##### Commands

Bus operations are split into `commands` represented by `libswd_cmd_t` data type. They form a bidirectional command queue that is part of `libswd_ctx_t` structure. Command type, and so its payload, can be one of: control (user defined 8-bit payload), request (according to the standard), ack, data, parity (data and parity are separate commands!), trn, bitbang and idle (equals to control with zero data). Command type is defined by `libswd_cmdtype_t` and its code can be negative (for `MOSI` / Master Output Slave Input operations) or positive (for `MISO` / Master Input Slave Output operations) - this way bus direction can be easily calculated by multiplying two operation codes (when the result is negative bus will have to change direction), so the libswd "knows" when to put additional `TRN` command of proper type between enqueued commands.

Payload is stored within union type and its data can be accessed according to payload name, or simply with `data8` (`char`) and `data32` (`int`) fields. Payload for write (`MOSI`) operations is stored on command creation, but payload for read (`MISO`) operations becomes available only after command is executed by the interface driver. There are 3 methods of accessing read data - flushing the queue into driver then reading queue directly, single stepping queue execution (flush one-by-one) then reading context log of last executed command results (there are separate fields of type `libswd_transaction_t` in `libswd_ctx_t` log structure for read and write operations that are updated by `libswd_drv_transmit()` function before write and after read), or  providing a double pointer on command creation to have constant access to its data after execution.

After all commands are enqueued with `libswd_cmd_enqueue*` function set, it is time to send them into physical device with `libswd_cmdq_flush()`. According to the `libswd_operation_t` parameter commands can be flushed one-by-one, all of them, only to the selected command or only after selected command. For low level functions all of these options are available, but for high-level functions only two of them can be used - `LIBSWD_OPERATION_ENQUEUE` (but not send to the driver) and `LIBSWD_OPERATION_EXECUTE` (all unexecuted commands on the queue are executed by the driver sequentially) - that makes it possible to perform bus operations one after another having their result just at function return, or compose more advanced sequences leading to preferred result at execution time. Because high-level functions provide simple and elegant manner to get the operation result, it is advised to use them instead dealing with low-level functions (implementing memory management, data allocation and queue operation) that exist only to make high-level functions possible. 

##### Drivers

Calling the `libswd_cmdq_flush()` function leads to execution of not yet executed commands from the queue (in a manner specified by the operation parameter) on the `SWD` bus (transport layer between interface and target, not the bus of the target itself) by `libswd_drv_transmit()` function that use application specific `extern` functions defined in external file (ie. `liblibswd_drv_urjtag.c`) to operate on a real hardware using drivers from existing application. LibSWD use only `libswd_drv_{mosi,miso}_{8,32}` (separate for `8-bit char` and `32-bit int` data cast type) and `libswd_drv_{mosi,miso}_trn` functions to interact with drivers, so it is possible to easily reuse low-level and high-level devices for communications, as they have all information necessary to perform exact actions - number of bits, payload, command type, shift direction and bus direction. It is even possible to send raw bytes on the bus (control command) or bitbang the bus (bitbang command) if necessary. `MOSI` (Master Output Slave Input) and `MISO` (Master Input Slave Output) was used to clearly distinguish transfer direction (from master-interface to target-slave), as opposed to ambiguous read/write statements, so after `libswd_drv_mosi_trn()` master should have its buffers set to output and target inputs active. Drivers, as most of the LibSWD functions, works on data pointers instead data copy and returns number of elements processed (bits in this case) or negative error code on failure.

##### Error and Retry handling

LibSWD is equipped with optional automatic error handling in order to make error and retry handling easier for external applications that were meant for JTAG applications (such as OpenOCD) which first enqueue lots of operations and then flushes them into hardware loosing information on where the target reported problem with `ACK!=OK`. The default behavior of LibSWD for `ACK!=OK` response from Target is to truncate the queue right after the bad `ACK` (eventually executing the necessary data phase before doing that) to preserve synchronization between command queue (`libswd_ctx_t->cmdq`) and the Target state. This can be changed by clearing out the `libswd_ctx_t.config.autofixerrors` field that disables queue truncate on error, then applying the `libswd_dap_retry()` in the application flush mechanism for both DP and AP operations. `libswd_dap_retry()` will try to find the `ACK!=OK` on the queue that caused an error then perform operation retry to fix the situation, or fail permanently (Protocol Error Sequence, Retry Count, etc). Note that retry will be handled in a different way than it was performed on the original command queue and it will use separate command queue attached to a bad `ACK` command element on the queue. This approach gives ability to handle different situations accordingly, does not interfere with the original queue and does not loose information what additional operations had been performed, in perfect situation it should end up in having the original queue executed as there was no error/retry. 

#### Example

```C
  #include <libswd.h>
  int main(){
   libswd_ctx_t *libswdctx;
   int res, *idcode;
   libswdctx=libswd_init();
   if (libswdctx==NULL) return -1;
   //we might need to pass external driver structure to libswd_drv* functions 
   //libswdctx->driver->device=...
   res=libswd_dap_detect(libswdctx, LIBSWD_OPERATION_EXECUTE, &idcode);
   if (res<0){
    printf("ERROR: %s\n", libswd_error_string(res));
    return res;
   } else printf("IDCODE: 0x%X (%s)\n", *idcode, libswd_bin32_string(idcode));
   libswd_deinit(libswdctx);
   return 0;
  }
```

<hr/>
<sup>LibSWD (C) 2010-2017 [CeDeROM Tomasz Bolesław CEDRO](http://www.tomek.cedro.info)</sup> 
