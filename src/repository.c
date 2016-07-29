#include <stdlib.h>
#include <libgen.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mod_dav_castor.h"

#include <http_request.h>

#include "serrno.h"
#include "Cns_api.h"


struct dav_stream {
	apr_pool_t *pool;
	int         fd;
};


static dav_error *
get_resource_from_hsm_filename(request_rec *r, const char *hsm_filename,
                               dav_resource **result_resource)
{
	dav_castor_dir_config  *conf;
	dav_resource_private   *info;
	dav_resource           *resource;
	int                     http_error;

	struct Cns_filestat    *statbuf;

	conf = GET_CONFIG(r);

	info = apr_pcalloc(r->pool, sizeof(*info));
	info->request = r;

	statbuf = apr_pcalloc(r->pool, sizeof(*statbuf));

	resource = apr_pcalloc(r->pool, sizeof(*resource));
	resource->uri   = hsm_filename;
	resource->info  = info;
	resource->hooks = &dav_castor_hooks_repository;
	resource->pool  = r->pool;

	if (Cns_stat(hsm_filename, statbuf) < 0) {
		switch (serrno) {
		 case ENOENT:
		 case ENOTDIR:
			// Not found (404) - not a mod_dav error
			resource->type = DAV_RESOURCE_TYPE_REGULAR;
			*result_resource = resource;
			return NULL;
		 case EACCES:
			// Forbidden (403)
			http_error = HTTP_FORBIDDEN;
			break;
		 default:
			// Internal server error (500)
			http_error = HTTP_INTERNAL_SERVER_ERROR;
			break;
		}

		// Prevent mod_dav from logging random error message
		errno = 0;

		return sdav_perror(r->pool, http_error, "Cns_stat()");
	}

	resource->type = DAV_RESOURCE_TYPE_REGULAR;
	resource->exists = 1;
	resource->collection = S_ISDIR(statbuf->filemode) ? 1 : 0;
	info->filestat = statbuf;

	if (resource->collection
	 && hsm_filename[strlen(hsm_filename) - 1] != '/')
		resource->uri = apr_pstrcat(r->pool, hsm_filename, "/", NULL);

	*result_resource = resource;
	return NULL;
}

static dav_error *
get_resource(request_rec *r, const char *root_dir, const char *label,
             int use_checked_in, dav_resource **result_resource)
{
	dav_error *err;

	// Need to set NS hostname and uid/gid here
	if ((err = ns_init(r)) != NULL)
		return err;

	return get_resource_from_hsm_filename(r, r->parsed_uri.path,
	                                      result_resource);
}

static dav_error *
get_parent_resource(const dav_resource *resource,
                    dav_resource **result_parent)
{
	char *hsm_filename;

	*result_parent = NULL;

	hsm_filename = apr_pstrdup(resource->pool, resource->uri);

	if (strcmp(hsm_filename, "/") == 0)
		return NULL;

	hsm_filename = apr_pstrdup(resource->pool, dirname(hsm_filename));

	if (strcmp(hsm_filename, "/") == 0)
		return NULL;

	return get_resource_from_hsm_filename(resource->info->request,
	                                      hsm_filename, result_parent);
}

static int
is_same_resource(const dav_resource *res1, const dav_resource *res2)
{
	dav_resource_private *info1 = res1->info;
	dav_resource_private *info2 = res2->info;

	if (res1->hooks != res2->hooks)
		return 0;

	if (res1->exists && res2->exists) {
		return (info1->filestat->fileid == info2->filestat->fileid);
	} else {
		return (strcmp(res1->uri, res2->uri) == 0);
	}
}

static int
is_parent_resource(const dav_resource *res1, const dav_resource *res2)
{
	size_t len1;
	size_t len2;

	if (res1->hooks != res2->hooks)
		return 0;

	len1 = strlen(res1->uri);
	len2 = strlen(res1->uri);

	return (len2 > len1)
	    && (memcmp(res1->uri, res2->uri, len1) == 0)
	    && ('/' == res2->uri[len1]);
}

