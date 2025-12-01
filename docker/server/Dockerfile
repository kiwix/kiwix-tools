ARG VERSION=latest

# kiwix-tools is multi-arch
FROM ghcr.io/kiwix/kiwix-tools:$VERSION
LABEL org.opencontainers.image.source=https://github.com/openzim/kiwix-tools

# expose kiwix-serve default port and workdir
EXPOSE 8080
VOLUME /data
WORKDIR /data

# running as a named unprivileged user
RUN addgroup -S -g 1001 user && adduser -S -u 1001 user -G user
RUN chown user:user /data
USER user

COPY ./start.sh /usr/local/bin/

ENTRYPOINT ["/usr/bin/dumb-init", "--", "/usr/local/bin/start.sh"]
