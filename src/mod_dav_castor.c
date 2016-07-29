#include "mod_dav_castor.h"


static void *
create_dir_config(apr_pool_t *p, char *dir)
{
	return apr_pcalloc(p, sizeof(dav_castor_dir_config));
}

static void *
merge_dir_config(apr_pool_t *p, void *base_conf, void *new_conf)
{
#define MERGE(x) (conf->x = (child->x != NULL) ? child->x : parent->x)

	dav_castor_dir_config *parent = base_conf;
	dav_castor_dir_config *child = new_conf;
	dav_castor_dir_config *conf;

	conf = apr_pcalloc(p, sizeof(*conf));

	MERGE(gridmapfile);
	MERGE(nameserver);
	MERGE(stager_host);
	MERGE(service_class);
	MERGE(umask);
	MERGE(allow_put);
	MERGE(allow_delete);
	MERGE(allow_move);
	MERGE(allow_mkcol);
	MERGE(tape_recall);

	return conf;
}


static const command_rec cmds[] = {
	AP_INIT_TAKE1("CastorStager", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   stager_host),
	              ACCESS_CONF, "Stager hostname"),

	AP_INIT_TAKE1("CastorGridmapfile", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   gridmapfile),
	              ACCESS_CONF, "Location of the grid-mapfile"),

	AP_INIT_TAKE1("CastorNameserver", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   nameserver),
	              ACCESS_CONF, "Nameserver hostname"),

	AP_INIT_TAKE1("CastorServiceClass", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   service_class),
	              ACCESS_CONF, "Service class"),

	AP_INIT_TAKE1("CastorUmask", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   umask),
	              ACCESS_CONF, "Umask"),

	AP_INIT_TAKE1("CastorAllowPut", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   allow_put),
	              ACCESS_CONF, "Allow PUT requests?"),

	AP_INIT_TAKE1("CastorAllowDelete", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   allow_delete),
	              ACCESS_CONF, "Allow DELETE requests (+ overwrites)?"),

	AP_INIT_TAKE1("CastorAllowMove", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   allow_move),
	              ACCESS_CONF, "Allow MOVE requests?"),

	AP_INIT_TAKE1("CastorAllowMkcol", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   allow_mkcol),
	              ACCESS_CONF, "Allow MKCOL requests?"),

	AP_INIT_TAKE1("CastorTapeRecall", ap_set_string_slot,
	              (void *)APR_OFFSETOF(dav_castor_dir_config,
	                                   tape_recall),
	              ACCESS_CONF, "Automatic tape recall on GET requests?"),

	{ NULL }
};

static const dav_provider hooks = {
	&dav_castor_hooks_repository,
	&dav_castor_hooks_propdb,
	&dav_castor_hooks_locks,
	NULL,                   // vsn
	NULL,                   // binding
	NULL,                   // search
	NULL                    // ctx
};

static void
register_hooks(apr_pool_t *p)
{
	init_liveprops();

	dav_register_provider(p, "Castor", &hooks);
}

module AP_MODULE_DECLARE_DATA dav_castor_module = {
	STANDARD20_MODULE_STUFF,
	create_dir_config,
	merge_dir_config,
	NULL,                   // create_server_config
	NULL,                   // merge_server_config
	cmds,
	register_hooks
};
