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
    jzeng9/envoy-dev:latest /bin/bash /source/dosa-build-docker.sh
docker build -t csci2952fmicrocow/test-storage-envoy:latest .

if onmaster; then
    # Avoid `set -x` leaking secret info into Travis logs
    set +x
    echo "+docker login..."
    docker login -u "${DOCKER_USERNAME}" -p "${DOCKER_PASSWORD}"
    set -x
    docker push csci2952fmicrocow/test-storage-envoy:latest
else
    echo "not on master; not pushing to Docker Hub"
fi
