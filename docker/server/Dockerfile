FROM alpine:latest
LABEL maintainer Emmanuel Engelhart <kelson@kiwix.org>

# Install kiwix-serve
WORKDIR /
RUN apk add --no-cache curl bzip2
RUN curl -kL https://download.kiwix.org/release/kiwix-tools/kiwix-tools_linux-x86_64-1.1.0.tar.gz | tar -xz && \
    mv kiwix-tools*/kiwix-serve /usr/local/bin && \
    rm -r kiwix-tools*

# Configure kiwix-serve
VOLUME /data
ENV PORT 80
EXPOSE $PORT

# Run kiwix-serve
WORKDIR /data
ENTRYPOINT ["/usr/local/bin/kiwix-serve", "--port", "$PORT"]
