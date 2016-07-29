/*
 * dav_errors.c
 *
 * Wrapper functions for `dav_new_error()'.
 */

#include "mod_dav_castor.h"

#include "serrno.h"
#include "rfio_errno.h"


/*
 * Create a new error with message str.
 */
dav_error *
dav_strerror(apr_pool_t *p, int http_status, const char *str)
{
	// Prevent erroneous error messages in logs
	errno = 0;

	return dav_new_error(p, http_status, 0, str);
}

/*
 * Creates a new error using the value errno.
 */
dav_error *
dav_perror(apr_pool_t *p, int http_status, const char *func)
{
	const char *str;

	str = apr_psprintf(p, "%s: %s [errno=%d]", func, strerror(errno),
	                   errno);

	return dav_strerror(p, http_status, str);
}

/*
 * Creates a new error using the value of serrno. (for stager / ns errors)
 */
dav_error *
sdav_perror(apr_pool_t *p, int http_status, const char *func)
{
	const char *str;

	str = apr_psprintf(p, "%s: %s [serrno=%d]", func, sstrerror(serrno),
	                   serrno);

	return dav_strerror(p, http_status, str);
}

/*
 * Creates a new error using the value of serrno, rfio_errno or errno as
 * appropriate. (for rfio errors)
 */
dav_error *
rdav_perror(apr_pool_t *p, int http_status, const char *func)
{
	const char *str;

	str = apr_psprintf(p, "%s: %s [serrno=%d, rfio_errno=%d, errno=%d]",
	                   func, rfio_serror(), serrno, rfio_errno, errno);

	return dav_strerror(p, http_status, str);
}

