FROM alpine:3.18
LABEL org.opencontainers.image.source https://github.com/openzim/kiwix-tools

# TARGETPLATFORM is injected by docker build
ARG TARGETPLATFORM
ARG VERSION

RUN set -e && \
    apk --no-cache add dumb-init curl && \
    echo "TARGETPLATFORM: $TARGETPLATFORM" && \
    if [ "$TARGETPLATFORM" = "linux/386" ]; then ARCH="i586"; \
    # linux/arm64/v8 points to linux/arm64
    elif [ "$TARGETPLATFORM" = "linux/arm64/v8" \
        -o "$TARGETPLATFORM" = "linux/arm64" ]; then ARCH="aarch64"; \
    # linux/arm translates to linux/arm/v7
    elif [ "$TARGETPLATFORM" = "linux/arm/v7" ]; then ARCH="armv8"; \
    elif [ "$TARGETPLATFORM" = "linux/arm/v6" ]; then ARCH="armv6"; \
    elif [ "$TARGETPLATFORM" = "linux/amd64/v3" \
        -o "$TARGETPLATFORM" = "linux/amd64/v2" \
        -o "$TARGETPLATFORM" = "linux/amd64" ]; then ARCH="x86_64"; \
    # we dont suppot any other arch so let it fail
    else ARCH="unknown"; fi && \
    # download requested kiwix-tools version
    url="http://mirror.download.kiwix.org/release/kiwix-tools/kiwix-tools_linux-$ARCH-$VERSION.tar.gz" && \
    echo "URL: $url" && \
    curl -k -L $url | tar -xz -C /usr/local/bin/ --strip-components 1 && \
    # only needed in dockerfile
    apk del curl

# expose kiwix-serve default port
EXPOSE 80

ENTRYPOINT ["/usr/bin/dumb-init", "--"]
CMD ["/bin/sh", "-c", "echo 'Welcome to kiwix-tools! The following binaries are available:' && ls /usr/local/bin/"]
