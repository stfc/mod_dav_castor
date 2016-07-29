/*
 * propdb.c
 *
 * Routines for dealing with dead properties. We don't support dead properties
 * so they are just stubs.
 *
 * These routines implement a set of hooks required by mod_dav.
 *
 * See `struct dav_hooks_propdb' in `mod_dav.h' for documentation.
 */

#include "mod_dav_castor.h"


static dav_error *
propdb_open(apr_pool_t *p, const dav_resource *resource, int ro, dav_db **pdb)
{
	// If pdb is NULL, mod_dav will whinge during PROPPATCH requests
	*pdb = apr_pcalloc(p, sizeof(int));

	return NULL;
}

static void
propdb_close(dav_db *db)
{
	return;
}

static dav_error *
propdb_define_namespaces(dav_db *db, dav_xmlns_info *xi)
{
	return NULL;
}

static dav_error *
propdb_output_value(dav_db *db, const dav_prop_name *name, dav_xmlns_info *xi,
                    apr_text_header *phdr, int *found)
{
	*found = 0;

	return NULL;
}

static dav_error *
propdb_map_namespaces(dav_db *db, const apr_array_header_t *namespaces,
                      dav_namespace_map **mapping)
{
	*mapping = NULL;

	return NULL;
}

static dav_error *
propdb_store(dav_db *db, const dav_prop_name *name, const apr_xml_elem *elem,
             dav_namespace_map *mapping)
{
	return NULL;
}

static dav_error *
propdb_remove(dav_db *db, const dav_prop_name *name)
{
	return NULL;
}

static int
propdb_exists(dav_db *db, const dav_prop_name *name)
{
	return 0;
}

static dav_error *
propdb_first_name(dav_db *db, dav_prop_name *pname)
{
	// mod_dav will enter an infinite loop if we don't NULL these
	pname->ns   = NULL;
	pname->name = NULL;

	return NULL;
}

static dav_error *
propdb_next_name(dav_db *db, dav_prop_name *pname)
{
	// mod_dav will enter an infinite loop if we don't NULL these
	pname->ns   = NULL;
	pname->name = NULL;

	return NULL;
}

static dav_error *
propdb_get_rollback(dav_db *db, const dav_prop_name *name,
                    dav_deadprop_rollback **prollback)
{
	*prollback = NULL;

	return NULL;
}

static dav_error *
propdb_apply_rollback(dav_db *db, dav_deadprop_rollback *rollback)
{
	return NULL;
}


const dav_hooks_propdb dav_castor_hooks_propdb = {
	propdb_open,
	propdb_close,
	propdb_define_namespaces,
	propdb_output_value,
	propdb_map_namespaces,
	propdb_store,
	propdb_remove,
	propdb_exists,
	propdb_first_name,
	propdb_next_name,
	propdb_get_rollback,
	propdb_apply_rollback,
	NULL                    // ctx
};
