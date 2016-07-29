/*
 * bucket_castor.c
 *
 * Routines that implement a bucket type for reading CASTOR files.
 *
 * An introduction to buckets and brigades can be found at:
 * http://www.apachetutor.org/dev/brigades
 *
 * These routines are based on the routines that implement a local file bucket
 * in `buckets/apr_buckets_file.c' in apr-util.
 */

#include "mod_dav_castor.h"


#define MAX_BUCKET_SIZE (0x10000000) // 256 MB


typedef struct {
	apr_bucket_refcount     refcount;
	int                     fd;
	apr_pool_t             *readpool;
} apr_bucket_castor;

static const apr_bucket_type_t apr_bucket_type_castor;


static void
castor_bucket_destroy(void *data)
{
	apr_bucket_castor *f = data;

	if (apr_bucket_shared_destroy(f))
		// No need to close the file here; it will get done
		// automatically when the pool gets cleaned up
		apr_bucket_free(f);
}

static apr_status_t
castor_bucket_read(apr_bucket *e, const char **str, size_t *len,
                   apr_read_type_e block)
{
	*str = NULL;

	apr_bucket_castor  *a  = e->data;
	int                 fd = a->fd;
	apr_bucket         *b;
	char               *buf;
	apr_status_t        rv = APR_SUCCESS;
	size_t              filelength = e->length;
	off_t               fileoffset = e->start;
	ssize_t             readsize;

	*len = (filelength > APR_BUCKET_BUFF_SIZE)
	     ? APR_BUCKET_BUFF_SIZE
	     : filelength;
	buf = apr_bucket_alloc(*len, e->list);

	// Offset and read
	readsize = do_offset_and_read(fd, fileoffset, buf, *len);
	if (-1 == readsize) {
		apr_bucket_free(buf);
		return APR_EGENERAL;
	}
	*len = readsize;
	filelength -= *len;
	if (0 == readsize)
		rv = APR_EOF;

	// Change the current bucket to refer to what we read, even if we read
	// nothing because we hit EOF.
	apr_bucket_heap_make(e, buf, *len, apr_bucket_free);

	// If we have more to read from the file, then create another bucket
	if (filelength > 0 && rv != APR_EOF) {
		// For efficiency, we can just build a new apr_bucket struct
		// to wrap around the existing file bucket
		b = apr_bucket_alloc(sizeof(*b), e->list);
		b->start  = fileoffset + (*len);
		b->length = filelength;
		b->data   = a;
		b->type   = &apr_bucket_type_castor;
		b->free   = apr_bucket_free;
		b->list   = e->list;
		APR_BUCKET_INSERT_AFTER(e, b);
	} else {
		castor_bucket_destroy(a);
	}

	*str = buf;
	return rv;
}

static apr_bucket *
bucket_castor_make(apr_bucket *b, int fd, off_t offset, apr_size_t len,
                   apr_pool_t *p)
{
	apr_bucket_castor *f;

	f = apr_bucket_alloc(sizeof(*f), b->list);
	f->fd = fd;
	f->readpool = p;

	b = apr_bucket_shared_make(b, f, offset, len);
	b->type = &apr_bucket_type_castor;

	return b;
}

static apr_bucket*
bucket_castor_create(int fd, off_t offset, size_t len, apr_pool_t *p,
                     apr_bucket_alloc_t *list)
{
	apr_bucket *b = apr_bucket_alloc(sizeof(*b), list);

	APR_BUCKET_INIT(b);
	b->free = apr_bucket_free;
	b->list = list;
	return bucket_castor_make(b, fd, offset, len, p);
}

static apr_status_t
castor_bucket_setaside(apr_bucket *data, apr_pool_t *reqpool)
{
	apr_bucket_castor *a = (apr_bucket_castor*)data->data;

	if (!apr_pool_is_ancestor(a->readpool, reqpool))
		a->readpool = reqpool;

	return APR_SUCCESS;
}


static const apr_bucket_type_t apr_bucket_type_castor = {
	"CASTOR", 5, APR_BUCKET_DATA,
	castor_bucket_destroy,
	castor_bucket_read,
	castor_bucket_setaside,
	apr_bucket_shared_split,
	apr_bucket_shared_copy
};


/*
 * This function is based on `apr_brigade_insert_file' in `apr_brigade.c'.
 */
apr_bucket *
brigade_insert_castor(apr_bucket_brigade *bb, int fd, off_t start,
                      size_t length, apr_pool_t *p)
{
	apr_bucket *e;

	if (sizeof(apr_off_t) == sizeof(apr_size_t)
	 || length < MAX_BUCKET_SIZE) {
		e = bucket_castor_create(fd, start, length, p,
		                         bb->bucket_alloc);
	} else {
		// Several buckets are needed
		e = bucket_castor_create(fd, start, MAX_BUCKET_SIZE, p,
		                         bb->bucket_alloc);

		while (length > MAX_BUCKET_SIZE) {
			apr_bucket *ce;
			apr_bucket_copy(e, &ce);
			APR_BRIGADE_INSERT_TAIL(bb, ce);
			e->start += MAX_BUCKET_SIZE;
			length   -= MAX_BUCKET_SIZE;
		}

		e->length = length; // Resize just the last bucket
	}

	APR_BRIGADE_INSERT_TAIL(bb, e);
	return e;
}
