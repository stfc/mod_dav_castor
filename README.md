# mod_dav_castor

mod_dav_castor is an Apache httpd module that provides a WebDAV interface to the CASTOR storage system.


## Installing

You will need the development packages for httpd and the castor-lib. To install the module, run:
```sh
cd src
make
sudo make install
```

The build uses the apxs programme (should be part of the httpd devel package): http://httpd.apache.org/docs/2.2/programs/apxs.html


## Quick Setup

Information on setting up the Apache configuration files for using the module can be found in [doc/CONFIG.md](doc/CONFIG.md).

Setting up authentication is non-trivial. This module does not do any authentication itself, but requires an authenticated user. Read [doc/AUTH.md](doc/AUTH.md) for more details.


## Documentation

The following can be found in the `doc` directory:

 - [AUTH.md](doc/AUTH.md): How authorisation and authentication are handled in this module.  
 - [CONFIG.md](doc/CONFIG.md): How to configure Apache to use this module, and the directives for the Apache config files implemented by this module.  
 - [HACKING.md](doc/HACKING.md): A guide to improving the module.  
 - [IO.md](doc/IO.md): An overview of how IO (PUT and GET requests) work.  
 - [RPM.md](doc/RPM.md): A guide to making an RPM for the module.  
 - [WIN7.md](doc/WIN7.md): A summary of workarounds for deficiencies in the Windows 7 client.
