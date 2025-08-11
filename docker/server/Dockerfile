ARG VERSION=latest

# kiwix-tools is multi-arch
FROM ghcr.io/kiwix/kiwix-tools:$VERSION
LABEL org.opencontainers.image.source=https://github.com/openzim/kiwix-tools

# expose kiwix-serve default port and workdir
EXPOSE 8080
VOLUME /data
WORKDIR /data

COPY ./start.sh /usr/local/bin/

# Create non-root user for better security
RUN addgroup -S kiwix && adduser -S kiwix -G kiwix
# Change ownership of the start script to the new user
RUN chown kiwix:kiwix /usr/local/bin/start.sh
# Switch to the non-root user
USER kiwix

ENTRYPOINT ["/usr/bin/dumb-init", "--", "/usr/local/bin/start.sh"]
