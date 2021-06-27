AtomSpace RESTful Web API
=========================

This module exposes a RESTful web interface to the
[AtomSpace](https://github.com/opencog/atomspace).

The code here is needed by the
[AtomSpace Explorer](https://github.com/opencog/atomspace-explorer).

See http://wiki.opencog.org/w/REST_API for some more info.

Prerequisites
-------------
To build and run this module, the packages listed below are required.
With a few exceptions, most Linux distributions will provide these
packages. Users of Ubuntu may use the dependency installer from the
`/opencog/octool` repository.  Users of any version of Linux may
use the Dockerfile to quickly build a container in which OpenCog will
be built and run.

###### cogutil
> Common OpenCog C++ utilities
> https://github.com/opencog/cogutil
> It uses exactly the same build procedure as this package. Be sure
  to `sudo make install` at the end.

###### atomspace
> OpenCog Atomspace database and reasoning engine
> https://github.com/opencog/atomspace
> It uses exactly the same build procedure as this package. Be sure
  to `sudo make install` at the end.

###### cogserver
> OpenCog CogServer Network Server.
> https://github.com/opencog/cogserver
> It uses exactly the same build procedure as this package. Be sure
  to `sudo make install` at the end.

###### Flask
> Python micro web server
> Try `sudo apt install python3-flask`

###### CppREST
> C++ HTTP RESTful interfaces
> (Optional) Used by the Pattern miner for distributed processing
  (this will be replaced by gearman in future releases).
> `sudo apt-get install libcpprest-dev`

###### Threading Building Blocks
> C++ template library for parallel programming
> (Optional) Used to implement the optional REST API. (TODO: the
  REST API should be refactored to not use TBB)
> `sudo apt-get install libtbb-dev`

Building
--------
Perform the following steps at the shell prompt:
```
    cd to project root dir
    mkdir build
    cd build
    cmake ..
    make
```
Libraries will be built into subdirectories within build, mirroring
the structure of the source directory root.

Unit tests
----------
To build and run the unit tests, from the `./build` directory enter
(after building opencog as above):
```
    make test
```

Running
-------
To start the system, run one of the servers in the
[examples/restapi](examples/restapi) directory.

TODO
----
The current code assumes an obsolete API to the AtomSpace.  In particular,
it does not support Values in general, and assumes that all atoms have
a TruthValue (they don't, in general) and have an Attention value
(they don't when they are not in the attention bank).  This needs to be
fixed. Volunteers needed.
