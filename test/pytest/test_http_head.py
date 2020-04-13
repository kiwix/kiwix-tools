import pytest

import os
from pathlib import Path
import subprocess
from urllib.request import urlopen, Request as HTTPRequest
import time

DATADIR = Path(__file__).resolve().parent.parent/'data'
ZIMFILE = 'wikipedia_en_ray_charles_mini_2020-03.zim'

class KiwixServer:
    def __init__(self, host, port, zimfile):
        port = str(port)
        self.url = "http://{}:{}/".format(host, port)
        self.server = subprocess.Popen(['kiwix-serve', '-p', port, zimfile])
        time.sleep(1)

    def head(self, path):
        return urlopen(HTTPRequest(self.url + path, method='HEAD'))

    def stop(self):
        self.server.terminate()

@pytest.fixture
def kiwix_server():
    ks = KiwixServer('localhost', 8888, DATADIR/ZIMFILE)
    yield ks
    ks.stop()


def test_homepage(kiwix_server):
    h = kiwix_server.head('')
    assert 'ETag' in h.info().keys()
