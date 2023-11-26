# Rough release process

0. Update `CHANGES` & commit!
1. `export VERSION=2...`
2. `sed -i -e "/^AC_INIT/s,\[.*\],[$VERSION],g" configure.ac`
3. `git commit -s -m "Release ${VERSION}" configure.ac`
4. `git tag -s v${VERSION} -m "$VERSION"`
5. `docker rmi radvd-autogen:latest`
6. `./autogen-container.sh`
7. `./configure`
8. `make packages`
9. `gh release create v${VERSION} radvd-${VERSION}.tar.{xz,gz}{,.asc,.sha256,.sha512}`


## Tools
https://cli.github.com/
