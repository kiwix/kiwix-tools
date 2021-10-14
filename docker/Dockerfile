# declare build option ARCH if not using buildx
ARG ARCH=
# declare version to build image for
ARG VERSION=

# alpine is a multi-arch image
FROM alpine:3
LABEL org.opencontainers.image.source https://github.com/openzim/kiwix-tools

# TARGETARCH is injected by buildx
ARG TARGETARCH
ARG VERSION
ARG ARCH

# defaulting to amd64
RUN echo "amd64" > /etc/docker_arch
# use --build-arg ARCH if it was provided (not using buildx)
RUN if [ ! -z "$ARCH" ] ; then echo "$ARCH" > /etc/docker_arch ; fi
# use $TARGETARCH if provided.
RUN if [ ! -z "$TARGETARCH" ] ; then echo "$TARGETARCH" > /etc/docker_arch ; fi

# decide which kiwix arch to download later (`armhf` for all arm* and x86_64 otherwise)
RUN if [ $(cut -c 1-3 /etc/docker_arch) = "arm" ] ; then echo "armhf" > /etc/kiwix_arch ; else echo "x86_64" > /etc/kiwix_arch ; fi

# dumb-init to use as entrypoint so it can forward signals to our CMD
RUN wget -q -O /usr/bin/dumb-init https://github.com/Yelp/dumb-init/releases/download/v1.2.5/dumb-init_1.2.5_x86_64 && \
    chmod +x /usr/bin/dumb-init

# Install kiwix-tools
RUN url="http://mirror.download.kiwix.org/release/kiwix-tools/kiwix-tools_linux-$(cat /etc/kiwix_arch)-$VERSION.tar.gz" && \
    echo "URL: $url" && \
    wget -q -O - $url | tar -xz && \
    mv kiwix-tools*/kiwix-* /usr/local/bin && \
    rm -r kiwix-tools*

# expose kiwix-serve default port
EXPOSE 80

ENTRYPOINT ["/usr/bin/dumb-init", "--"]
CMD ["/bin/sh", "-c", "echo 'Welcome to kiwix-tools! The following binaries are available:' && ls /usr/local/bin/"]
