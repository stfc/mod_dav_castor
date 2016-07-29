#include <pwd.h>

#include "mod_dav_castor.h"

#include "Cpwd.h"
#include "Cns_api.h"


static dav_error *
init_hostname(apr_pool_t *p, const char *nameserver)
{
	struct Cns_api_thread_info *thip;

	if (Cns_apiinit(&thip) < 0)
		return sdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                   "Cns_apiinit()");

	if (NULL == nameserver)
		return dav_strerror(p, HTTP_INTERNAL_SERVER_ERROR,
		                    "No nameserver specified");

	strcpy(thip->defserver, nameserver);

	return NULL;
}

static dav_error *
init_umask(apr_pool_t *p, const char *umask_str)
{
	mode_t  umask;
	char   *endptr;

	if (NULL == umask_str) {
		umask = 0022; // sane default
	} else {
		umask = strtol(umask_str, &endptr, 8);
		if ('\0' == *umask_str || '\0' != *endptr)
			return dav_strerror(p, HTTP_INTERNAL_SERVER_ERROR,
					    "Invalid umask");
	}

	Cns_umask(umask);

	return NULL;
}

static dav_error *
init_uid(apr_pool_t *p, const char *username)
{
	struct passwd *pwd;

	if (NULL == username)
		return dav_strerror(p, HTTP_FORBIDDEN, "No username");

	// Thread-safe getpwnam()
	pwd = Cgetpwnam(username);

	if (NULL == pwd)
		return dav_strerror(p, HTTP_FORBIDDEN,
		                    "Cannot determine uid/gid from username");

	if (Cns_setid(pwd->pw_uid, pwd->pw_gid) < 0)
		return sdav_perror(p, HTTP_INTERNAL_SERVER_ERROR,
		                   "Cns_setid()");

	return NULL;
}

dav_error *
ns_init(const request_rec *r)
{
	apr_pool_t *p = r->pool;

	dav_castor_dir_config *conf;
	dav_error             *err;

	conf = GET_CONFIG(r);

	if ((err = init_hostname(p, conf->nameserver)) != NULL)
		return err;

	if ((err = init_umask(p, conf->umask)) != NULL)
		return err;

	if ((err = init_uid(p, r->user)) != NULL)
		return err;

	return NULL;
}
