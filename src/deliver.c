#include "mod_dav_castor.h"

#include "serrno.h"
#include "Cns_api.h"


static dav_error *
get_collection_callback(dav_walk_resource *wres, int calltype)
{
	const dav_resource     *resource = wres->resource;
	const dav_walker_ctx   *walk_ctx = wres->walk_ctx;

	void *output_filters = resource->info->request->output_filters;

	ap_fprintf(output_filters, walk_ctx->bb, "%8s %04o %11s %11s ",
	           is_migrated(resource) ? "MIGRATED" : "",
	           resource->info->filestat->filemode & 07777,
	           get_user(resource->info->filestat->uid),
	           get_group(resource->info->filestat->gid));

	// No size for directories
	if (resource->collection) {
		ap_fprintf(output_filters, walk_ctx->bb, "%11s ", "");
	} else {
		ap_fprintf(output_filters, walk_ctx->bb, "%11lu ",
		           (unsigned long)resource->info->filestat->filesize);
	}

	ap_fprintf(output_filters, walk_ctx->bb, "<a href=\"%s\">%s</a>\n",
	           resource->uri,
	           resource->uri + strlen(walk_ctx->w.root->uri));

	return NULL;
}

dav_error *
deliver_collection(const dav_resource *resource, ap_filter_t *output)
{
	apr_bucket_brigade *bb;
	apr_bucket         *bkt;

	dav_walker_ctx  walk_ctx;
	dav_response   *response;

	bb = apr_brigade_create(resource->pool, output->c->bucket_alloc);

	memset(&walk_ctx, 0, sizeof(walk_ctx));
	walk_ctx.w.walk_type = DAV_WALKTYPE_NORMAL;
	walk_ctx.w.func      = get_collection_callback;
	walk_ctx.w.walk_ctx  = &walk_ctx;
	walk_ctx.w.pool      = resource->pool;
	walk_ctx.w.root      = resource;
	walk_ctx.bb          = bb;

	ap_fprintf(output, bb, "<h1>%s</h1>\n<pre>\n", resource->uri);

	walk_collection(&walk_ctx.w, &response);

	ap_fprintf(output, bb, "</pre>\n");

	bkt = apr_bucket_eos_create(output->c->bucket_alloc);
	APR_BRIGADE_INSERT_TAIL(bb, bkt);
	ap_pass_brigade(output, bb);

	return NULL;
}

dav_error *
deliver_file(const dav_resource *resource, ap_filter_t *output)
{
	apr_bucket_brigade   *bb;
	apr_bucket           *bkt;

	int fd;
	dav_castor_dir_config *conf;
	dav_error *err;

	conf = GET_CONFIG(resource->info->request);

	if (NULL == conf->stager_host)
		return dav_strerror(resource->pool, HTTP_FORBIDDEN,
		                    "No stager specified");

	if (NULL == conf->service_class)
		return dav_strerror(resource->pool, HTTP_FORBIDDEN,
		                    "No service class specified");

	if (0 == resource->info->filestat->filesize)
		return NULL;

	if (check_flag(conf->tape_recall))
		if ((err = do_stage_prepareToGet(resource)) != NULL)
			return err;

	if ((err = do_stage_filequery(resource)) != NULL)
		return err;

	if ((err = do_open(resource, O_RDONLY, &fd)) != NULL)
		return err;

	bb = apr_brigade_create(resource->pool, output->c->bucket_alloc);

	brigade_insert_castor(bb, fd, 0, resource->info->filestat->filesize,
	                      resource->pool);

	bkt = apr_bucket_eos_create(output->c->bucket_alloc);
	APR_BRIGADE_INSERT_TAIL(bb, bkt);
	ap_pass_brigade(output, bb);

	return NULL;
}
