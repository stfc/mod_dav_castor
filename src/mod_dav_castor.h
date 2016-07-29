#include <apr.h>
#include <apr_strings.h>

#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>

#include <mod_dav.h>


struct dav_resource_private {
	request_rec            *request;
	struct Cns_filestat    *filestat;
};

typedef struct {
        const char *stager_host;
        const char *gridmapfile;
        const char *nameserver;
	const char *service_class;
	const char *umask;
	const char *allow_put;
	const char *allow_delete;
	const char *allow_move;
	const char *allow_mkcol;
	const char *tape_recall;
} dav_castor_dir_config;


#define GET_CONFIG(r) ((dav_castor_dir_config *) \
	 ap_get_module_config(r->per_dir_config, &dav_castor_module))


/* From mod_dav_castor.c */

extern module AP_MODULE_DECLARE_DATA dav_castor_module;


/* From repository.c */

extern const dav_hooks_repository dav_castor_hooks_repository;


/* From propdb.c */

extern const dav_hooks_propdb dav_castor_hooks_propdb;


/* From locks.c*/

extern const dav_hooks_locks dav_castor_hooks_locks;


/* From bucket_castor.c */

extern apr_bucket *
brigade_insert_castor(apr_bucket_brigade *bb, int fd, off_t start,
                      size_t length, apr_pool_t *p);


/* From liveprops.c */

extern void
init_liveprops();


/* From ns_init.c */

extern dav_error *
ns_init(const request_rec *r);


/* From util.c */

extern int
is_migrated(const dav_resource *resource);

extern const char *
get_user(uid_t uid);

extern const char *
get_group(gid_t gid);

extern uid_t
get_uid(const char *user);

extern gid_t
get_gid(const char *group);

int
check_flag(const char *flag);


/* From walk.c */

extern dav_error *
walk_collection(const dav_walk_params *params, dav_response **response);


/* from stage.c */

extern dav_error *
do_stage_prepareToGet(const dav_resource *resource);

extern dav_error *
do_stage_filequery(const dav_resource *resource);


/* From rfio_io.c */

extern dav_error *
do_open(const dav_resource *resource, int mode, int *fd);

extern int
do_offset_and_read(int fd, off_t fileoffset, char *buf, size_t len);

extern dav_error *
do_close(apr_pool_t *p, int fd);

extern dav_error *
do_write(apr_pool_t *p, int fd, const void *buf, size_t bufsize);


/* From dav_errors.c */

extern dav_error *
dav_strerror(apr_pool_t *p, int http_status, const char *str);

extern dav_error *
dav_perror(apr_pool_t *p, int http_status, const char *func);

extern dav_error *
sdav_perror(apr_pool_t *p, int http_status, const char *func);

extern dav_error *
rdav_perror(apr_pool_t *p, int http_status, const char *func);


/* From delivery.c */

dav_error *
deliver_file(const dav_resource *resource, ap_filter_t *output);

dav_error *
deliver_collection(const dav_resource *resource, ap_filter_t *output);
