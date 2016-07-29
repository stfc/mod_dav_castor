/*
 * walk.c
 *
 * Routines for 'walking' a directory. Since we don't support `Depth:
 * infinity' (including when implied in DELTE, MOVE and COPY), there is no
 * need for any recursion.
 */

#include "mod_dav_castor.h"

#include "serrno.h"
#include "Cns_api.h"


dav_error *
walk_collection(const dav_walk_params *params, dav_response **response)
{
	*response = NULL;

	apr_pool_t *p = params->pool;

	Cns_DIR *dirp;

	int sav_serrno;
	dav_error *err = NULL;

	struct Cns_direnstat *statbuf;

	dav_walk_resource walk_resource;
	dav_resource         resource;
	dav_resource_private info;

	memset(&walk_resource, 0, sizeof(walk_resource));

	walk_resource.walk_ctx  = params->walk_ctx;
	walk_resource.pool      = p;
	walk_resource.resource  = params->root;

	info.request = params->root->info->request;

	resource.type   = DAV_RESOURCE_TYPE_REGULAR;
	resource.exists = 1;
	resource.info   = &info;
	resource.hooks  = &dav_castor_hooks_repository;
	resource.pool   = p;

	dirp = Cns_opendir(params->root->uri);
	if (NULL == dirp)
		return sdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                   "Cns_opendir()");

	while((serrno = 0, statbuf = Cns_readdirx(dirp)) != NULL) {
		resource.collection = S_ISDIR(statbuf->filemode) ? 1 : 0;
		resource.uri = apr_pstrcat(p, params->root->uri,
		                           statbuf->d_name,
		                           S_ISDIR(statbuf->filemode) ? "/"
		                                                      : "",
		                           NULL);

		info.filestat = (void *)statbuf;

		walk_resource.resource = &resource;
		err = params->func(&walk_resource, resource.collection
		                           ? DAV_CALLTYPE_COLLECTION
		                           : DAV_CALLTYPE_MEMBER);

		if (err != NULL)
			break;
	}

	sav_serrno = serrno;

	Cns_closedir(dirp);

	if (err != NULL)
		return err;

	if (0 != sav_serrno)
		return sdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                   "Cns_readdirx()");

	return NULL;
}
