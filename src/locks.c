/*
 * locks.c
 *
 * Routines for handling locks. We don't support locks, but the Windows 7
 * client insists on locking during writes.
 *
 * There is nothing stateful - all LOCK and UNLOCK requests succeed and all
 * requests that require a lock act as if it exists.
 *
 * These routines implement a set of hooks that are optional in mod_dav, but
 * are implemented to support Windows 7.
 *
 * See `struct dav_hooks_locks' in `mod_dav.h' for documentation.
 */

#include "mod_dav_castor.h"


// request_rec is needed by get_locks
struct dav_lockdb_private {
	request_rec *r;
};


static const char *
get_supportedlock(const dav_resource *resource)
{
	return NULL;
}

static dav_error *
parse_locktoken(apr_pool_t *p, const char *char_token,
                dav_locktoken **locktoken_p)
{
	*locktoken_p = NULL;

	return NULL;
}

static const char *
format_locktoken(apr_pool_t *p, const dav_locktoken *locktoken)
{
	// return the same token for all locks
	return "dummy-lock-token";
}

static int
compare_locktoken(const dav_locktoken *lt1, const dav_locktoken *lt2)
{
	// all tokens are the same
	return 0;
}

static dav_error *
open_lockdb(request_rec *r, int ro, int force, dav_lockdb **lockdb)
{
	dav_lockdb_private *info;

	info = apr_palloc(r->pool, sizeof(*info));
	info->r = r;

	*lockdb = apr_palloc(r->pool, sizeof(**lockdb));
	(*lockdb)->hooks = &dav_castor_hooks_locks;
	(*lockdb)->ro = ro;
	(*lockdb)->info = info;

	return NULL;
}

static void
close_lockdb(dav_lockdb *lockdb)
{
	return;
}

static dav_error *
remove_locknull_state(dav_lockdb *lockdb, const dav_resource *resource)
{
	return NULL;
}

static dav_error *
create_lock(dav_lockdb *lockdb, const dav_resource *resource, dav_lock **lock)
{
	*lock = apr_pcalloc(resource->pool, sizeof(**lock));
	(*lock)->rectype = DAV_LOCKREC_DIRECT;
	(*lock)->is_locknull = !resource->exists;

	return NULL;
}

static dav_error *
get_locks(dav_lockdb *lockdb, const dav_resource *resource, int calltype,
          dav_lock **locks)
{
	// If there is an `If:' header in the request, assume it is a lock
	// precondition so return the lock.
	if (apr_table_get(lockdb->info->r->headers_in, "If") != NULL)
		return create_lock(lockdb, resource, locks);

	*locks = NULL;

	return NULL;
}

static dav_error *
find_lock(dav_lockdb *lockdb, const dav_resource *resource,
          const dav_locktoken *locktoken, int partial_ok, dav_lock **lock)
{
	// Always find a lock if we look for it
	return create_lock(lockdb, resource, lock);
}

static dav_error *
has_locks(dav_lockdb *lockdb, const dav_resource *resource,
          int *locks_present)
{
	*locks_present = 0;

	return NULL;
}

static dav_error *
append_locks(dav_lockdb *lockdb, const dav_resource *resource,
             int make_indirect, const dav_lock *lock)
{
	return NULL;
}

static dav_error *
remove_lock(dav_lockdb *lockdb, const dav_resource *resource,
            const dav_locktoken *locktoken)
{
	return NULL;
}

static dav_error *
refresh_locks(dav_lockdb *lockdb, const dav_resource *resource,
              const dav_locktoken_list *ltl, time_t new_time,
              dav_lock **locks)
{
	*locks = NULL;

	return NULL;
}


const dav_hooks_locks dav_castor_hooks_locks = {
	get_supportedlock,
	parse_locktoken,
	format_locktoken,
	compare_locktoken,
	open_lockdb,
	close_lockdb,
	remove_locknull_state,
	create_lock,
	get_locks,
	find_lock,
	has_locks,
	append_locks,
	remove_lock,
	refresh_locks,
	NULL,                   // lookup_resource
	NULL                    // ctx
};
