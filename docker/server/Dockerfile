FROM alpine:latest
LABEL maintainer Emmanuel Engelhart <kelson@kiwix.org>

# Install kiwix-serve
WORKDIR /
RUN apk add --no-cache curl bzip2
RUN curl -kL https://download.kiwix.org/release/kiwix-tools/kiwix-tools_linux-x86_64.tar.gz | tar -xz && \
    mv kiwix-tools*/kiwix-serve /usr/local/bin && \
    rm -r kiwix-tools*

# Run kiwix-serve
EXPOSE 80
VOLUME /data
WORKDIR /data

COPY ./start.sh /usr/local/bin/

ENTRYPOINT ["/usr/local/bin/start.sh"]
