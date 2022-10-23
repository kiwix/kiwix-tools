Kiwix serve Docker image
========================

With local ZIM file(s)
----------------------

* Download a ZIM file from <https://wiki.kiwix.org/wiki/Content>

* Given `wikipedia.zim` and `wiktionary.zim` reside in `/tmp/zim/`, execute the following:

```bash
docker run -v /tmp/zim:/data -p 8080:8080 kiwix/kiwix-serve wikipedia.zim wiktionary.zim
```

With remote ZIM file
--------------------

```bash
docker run -e "DOWNLOAD=https://download.kiwix.org/zim/wikipedia_bm_all.zim" -p 8080:8080 kiwix/kiwix-serve
```

Change default port
-------------------

You can change port to expose with environment PORT, useful if running on Podman, K8s or OpenShift

```bash
podman run -e "DOWNLOAD=https://download.kiwix.org/zim/wikipedia_bm_all.zim" -e PORT=8888 -p 8080:8888 kiwix/kiwix-serve
```

ARM
---

Build an image for an ARM based GNU/Linux:
```bash
docker build . -t kiwix/kiwix-serve:latest --build-arg ARCH="arm32v7/"
```

You can also deploy kiwix with [`docker-compose`](https://docs.docker.com/compose/). Check out a sample at [docker-compose.yml.example](docker-compose.yml.example)

Screenshots
-----------

![screenshot_1.png](https://github.com/kiwix/kiwix-tools/raw/master/docker/server/pictures/screenshot_1.png)
![screenshot_2.png](https://github.com/kiwix/kiwix-tools/raw/master/docker/server/pictures/screenshot_2.png)