static dav_error *
open_stream(const dav_resource *resource, dav_stream_mode mode,
            dav_stream **stream)
{
	apr_pool_t *p = resource->pool;

	int          fd;
	dav_stream  *ds;
	dav_error   *err;
	const dav_castor_dir_config *conf;

	int http_status;

	conf = GET_CONFIG(resource->info->request);

	if (!check_flag(conf->allow_put))
		return dav_strerror(p, HTTP_FORBIDDEN, "PUT not allowed");

	if (!check_flag(conf->allow_delete))
		return dav_strerror(p, HTTP_FORBIDDEN,
		                    "PUT would overwrite + DELETE not allowed");

	if (NULL == conf->stager_host)
		return dav_strerror(p, HTTP_FORBIDDEN, "No stager mapping");

	if (NULL == conf->service_class)
		return dav_strerror(p, HTTP_FORBIDDEN,
		                    "No service class mapping");

	if (Cns_creat(resource->uri, 0666) < 0) {
		switch (serrno) {
		 case EACCES:
			http_status = HTTP_FORBIDDEN;
			break;
		 default:
			http_status = HTTP_INTERNAL_SERVER_ERROR;
			break;
		}

		return sdav_perror(p, http_status, "Cns_creat()");
	}

	err = do_open(resource, O_WRONLY, &fd);
	if (err != NULL)
		return err;

	ds = apr_palloc(p, sizeof(*ds));
	ds->pool = p;
	ds->fd = fd;

	*stream = ds;

	return NULL;
}

static dav_error *
close_stream(dav_stream *stream, int commit)
{
	dav_error *err;

	err = do_close(stream->pool, stream->fd);
	if (err != NULL)
		return err;

	if (!commit)
		return dav_strerror(stream->pool, HTTP_INTERNAL_SERVER_ERROR,
		                    "close_stream(): !commit");

	return NULL;
}

static dav_error *
write_stream(dav_stream *stream, const void *buf, apr_size_t bufsize)
{
	return do_write(stream->pool, stream->fd, buf, bufsize);
}

static dav_error *
set_headers(request_rec *r, const dav_resource *resource)
{
	int http_status;
	int amode = R_OK;

	// Directories also need execute permission
	if (resource->collection)
		amode |= X_OK;

	// Check for read permission
	if (Cns_access(resource->uri, amode) < 0) {
		switch (serrno) {
		 case EACCES:
			http_status = HTTP_FORBIDDEN;
			break;
		 default:
			http_status = HTTP_INTERNAL_SERVER_ERROR;
			break;
		}

		return sdav_perror(resource->pool, http_status,
		                   "Cns_access()");
	}

	ap_update_mtime(r, resource->info->filestat->mtime * 1000000);
	ap_set_last_modified(r);

	apr_table_setn(r->headers_out, "ETag",
	               resource->hooks->getetag(resource));

	if (resource->collection) {
		ap_set_content_type(r, "text/html");
	} else {
		// We DO NOT support byte-ranges due to rfio_lseek hanging
		apr_table_setn(r->headers_out, "Accept-Ranges", "none");

		ap_set_content_length(r, resource->info->filestat->filesize);
	}

	return NULL;
}

/*
 * This is called to handle GET requests.
 */
static dav_error *
deliver(const dav_resource *resource, ap_filter_t *output)
{
	if (resource->collection)
		return deliver_collection(resource, output);

	return deliver_file(resource,  output);
}

/*
 * This is called to handle MKCOL requests.
 */
static dav_error *
create_collection(dav_resource *resource)
{
	int http_status;

	if (!check_flag(GET_CONFIG(resource->info->request)->allow_mkcol))
		return dav_strerror(resource->pool, HTTP_FORBIDDEN,
		                    "MKCOL not allowed");

	if (Cns_mkdir(resource->uri, 0777) < 0) {
		switch (serrno) {
		 case ENOENT:
			http_status = HTTP_CONFLICT;
			break;
		 case EACCES:
		 case ENOTDIR:
			http_status = HTTP_FORBIDDEN;
			break;
		 case EEXIST:
			http_status = HTTP_METHOD_NOT_ALLOWED;
			break;
		 default:
			http_status = HTTP_INTERNAL_SERVER_ERROR;
			break;
		}

		return sdav_perror(resource->pool, http_status,
		                   "Cns_mkdir()");
	}

	resource->exists      = 1;
	resource->collection  = 1;

	return NULL;
}

/*
 * This is called to handle COPY requests.
 */
static dav_error *
copy_resource(const dav_resource *src, dav_resource *dst, int depth,
              dav_response **response)
{
	*response = NULL;

	return dav_strerror(src->pool, HTTP_NOT_IMPLEMENTED, "COPY");
}

/*
 * This is called to handle MOVE requests.
 */
