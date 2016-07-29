# Windows 7 Client

This document notes some limitations with the Windows 7 client - which MS calls the 'WebDAV Mini Redirector', and how they can be overcome.


## X.509 Certificates

There is support for client-side X.509 authentication, but only when the certificate is stored unencrypted. Since this is a bad idea, client-side X.509 authentication should not be considered as an option when using the Windows 7 client.

If X.509 is the preferred method of authentication, this can be bypassed by using a myproxy server. There is Apache support in the mod_authn_myproxy module: http://grid.ncsa.illinois.edu/myproxy/apache/.


## Locking

When the webserver does not advertise support for the LOCK and UNLOCK methods in an OPTIONS request, the Windows 7 client will still send LOCK requests, with inevitable failure. To get around this, a dummy locking mechanism has been implemented:
 - all LOCK requests succeed;
 - all UNLOCK requests succeed;
 - all requests with an 'If:' header (a lock precondition) behave as if the lock existed.
