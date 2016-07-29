/*
 * rfio_io.c
 *
 * Routines that interact with the rfio library.
 */


#include "mod_dav_castor.h"

// For xroot: XrdPosixExtern.h
#include "rfio_api.h"


static apr_status_t
cleanup_func(void *fd)
{
	// For xroot: XrdPosix_Close
	rfio_close((int)fd);

	return APR_SUCCESS;
}

dav_error *
do_open(const dav_resource *resource, int mode, int *fd)
{
	apr_pool_t *p = resource->pool;

	char *rfio_uri;

	dav_castor_dir_config *conf = GET_CONFIG(resource->info->request);

	// For xroot, need a redirector rather than the stager
	rfio_uri = apr_pstrcat(p, "rfio://", conf->stager_host, ":9002/",
	                       resource->uri, "?svcClass=",
	                       conf->service_class, NULL);

	// This snippet copied from rfcp.c - Activate new transfer mode
	// Don't know what it does but if rfcp does it it must be the right
	// thing to do!
	int v = RFIO_STREAM;
	rfiosetopt(RFIO_READOPT, &v, 4);

	// rfio_open() on a 0-size file won't work without O_TRUNC.
	if (O_WRONLY == mode)
		mode |= O_TRUNC;

	// For xroot: XrdPosix_Open
	*fd = rfio_open(rfio_uri, mode, 0);

	if (*fd < 0)
		return rdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                   "rfio_open()");

	apr_pool_cleanup_register(p, (const void *)*fd, cleanup_func,
	                          apr_pool_cleanup_null);

	return NULL;
}

int
do_offset_and_read(int fd, off_t fileoffset, char *buf, size_t len)
{

	// rfio_lseek() hangs - should be fine if we disable partial downloads
/*	if (rfio_lseek(fd, fileoffset, SEEK_SET) == -1)
		return -1;
*/

	// For xroot: XrdPosix_Read
	return rfio_read(fd, buf, len);
}

dav_error *
do_close(apr_pool_t *p, int fd)
{
	apr_pool_cleanup_kill(p, (const void *)fd, cleanup_func);

	// For xroot: XrdPosix_Close
	if (rfio_close(fd) < 0)
		return rdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                   "rfio_close()");

	return NULL;
}

dav_error *
do_write(apr_pool_t *p, int fd, const void *buf, size_t bufsize)
{
	// For xroot: XrdPosix_Write
	if (rfio_write(fd, (void *)buf, bufsize) == -1)
		return rdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                  "rfio_write()");

	return NULL;
}
