FROM alpine:3.16
LABEL org.opencontainers.image.source https://github.com/openzim/kiwix-tools
ARG VERSION

RUN apk --no-cache add dumb-init curl && \
    ARCH=$(cat /etc/apk/arch) && \
    if [ "$ARCH" = "x86" ]; then ARCH="i586"; \
    elif [ "$ARCH" = "aarch64" ]; then ARCH="armhf"; \
    elif [ "$ARCH" = "armv7" ]; then ARCH="armhf"; fi && \
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
