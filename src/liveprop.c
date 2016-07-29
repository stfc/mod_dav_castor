/*
 * liveprop.c
 *
 * Routines for handling live properties. We support the getcontentlength
 * (file size) and getlastmodified (mtime) properties.
 */

#include "mod_dav_castor.h"

#include "Cns_api.h"


static const dav_hooks_liveprop dav_castor_hooks_liveprop;


static const char * const namespace_uris[] = {
	"DAV:",
	NULL
};

enum {
	DAV_CASTOR_URI_DAV,
};

static const dav_liveprop_spec dav_castor_props[] = {
	{ DAV_CASTOR_URI_DAV, "getcontentlength",
	  DAV_PROPID_getcontentlength, 0 },
	{ DAV_CASTOR_URI_DAV, "getlastmodified",
	  DAV_PROPID_getlastmodified, 0 },
	{ DAV_CASTOR_URI_DAV, "getetag",
	  DAV_PROPID_getetag, 0 },
	{ 0 }
};

static const dav_liveprop_group dav_castor_liveprop_group =
{
	dav_castor_props,
	namespace_uris,
	&dav_castor_hooks_liveprop
};


static dav_prop_insert
insert_prop(const dav_resource *resource, int propid, dav_prop_insert what,
            apr_text_header *phdr)
{
	const dav_liveprop_spec *info;
	const char *s;
	char *value;
	int global_ns;

	if (!resource->exists)
		return DAV_PROP_INSERT_NOTDEF;

	switch (propid) {
	 case DAV_PROPID_getcontentlength:
		if (resource->collection)
			return DAV_PROP_INSERT_NOTDEF;

		value = apr_psprintf(resource->pool, "%lu",
				     (unsigned long)resource->info->filestat
		                                                  ->filesize);
		break;
	 case DAV_PROPID_getetag:
		value = resource->hooks->getetag(resource);
		break;
	 case DAV_PROPID_getlastmodified:
		value = apr_pcalloc(resource->pool, DAV_TIMEBUF_SIZE);
		apr_rfc822_date(value,
		                resource->info->filestat->mtime * 1000000);
		break;
	 default:
		return DAV_PROP_INSERT_NOTSUPP;
	}

	global_ns = dav_get_liveprop_info(propid, &dav_castor_liveprop_group,
	                                  &info);

	switch (what) {
	 case DAV_PROP_INSERT_NAME:
		s = apr_psprintf(resource->pool, "<lp%d:%s/>\n", global_ns,
		                 info->name);
		break;
	 case DAV_PROP_INSERT_VALUE:
		s = apr_psprintf(resource->pool, "<lp%d:%s>%s</lp%d:%s>\n",
		                 global_ns, info->name, value, global_ns,
		                 info->name);
		break;
	 case DAV_PROP_INSERT_SUPPORTED:
		s = apr_psprintf(resource->pool,
		                 "<D:supported-live-property D:name=\"%s\""
		                 " D:namespace=\"%s\"/>\n", info->name,
		                 namespace_uris[info->ns]);
		break;
	 default:
		return -1;
	}

	apr_text_append(resource->pool, phdr, s);

	return what;
}

static int
is_writable(const dav_resource *resource, int propid)
{
	// No writable properties
	return 0;
}

static dav_error *
patch_validate(const dav_resource *resource, const apr_xml_elem *elem,
               int operation, void **context, int *defer_to_dead)
{
	return NULL;
}

static dav_error *
patch_exec(const dav_resource *resource, const apr_xml_elem *elem,
           int operation, void *context, dav_liveprop_rollback **rollback_ctx)
{
	*rollback_ctx = NULL;

	return NULL;
}

static void
patch_commit(const dav_resource *resource, int operation, void *context,
             dav_liveprop_rollback *rollback_ctx)
{
	return;
}

static dav_error *
patch_rollback(const dav_resource *resource, int operation, void *context,
               dav_liveprop_rollback *rollback_ctx)
{
	return NULL;
}

static int
find_liveprop(const dav_resource *resource, const char *ns_uri,
              const char *name, const dav_hooks_liveprop **hooks)
{
	if (resource->hooks != &dav_castor_hooks_repository)
		return 0;

	return dav_do_find_liveprop(ns_uri, name, &dav_castor_liveprop_group,
	                            hooks);
}

static void
insert_all_liveprops(request_rec *r, const dav_resource *resource,
                     dav_prop_insert what, apr_text_header *phdr)
{
	if (resource->hooks != &dav_castor_hooks_repository)
		return;

	if (!resource->exists)
		return;

	insert_prop(resource, DAV_PROPID_getcontentlength, what, phdr);
	insert_prop(resource, DAV_PROPID_getlastmodified, what, phdr);
	insert_prop(resource, DAV_PROPID_getetag, what, phdr);
}

void
init_liveprops()
{
	dav_hook_find_liveprop(find_liveprop, NULL, NULL, APR_HOOK_MIDDLE);

        dav_hook_insert_all_liveprops(insert_all_liveprops, NULL, NULL,
	                              APR_HOOK_MIDDLE);	
}


static const dav_hooks_liveprop dav_castor_hooks_liveprop = {
	insert_prop,
	is_writable,
	namespace_uris,
	patch_validate,
	patch_exec,
	patch_commit,
	patch_rollback,
	NULL                           // ctx
};
