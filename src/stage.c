/*
 * stage.c
 *
 * Routines that call the stager library - used during GET requests.
 *
 * The calls are unauthorised, i.e. they run as `stage'. There is a function
 * `int stage_setid(uid_t, gid_t)' but it's not in a public header file.
 *
 * For some reason the stager API calls don't use `const', so the string
 * arguments are cast from `const char *' to `char *'.
 *
 * The stager calls are designed for making multiple requests in a single
 * call. We must therefore check the return value from the function call AND
 * the value of `errorCode' from the `response' array returned by the
 * function.
 */

#include "mod_dav_castor.h"

#include "serrno.h"
#include "stager_client_api.h"


dav_error *
do_stage_prepareToGet(const dav_resource *resource)
{
	apr_pool_t *p = resource->pool;

	const char                   *str;
	dav_error                    *err = NULL;
	const dav_castor_dir_config  *conf;

	struct stage_prepareToGet_filereq   request;
	struct stage_prepareToGet_fileresp *response;
	int                                 nbresps;
	char                               *requestId;
	struct stage_options                opts;

	conf = GET_CONFIG(resource->info->request);

	request.protocol = MOVER_PROTOCOL_RFIOV3;
	request.filename = (char *)resource->uri;
	request.priority = 1;

	opts.stage_host    = (char *)conf->stager_host;
	opts.service_class = (char *)conf->service_class;
	opts.stage_version = 0;
	opts.stage_port    = 9002;

	if (stage_prepareToGet("WebDAV", &request, 1, &response, &nbresps,
	                       &requestId, &opts) < 0)
		return sdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                   "stage_prepareToGet()");

	if (response->errorCode) {
		str = apr_psprintf(p,
		                   "stage_prepareToGet(): %s [errorCode=%d]",
		                   response->errorMessage,
		                   response->errorCode);
		err = dav_strerror(p, HTTP_INTERNAL_SERVER_ERROR, str);
	}

	free_prepareToGet_fileresp(response, nbresps);
	free(requestId);

	return err;
}

dav_error *
do_stage_filequery(const dav_resource *resource)
{
	apr_pool_t *p = resource->pool;

	const char                     *str;
	dav_error                      *err = NULL;
	const dav_castor_dir_config    *conf;

	struct stage_query_req          request;
	struct stage_filequery_resp    *response;
	int                             nbresps;
	struct stage_options            opts;

	conf = GET_CONFIG(resource->info->request);

	request.type  = BY_FILENAME;
	request.param = (char *)resource->uri;

	opts.stage_host    = (char *)conf->stager_host;
	opts.service_class = (char *)conf->service_class;
	opts.stage_version = 0;
	opts.stage_port    = 9002;

	if (stage_filequery(&request, 1, &response, &nbresps, &opts) < 0)
		return sdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                   "stage_filequery()");

	if (response->errorCode) {
		str = apr_psprintf(p, "stage_fileuery(): %s [errorCode=%d]",
		                   response->errorMessage,
		                   response->errorCode);
		err = dav_strerror(p, HTTP_LOCKED, str);
	} else {
		switch (response->status) {
		 case FILE_STAGED:
		 case FILE_CANBEMIGR:
			// We can read the file in these states :-)
			err = NULL;
			break;
		 default:
			// File not staged - there's no sensible HTTP status
			// code for this so lets go with `Locked'
			str = apr_psprintf(p, "File not staged (%s)",
			                   stage_fileStatusName(response
			                                        ->status));
			err = dav_strerror(p, HTTP_LOCKED, str);
			break;
		}
	}

	free_filequery_resp(response, nbresps);

	return err;
}
