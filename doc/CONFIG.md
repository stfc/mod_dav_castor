# Apache Configuration

To load the module, add the line `LoadModule dav_castor_module modules/mod_dav_castor.so` to an appropriate Apache configuration file (probably `httpd.conf`, but you may want a seperate file for WebDAV configuration).

To use the mod_dav_castor module, add the directive `dav Castor` to the appropriate `<Location>` section.

For the module to work correctly, the webserver must run as the `stage` user.  Change the `User` and `Group` directives in `httpd.conf` to `User stage` and `Group st`.


## Directives

These are the directives implemented by mod_dav_castor for the Apache configuration files. They go inside `<Location>` sections.

#### CastorNameserver
The hostname of the nameserver. Required.

#### CastorStager
The hostname of the stager. If not specified, GET / PUT requests will fail with '403 Forbidden'.

#### CastorServiceClass
The service class. If not specified, GET / PUT requests will fail with '403 Forbidden'.

#### CastorUmask
The umask to use on the nameserver - an octal number. The default is 022.


## Boolean Directives

The following options take 'Yes' for true, any other value for false. They all default to false:

#### CastorAllowPut
Whether to allow PUT requests.

#### CastorAllowDelete
Whether to allow DELETE requests. Also specifies whether files can be overwritten during MOVE and PUT requests.

#### CastorAllowMove
Whether to allow MOVE requests.

#### CastorAllowMkcol
Whether to allow MKCOL requests. If you allow MKCOL you should allow MOVE as well, because the clients tend to create directories called 'New Folder' and then rename them.

#### CastorTapeRecall
Whether to automatically recall files from tape during GET requests (if the file is not online, the request will fail regardless). Note that if this is not set, then you cannot differentiate between a file on tape and a file on a disabled disk server.


## Example

```apache
# The following is an example configuration for the pre-production instance at
# RAL:

LoadModule dav_castor_module modules/mod_dav_castor.so

<Location /castor>
	Dav Castor
	CastorNameserver "cprens.ads.rl.ac.uk"
	CastorStager "cprestager.ads.rl.ac.uk"
	CastorUmask 0022
	CastorAllowMkcol Yes
	CastorAllowMove Yes
	CastorAllowDelete Yes
	CastorAllowPut Yes
	CastorTapeRecall Yes
</Location>

<Location /castor/preprod.ral/preprodDisk>
	CastorServiceClass "preprodDisk"
</Location>
<Location /castor/preprod.ral/preprodTape>
	CastorServiceClass "preprodTape"
</Location>
<Location /castor/preprod.ral/preprodTape2>
	CastorServiceClass "preprodTape2"
</Location>
```
