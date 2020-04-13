import pytest

import os
from pathlib import Path
import subprocess
from urllib.request import urlopen, Request as HTTPRequest
import time

DATADIR = Path(__file__).resolve().parent.parent/'data'
ZIMFILE = 'wikipedia_en_ray_charles_mini_2020-03.zim'
KIWIX_SERVE_EXE=os.environ.get('KIWIX_SERVE_EXE_PATH', 'kiwix-serve')

class KiwixServer:
    def __init__(self, host, port, zimfile):
        port = str(port)
        self.url = "http://{}:{}/".format(host, port)
        self.server = subprocess.Popen([KIWIX_SERVE_EXE, '-p', port, zimfile])
        time.sleep(1)

    def head(self, path):
        return urlopen(HTTPRequest(self.url + path, method='HEAD'))

    def stop(self):
        self.server.terminate()

@pytest.fixture
def kiwix_servers():
    ks1 = KiwixServer('localhost', 8888, DATADIR/ZIMFILE)
    ks2 = KiwixServer('localhost', 8889, DATADIR/ZIMFILE)
    yield ks1, ks2
    ks1.stop()
    ks2.stop()


def test_homepage(kiwix_servers):
    h11 = kiwix_servers[0].head('')
    h12 = kiwix_servers[0].head('')
    h21 = kiwix_servers[1].head('')
    h22 = kiwix_servers[1].head('')
    assert 'ETag' in h11.info().keys()
    assert 'ETag' in h12.info().keys()
    assert 'ETag' in h21.info().keys()
    assert 'ETag' in h22.info().keys()
    assert h11.getheader('ETag') == h12.getheader('ETag')
    assert h21.getheader('ETag') == h22.getheader('ETag')
    assert h11.getheader('ETag') != h21.getheader('ETag')
