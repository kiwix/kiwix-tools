Kiwix-tools Docker image
===

- Available on [docker.io](https://hub.docker.com/r/kiwix/kiwix-tools) and [ghcr.io](https://ghcr.io/kiwix/kiwix-tools).
- multi-arch (`linux/amd64`, `linux/arm64`, `linux/arm/v7`)
- based on official `kiwix-tools` binaries.

## Usage

``` sh
$ docker run -it kiwix/kiwix-tools:3.1.2

Welcome to kiwix-tools! The following binaries are available:
kiwix-manage  kiwix-search  kiwix-serve
```

`kiwix-tools` operates on zim files. You shall mount a volume to access the files.

```sh
docker run -v $(pwd):/data -it kiwix/kiwix-tools kiwix-search /data/wikipedia_fr_test.zim "Mali"
```

## Building and reusing

- `kiwix/kiwix-tools` is multi-arch and is ideally built using `buildx`.
- requires a `--build-arg VERSION=` with the kiwix-tools release.
- can be built using `docker build` in which case it expects an additionnal `--build-arg ARCH=arm` for arm. Otherwise defaults to `amd64`.

**Notes:**

- `wget` in `alpine:3` on `arm/v7` (__inside github action only__) crashes when downloading from HTTPs locations. Keep http-only in Dockerfile.
- Was also unhappy when using the mirrors so it's using `mirror.download` on purpose.
