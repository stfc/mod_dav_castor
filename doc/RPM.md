# Creating An RPM

These instructions are inteded for use on 64-bit RedHat Enterprise Linux 5/6 and their derivatives (e.g. Scientific Linux).


## Prerequisits

 - Install the `http-devel`, `castor-devel`, `gcc`, `make` & `rpmbuild` packages.
 - Name the source directory `mod_dav_castor-<version>`.
 - Create a tarball of the directory named `mod_dav_castor-<version>.tar.gz`.


## Creating an RPM build environment

To create an RPM build environment in `~/rpmbuild/`:

```sh
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
echo %_topdir ~/rpmbuild > ~/.rpmmacros
```


## Creating the RPM

 - Put the tarball in the `~/rpmbuld/SOURCES` directory.
 - Copy the spec file from the `rpm` directory of the source tree to the `~/rpmbuild/SPECS` directory. Make sure the `Version` field in the SPEC file matches the tarball version; you may also want to change the `Release` field.
 - Run `rpmbuild -ba <specfile>` to create the binary and source RPMs.
