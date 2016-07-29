# Authentication And Authorisation

See http://httpd.apache.org/docs/trunk/howto/auth.html for an introduction to authentication and authorisation in Apache.


## Authentication

The mod_dav_castor module does no authentication - it expects other Apache modules to do the authentication and for the UNIX username of the user to be in the Apache request record when control is passed to mod_dav.

A typical configuration would be to use mod_ssl with the `SSLUserName` directive along with mod_auth_grid to look up the DN of a user's certificate in a grid-mapfile.


## Authorisation

The username from the Apache request record is passed to the POSIX `getpwnam` function to obtain the UID of the user and the GID of their primary group.  These are then passed to the nameserver using the `Cns_setid` function. The nameserver is then responsible for making authorisation decisions.

Nameserver operations are automatically checked against the UID+GID. Non-nameserver operations (stager* and rfio operations) are preceded by a call to `Cns_access` to check that the user is authorised.
(*there is an undocumented stager function `int stage_setid(gid_t, uid_t)` but functions that are not part of the API have been avoided)

Other Apache authorisation modules can be used to further restrict access.
