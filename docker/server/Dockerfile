FROM alpine:latest
LABEL maintainer Emmanuel Engelhart <kelson@kiwix.org>

# Install kiwix-serve
WORKDIR /
RUN apk add --no-cache curl bzip2
RUN curl -kL https://download.kiwix.org/release/kiwix-tools/kiwix-tools_linux-x86_64-1.2.1.tar.gz | tar -xz && \
    mv kiwix-tools*/kiwix-serve /usr/local/bin && \
    rm -r kiwix-tools*

# Run kiwix-serve
EXPOSE 80
VOLUME /data
WORKDIR /data
ENTRYPOINT ["/usr/local/bin/kiwix-serve", "--port", "80"]
