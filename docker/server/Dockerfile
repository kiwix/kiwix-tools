ARG VERSION=latest

# kiwix-tools is multi-arch
FROM ghcr.io/kiwix/kiwix-tools:$VERSION
LABEL org.opencontainers.image.source=https://github.com/openzim/kiwix-tools

# expose kiwix-serve default port and workdir
EXPOSE 8080
VOLUME /data
WORKDIR /data

# running as a named unprivileged user
RUN addgroup -S user && adduser -S user -G user
USER user

COPY ./start.sh /usr/local/bin/

ENTRYPOINT ["/usr/bin/dumb-init", "--", "/usr/local/bin/start.sh"]
