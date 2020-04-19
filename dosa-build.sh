#!/bin/sh

set -ex

env | grep DOSA | sort

# Are we on master?
ONMASTER=

if [ \( "$DOSA_BRANCH" = "master" \) -a \( "$DOSA_PULL_REQUEST" = "false" \) ]; then
    ONMASTER=yes
fi

# Syntactic sugar really...
onmaster () {
    test -n "$ONMASTER"
}

if onmaster; then
    git checkout ${DOSA_BRANCH}
fi

# Perform the build
git submodule update --init
mkdir -p dist
docker run -it --rm -v `pwd`:/source -w /source \
    envoyproxy/envoy-build:latest /bin/bash /source/dosa-build-docker.sh
# docker build -t CS2552F-DOSA/Envoy-Filters:latest .
# docker tag CS2552F-DOSA/Envoy-Filters:latest CS2552F-DOSA/Envoy-Filters:$COMMIT
docker build -t  tiancanyu/envoy-filters:latest .
docker push  tiancanyu/envoy-filters:latest

if onmaster; then
    # Avoid `set -x` leaking secret info into Travis logs
    set +x
    echo "+docker login..."
    docker login -u "${DOCKER_USERNAME}" -p "${DOCKER_PASSWORD}"
    set -x
    docker push CS2552F-DOSA/Envoy-Filters:latest
    docker push CS2552F-DOSA/Envoy-Filters:$COMMIT
else
    echo "not on master; not pushing to Docker Hub"
fi