static dav_error *
move_resource(dav_resource *src, dav_resource *dst, dav_response **response)
{
	*response = NULL;

	int http_status;

	if (!check_flag(GET_CONFIG(src->info->request)->allow_move))
		return dav_strerror(src->pool, HTTP_FORBIDDEN,
		                    "MOVE not allowed");

	if (Cns_rename(src->uri, dst->uri) < 0) {
		switch (serrno) {
		 case ENOENT:
			http_status = HTTP_CONFLICT;
			break;
		 case EACCES:
		 case ENOTDIR:
			http_status = HTTP_FORBIDDEN;
			break;
		 case EEXIST:
		 case EISDIR:
			http_status = HTTP_PRECONDITION_FAILED;
			break;
		 default:
			http_status = HTTP_INTERNAL_SERVER_ERROR;
			break;
		}

		return sdav_perror(src->pool, http_status, "Cns_rename()");
	}

	dst->exists      = 1;
	dst->collection  = src->collection;

	src->exists      = 0;
	src->collection  = 0;

	return NULL;
}

static dav_error *
remove_dir(apr_pool_t *p, const char *uri)
{
	int http_status;

	if (Cns_rmdir(uri) < 0) {
		switch (serrno) {
		 case EACCES:
			http_status = HTTP_FORBIDDEN;
			break;
		 case EEXIST:
			http_status = HTTP_NOT_IMPLEMENTED;
			break;
		 default:
			http_status = HTTP_INTERNAL_SERVER_ERROR;
			break;
		}

		return sdav_perror(p, http_status, "Cns_rmdir()");
	}

	return NULL;
}

static dav_error *
remove_file(apr_pool_t *p, const char *uri)
{
	int http_status;

	if (Cns_unlink(uri) < 0) {
		switch (serrno) {
		 case EACCES:
			http_status = HTTP_FORBIDDEN;
			break;
		 default:
			http_status = HTTP_INTERNAL_SERVER_ERROR;
			break;
		}

		return sdav_perror(p, http_status, "Cns_unlink()");
	}

	return NULL;
}

/*
 * This is called during DELETE requests and COPY / MOVE requests which
 * overwrite the destination.
 */
static dav_error *
remove_resource(dav_resource *resource, dav_response **response)
{
	*response = NULL;

	dav_error *err;

	if (!check_flag(GET_CONFIG(resource->info->request)->allow_delete))
		return dav_strerror(resource->pool, HTTP_FORBIDDEN,
		                    "DELETE not allowed");

	if (resource->collection) {
		err = remove_dir(resource->pool, resource->uri);
	} else {
		err = remove_file(resource->pool, resource->uri);
	}

	if (err != NULL)
		return err;

	resource->exists = 0;
	resource->collection = 0;

	return NULL;
}

/*
 * This is called to handle PROPFIND requests.
 */
static dav_error *
walk(const dav_walk_params *params, int depth, dav_response **response)
{
	*response = NULL;

	dav_error          *err;
	dav_walk_resource   walk_resource;

	memset(&walk_resource, 0, sizeof(walk_resource));

	walk_resource.walk_ctx  = params->walk_ctx;
	walk_resource.pool      = params->pool;
	walk_resource.resource  = params->root;

	err = params->func(&walk_resource, params->root->collection
	                                   ? DAV_CALLTYPE_COLLECTION
	                                   : DAV_CALLTYPE_MEMBER);

	if (err != NULL)
		return err;

	if (0 == depth || !params->root->collection)
		return NULL;

	if (1 == depth)
		return walk_collection(params, response);

	// Don't support depth > 1
	return NULL;
}

const char *
getetag(const dav_resource *resource)
{
	// Yes, mod_dav asks for ETags for non-existant resources
	if (!resource->exists)
		return "";

	// Weak ETag for directories
	return apr_psprintf(resource->pool, "%s\"%lx-%lx-%lx\"",
	                    resource->collection ? "W/" : "",
	                    (unsigned long)resource->info->filestat->fileid,
	                    (unsigned long)resource->info->filestat->ctime,
	                    (unsigned long)resource->info->filestat->mtime);
}

const dav_hooks_repository dav_castor_hooks_repository = {
	1,                      // handle_get
	get_resource,
	get_parent_resource,
	is_same_resource,
	is_parent_resource,
	open_stream,
	close_stream,
	write_stream,
	NULL,                   // seek_stream
	set_headers,
	deliver,
	create_collection,
	copy_resource,
	move_resource,
	remove_resource,
	walk,
	getetag
};
