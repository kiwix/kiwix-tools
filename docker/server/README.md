kiwix-serve Docker image
========================

* Download a ZIM file from <https://wiki.kiwix.org/wiki/Content>

* Given `wikipedia.zim` resides in `/tmp/zim/`, execute the following:

```
docker run -v /tmp/zim:/data -p 8080:80 kiwix/kiwix-serve wikipedia.zim
```

![screenshot_1.png](https://github.com/kiwix/kiwix-tools/raw/master/docker/server/pictures/screenshot_1.png)
![screenshot_2.png](https://github.com/kiwix/kiwix-tools/raw/master/docker/server/pictures/screenshot_2.png)
