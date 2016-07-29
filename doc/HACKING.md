# Hacking

You will want:
 - The RFC describing the WebDAV standard: RFC 4918.
 - The Apache httpd header files (from a `httpd-devel` package or the httpd source code).
 - The CASTOR header files (from a `castor-devel` package or the castor source code).
 - Sufficient supplies of caffeine.


## APR

The Apache Portable Runtime (APR) contains functions for all sorts of things, in particular memory management and string building functions are used heavily. All memory is allocated to a pool which is automatically freed when the request is completed.


## mod_dav

The majority of the work required to implement the WebDAV standard is in the mod_dav module, which is part of Apache. mod_dav_castor is implemented on top of mod_dav.

There is a lot of documentation in the mod_dav.h file.

The mod_dav_file module is a good place to see an alternative implementation (for storage on a local file system), as is the mod_lcgdm_dav module: https://svnweb.cern.ch/trac/lcgdm/browser/lcgdm-dav/trunk which has a redirecting implementation.


## CASTOR APIs

The CASTOR header files get installed in `/usr/include/shift`.

The nameserver API is in `Cns_api.h`. The majority of the calls mimic POSIX system calls (prefixed by `Cns_`), but use `serrno` rather than `errno` for error codes. All the nameserver calls have man pages.

The stager API is in `stager_client_api.h`. The API documentation is in that header file.

The rfio API is in `rfio_api.h`. The majority of the calls mimic POSIX system calls (prefixed by `rfio_`). The appropriate error code could be in `serrno`, `rfio_errno` or `errno`; use the `rfio_serrno` function to get the correct code. All the rfio calls have man pages.


## Files

Overview of what each source file does:
 - [bucket_castor.c](../src/bucket_castor.c): Implementation of a 'bucket brigade' for delivering CASTOR data. Based on the routines for delivering local file data in the APR. Lower-level IO calls are routed to `rfio_io.c`.  
 - [dav_errors.c](../src/dav_errors.c): Wrapper functions for the `dav_new_error` function that reduce copy-paste coding when creating error messages.  
 - [deliver.c](../src/deliver.c): Main code for handling GET requests, for files and directory listings in a browser.  
 - [liveprop.c](../src/liveprop.c): Live property handling - a few read-only properties are implemented.  
 - [locks.c](../src/locks.c): Dummy lock handling - breaks the standard but makes Win 7 work.  
 - [mod_dav_castor.c](../src/mod_dav_castor.c): Interface to Apache's module API.  
 - [ns_init.c](../src/ns_init.c): Nameserver initialisation code. Sets the hostname of the nameserver and the uid + gid of the user.  
 - [propdb.c](../src/propdb.c): Dead property handling - does nothing.  
 - [repository.c](../src/repository.c): Main procedures.  
 - [rfio_io.c](../src/rfio_io.c): Wrappers for RFIO functions - makes it easy to change to xrootd by replacing just this file.  
 - [stage.c](../src/stage.c): Stager call wrapper functions.  
 - [util.c](../src/util.c): Nice functions that do simple things.  
 - [walk.c](../src/walk.c): Function that iterates the contents of a directory (for GET / PROPFIND).
