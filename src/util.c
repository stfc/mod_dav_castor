#include "mod_dav_castor.h"

#include <pwd.h>
#include <grp.h>

#include "Cpwd.h"
#include "Cgrp.h"
#include "Cns_api.h"


int
is_migrated(const dav_resource *resource)
{
	if (!resource->exists)
		return 0;

	if ('m' == resource->info->filestat->status)
		return 1;

	return  0;
}

const char *
get_user(uid_t uid)
{
	struct passwd *pwd;

	pwd = Cgetpwuid(uid);

	if (NULL == pwd)
		return "?";

	return pwd->pw_name;
}

const char *
get_group(gid_t gid)
{
	struct group *grp;

	grp = Cgetgrgid(gid);

	if (NULL == grp)
		return "?";

	return grp->gr_name;
}

uid_t
get_uid(const char *user)
{
	struct passwd *pwd;

	pwd = Cgetpwnam(user);

	if (NULL == pwd)
		return -1;

	return pwd->pw_uid;
}

gid_t
get_gid(const char *group)
{
	struct group *grp;

	grp = Cgetgrnam(group);

	if (NULL == grp)
		return -1;

	return grp->gr_gid;
}

int
check_flag(const char *flag)
{
	return (NULL != flag && strcasecmp(flag, "Yes") == 0);
}
