# declare build option ARCH
ARG ARCH=
# download ARCH-specific base image if specified
FROM alpine:latest
# decide which kiwix arch to download later (`armhf` for all arm* and x86_64 otherwise)
ARG ARCH
RUN if [ $(echo $ARCH | cut -c 1-3) = "arm" ] ; then echo "armhf" > /etc/kiwix_arch ; else echo "x86_64" > /etc/kiwix_arch ; fi
LABEL maintainer Emmanuel Engelhart <kelson@kiwix.org>

# Install kiwix-serve
ARG RELEASE_ARCH="x86_64"
WORKDIR /
RUN apk add --no-cache curl bzip2
RUN curl -kL https://download.kiwix.org/release/kiwix-tools/kiwix-tools_linux-$(cat /etc/kiwix_arch).tar.gz | tar -xz && \
    mv kiwix-tools*/kiwix-serve /usr/local/bin && \
    rm -r kiwix-tools*

# Run kiwix-serve
EXPOSE 80
VOLUME /data
WORKDIR /data

COPY ./start.sh /usr/local/bin/

ENTRYPOINT ["/usr/local/bin/start.sh"]
