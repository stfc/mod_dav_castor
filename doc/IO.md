# IO (PUT & GET)

IO requires a mover protocol. There are two sensible choices: rfio and xroot.  Neither has an API to authenticate on behalf of a user (such as the nameserver `Cns_setid` function) so the mover will behave as if the user running the web server had made the request. This causes a problem when the 'apache' user does not have sufficient permissions within CASTOR (i.e. it's always a problem). To get around this - the webserver must be run as the stager superuser 'stage'.

The 'stage' user can do anything within the stager, so it is important that prior to making stager or mover calls, an authorisation check is made in the nameserver using the `Cns_access` function.

The RFIO protocol is currently used. Wherever there are rfio calls in the code, the corresponding xroot calls are shown in comments. 


## Input (PUT)

If a file is created by a call to the stager or a mover, it will be owned by 'stage', rather than the user. The file must therefore be created in the nameserver before any call is made, using the `Cns_creat` function. This has two side effects: an authorisation check is done by the nameserver (yay!), and if the file already exists it is truncated. It doesn't matter that the file is truncated because we're about to write a new version (we don't support updates or appending to files).

The sequence of API calls for handling a PUT request is:
 1. `Cns_creat`
 2. `rfio_open` (which calls `stage_put`)
 3. `rfio_write`
 4. `rfio_close` (which calls `stage_putdone`)

Additionally, two checks are made against the webserver configuration. First, the `CastorAllowPut` directive is checked. Then, if the file exist, the `CastorAllowDelete` directive is checked. If either are false, the request is denied with '403 Forbidden'.

There is a bug in the CASTOR xroot plugin that prevents writing to 0-size files. For this reason, the rfio protocol is currently used (all writes are to 0-size files due to the `Cns_creat` call).


## Output (GET)

GET requests have one major complication - the file might not be available; it may be on tape; it may be on disk but the diskserver may be offline.

If the file is on tape, a call to `rfio_open` will hang until the file has been recalled, which will cause the user to timeout. We therefore need to call `stage_filequery` first to check that the file is available.

To start recall from tape for nearline files, we call `stage_prepareToGet`.  The automatic tape recall is optional so we check the value of the `CastorTapeRecall` directive first.

The sequence of API calls for handling a GET request is:
 1. `Cns_access`
 2. `stage_prepareToGet`
 3. `stage_filequery`
 4. `rfio_open` (which calls `stage_get`)
 5. `rfio_read`
 6. `rfio_close`

Calling `stage_prepareToGet` before `stage_filequery` allows more errors to be realised, i.e. if the `stage_prepareToGet` call fails - but is otherwise pedantic.
