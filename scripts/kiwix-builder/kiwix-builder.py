#!/usr/bin/env python3

import os
import argparse
import urllib.request
import tarfile
from subprocess import check_call, STDOUT, CalledProcessError
import hashlib
import shutil
from collections import defaultdict

pj = os.path.join

REMOTE_PREFIX = 'http://download.kiwix.org/dev/'

SOURCE_DIR = pj(os.getcwd(), "SOURCE")
ARCHIVE_DIR = pj(os.getcwd(), "ARCHIVE")


################################################################################
##### UTILS
################################################################################

class Defaultdict(defaultdict):
    def __getattr__(self, name):
        return self[name]

def get_sha256(path):
    sha256 = hashlib.sha256()
    with open(path, 'br') as f:
        sha256.update(f.read())
    return sha256.hexdigest()

class SkipCommand(Exception):
    pass

class StopBuild(Exception):
    pass

def command(name, withlog='source_path', autoskip=False):
    def decorator(function):
        def wrapper(self, *args):
            print("  {} {} : ".format(name, self.name), end="", flush=True)
            if autoskip and os.path.exists(pj(self.source_path, ".{}_ok".format(name))):
                print("SKIP")
                return
            log = pj(getattr(self, withlog), 'cmd_{}_{}.log'.format(name, self.name))
            try:
                with open(log, 'w') as _log:
                    ret = function(self, *args, log=_log)
                print("OK")
                if autoskip:
                    with open(pj(self.source_path, ".{}_ok".format(name)), 'w') as f: pass
                return ret
            except SkipCommand:
                print("SKIP")
            except CalledProcessError:
                print("ERROR")
                with open(log, 'r') as f:
                    print(f.read())
                raise StopBuild()
            except:
                print("ERROR")
                raise
        return wrapper
    return decorator

def run_command(command, cwd, log, env=None, input=None):
    log.write("run command '{}'\n".format(command))
    if env:
        log.write("env is :\n")
        for k, v in env.items():
            log.write("  {} : {!r}\n".format(k, v))
    log.flush()

    kwargs = dict()
    if env:
        kwargs['env'] = env
    if input:
        kwargs['stdin'] = input
    return check_call(command, shell=True, cwd=cwd, stdout=log, stderr=STDOUT, **kwargs)



################################################################################
##### PROJECT
################################################################################


class Dependency:
    @property
    def source_path(self):
        return pj(SOURCE_DIR, self.source_dir)


class ReleaseDownloadMixin:
    _log_download = ARCHIVE_DIR
    _log_source = SOURCE_DIR

    archive_top_dir = ""

    @property
    def archive_path(self):
        return pj(ARCHIVE_DIR, self.archive_name)

    @property
    def extract_path(self):
        return SOURCE_DIR

    @property
    def archive_top_path(self):
        return pj(SOURCE_DIR, self.archive_top_dir or self.source_dir)

    @command("download", withlog='_log_download')
    def _download(self, log):
        if os.path.exists(self.archive_path):
            sha256 = get_sha256(self.archive_path)
            if sha256 == self.archive_sha256:
                raise SkipCommand()
            os.remove(self.archive_path)
        remote = REMOTE_PREFIX + self.archive_name
        urllib.request.urlretrieve(remote, self.archive_path)

    @command("extract", withlog='_log_source', autoskip=True)
    def _extract(self, log):
        if os.path.exists(self.source_path):
            shutil.rmtree(self.source_path)
        with tarfile.open(self.archive_path) as archive:
            archive.extractall(self.extract_path)

    @command("patch", autoskip=True)
    def _patch(self, log):
        if not hasattr(self, 'patch'):
            raise SkipCommand()
        with open(pj('patches', self.patch), 'r') as patch_input:
            run_command("patch -p1", self.source_path, log, input=patch_input)

    def prepare(self, options):
        self._download()
        self._extract()
        self._patch()


class GitCloneMixin:
    _log_clone = SOURCE_DIR
    git_ref = "master"

    @property
    def git_path(self):
        return pj(SOURCE_DIR, self.git_dir)

    @command("gitclone", withlog='_log_clone')
    def _git_clone(self, log):
        if os.path.exists(self.git_path):
            raise SkipCommand()
        command = "git clone " + self.git_remote
        run_command(command, SOURCE_DIR, log)

    @command("gitupdate")
    def _git_update(self, log):
        run_command("git pull", self.git_path, log)
        run_command("git checkout "+self.git_ref, self.git_path, log)

    def prepare(self, options):
        self._git_clone()
        self._git_update()


class MakeMixin:
    configure_option = ""
    make_option = ""
    install_option = ""
    configure_script = "./configure"
    configure_env = None

    @command("configure", autoskip=True)
    def _configure(self, options, log):
        command = self.configure_script + " " + self.configure_option + " --prefix " + options.install_dir
        env = Defaultdict(str, os.environ)
        if options.build_static:
            env['CFLAGS'] = env['CFLAGS'] + ' -fPIC'
        if self.configure_env:
           for k in self.configure_env:
               if k.startswith('_format_'):
                   v = self.configure_env.pop(k)
                   v = v.format(options=options, env=env)
                   self.configure_env[k[8:]] = v
           env.update(self.configure_env)
        run_command(command, self.source_path, log, env=env)

    @command("compile", autoskip=True)
    def _compile(self, log):
        command = "make -j4 " + self.make_option
        run_command(command, self.source_path, log)

    @command("install", autoskip=True)
    def  _install(self, log):
        command = "make install " + self.make_option
        run_command(command, self.source_path, log)

    def build(self, options):
        self._configure(options)
        self._compile()
        self._install()


class CMakeMixin(MakeMixin):
    @command("configure", autoskip=True)
    def _configure(self, options, log):
        libdir = "-DCMAKE_INSTALL_LIBDIR={}".format("lib64" if options.target_arch == "x86_64" else "lib")
        command = "cmake {configure_option} -DCMAKE_INSTALL_PREFIX={install_dir} {extra_options}".format(
            configure_option = self.configure_option,
            install_dir = options.install_dir,
            extra_options = libdir)
        run_command(command, self.source_path, log)

class MesonMixin(MakeMixin):
    @property
    def build_path(self):
        return pj(self.source_path, 'build')

    @command("configure", autoskip=True)
    def _configure(self, options, log):
        if os.path.exists(self.build_path):
            shutil.rmtree(self.build_path)
        os.makedirs(self.build_path)
        env = Defaultdict(str, os.environ)
        env['PKG_CONFIG_PATH'] = ':'.join([pj(options.install_dir, 'lib64', 'pkgconfig'),
                                          pj(options.install_dir, 'lib', 'pkgconfig')])
        if options.build_static:
            env['LDFLAGS'] = env['LDFLAGS'] + " -static-libstdc++ --static"
            library_type = 'static'
        else:
            library_type = 'shared'
        configure_option = self.configure_option.format(options=options)
        command = "meson.py --default-library={library_type} {configure_option} . build --prefix={options.install_dir}".format(
            library_type=library_type,
            configure_option=configure_option,
            options=options)
        run_command(command, self.source_path, log, env=env)

    @command("compile")
    def _compile(self, log):
        run_command("ninja-build -v", self.build_path, log)

    @command("install")
    def _install(self, log):
        run_command("ninja-build -v install", self.build_path, log)


# *************************************
# Missing dependencies
# Is this ok to assume that those libs
# exist in your "distri" (linux/mac) ?
# If not, we need to compile them here
# *************************************
# Zlib
# LZMA
# aria2
# Argtable
# MSVirtual
# Android
# libiconv
# gettext
# *************************************

class UUID(Dependency, ReleaseDownloadMixin, MakeMixin):
    name = 'uuid'
    archive_name = 'e2fsprogs-1.42.tar.gz'
    archive_sha256 = '55b46db0cec3e2eb0e5de14494a88b01ff6c0500edf8ca8927cad6da7b5e4a46'
    source_dir = 'e2fsprogs-1.42'
    configure_option = "--enable-libuuid"
    configure_env = {'_format_CFLAGS' : "{env.CFLAGS} -fPIC"}

    @command("compile", autoskip=True)
    def _compile(self, log):
        command = "make -j4 libs " + self.make_option
        run_command(command, self.source_path, log)

    @command("install", autoskip=True)
    def  _install(self, log):
        command = "make install-libs " + self.make_option
        run_command(command, self.source_path, log)


class Xapian(Dependency, ReleaseDownloadMixin, MakeMixin):
    name = "xapian"
    archive_name = 'xapian-core-1.4.0.tar.xz'
    source_dir = 'xapian-core-1.4.0'
    archive_sha256 = '10584f57112aa5e9c0e8a89e251aecbf7c582097638bfee79c1fe39a8b6a6477'
    configure_option = "--enable-shared --enable-static --disable-sse --disable-backend-inmemory"
    patch = "xapian_pkgconfig.patch"
    configure_env = {'_format_LDFLAGS' : "-L{options.install_dir}/lib",
                     '_format_CXXFLAGS' : "-I{options.install_dir}/include"}


class CTPP2(Dependency, ReleaseDownloadMixin, CMakeMixin):
    name = "ctpp2"
    archive_name = 'ctpp2-2.8.3.tar.gz'
    source_dir = 'ctpp2-2.8.3'
    archive_sha256 = 'a83ffd07817adb575295ef40fbf759892512e5a63059c520f9062d9ab8fb42fc'
    configure_option = "-DMD5_SUPPORT=OFF"
    patch = "ctpp2_include.patch"


class Pugixml(Dependency, ReleaseDownloadMixin, MesonMixin):
    name = "pugixml"
    archive_name = 'pugixml-1.2.tar.gz'
    extract_path = pj(SOURCE_DIR, 'pugixml-1.2')
    source_dir = 'pugixml-1.2'
    archive_sha256 = '0f422dad86da0a2e56a37fb2a88376aae6e931f22cc8b956978460c9db06136b'
    patch = "pugixml_meson.patch"


class MicroHttpd(Dependency, ReleaseDownloadMixin, MakeMixin):
    name = "microhttpd"
    archive_name = 'libmicrohttpd-0.9.19.tar.gz'
    source_dir = 'libmicrohttpd-0.9.19'
    archive_sha256 = 'dc418c7a595196f09d2f573212a0d794404fa4ac5311fc9588c1e7ad7a90fae6'
    configure_option = "--enable-shared --enable-static --disable-https --without-libgcrypt --without-libcurl"


class Icu(Dependency, ReleaseDownloadMixin, MakeMixin):
    name = "icu"
    archive_name = 'icu4c-56_1-src.tgz'
    archive_sha256 = '3a64e9105c734dcf631c0b3ed60404531bce6c0f5a64bfe1a6402a4cc2314816'
    archive_top_dir = 'icu'
    data_name = 'icudt56l.dat'
    data_sha256 = 'e23d85eee008f335fc49e8ef37b1bc2b222db105476111e3d16f0007d371cbca'
    source_dir = 'icu/source'
    configure_option = "Linux --disable-samples --disable-tests --disable-extras --enable-static --disable-dyload"
    configure_script = "./runConfigureICU"

    @property
    def data_path(self):
        return pj(ARCHIVE_DIR, self.data_name)

    @command("download_data", autoskip=True)
    def _download_data(self, log):
        if os.path.exists(self.data_path):
            sha256 = get_sha256(self.data_path)
            if sha256 == self.data_sha256:
                raise SkipCommand()
            os.remove(self.data_path)
        remote = REMOTE_PREFIX + self.data_name
        urllib.request.urlretrieve(remote, self.data_path)

    @command("copy_data", autoskip=True)
    def _copy_data(self, log):
        shutil.copyfile(self.data_path, pj(self.source_path, 'data', 'in', self.data_name))

    def prepare(self, options):
        super().prepare(options)
        self._download_data()
        self._copy_data()


class Zimlib(Dependency, GitCloneMixin, MesonMixin):
    name = "zimlib"
    git_remote = "https://gerrit.wikimedia.org/r/p/openzim.git"
    git_dir = "openzim"
    git_ref = "meson"
    source_dir = "openzim/zimlib"


class Kiwixlib(Dependency, GitCloneMixin, MesonMixin):
    name = "kiwix-lib"
    git_remote = "https://github.com/kiwix/kiwix-lib.git"
    git_dir = "kiwix-lib"
    git_ref = "meson"
    source_dir = "kiwix-lib"
    configure_option = "-Dctpp2-install-prefix={options.install_dir}"


class KiwixTools(Dependency, GitCloneMixin, MesonMixin):
    name = "kiwix-tools"
    git_remote = "https://github.com/kiwix/kiwix-tools.git"
    git_dir = "kiwix-tools"
    git_ref = "meson"
    source_dir = "kiwix-tools"
    configure_option = "-Dctpp2-install-prefix={options.install_dir}"


class Project:
    def __init__(self, options):
        self.options = options
        self.dependencies = [UUID(), Xapian(), CTPP2(), Pugixml(), Zimlib(), MicroHttpd(), Icu(), Kiwixlib(), KiwixTools()]
        os.makedirs(SOURCE_DIR, exist_ok=True)
        os.makedirs(ARCHIVE_DIR, exist_ok=True)

    def prepare(self):
        for dependency in self.dependencies:
            print("prepare {} :".format(dependency.name))
            dependency.prepare(self.options)

    def build(self):
        for dependency in self.dependencies:
            print("build {} :".format(dependency.name))
            dependency.build(self.options)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('install_dir')
    parser.add_argument('--target_arch', default="x86_64")
    parser.add_argument('--build_static', action="store_true")
    return parser.parse_args()

if __name__ == "__main__":
    options = parse_args()
    options.install_dir = os.path.abspath(options.install_dir)
    project = Project(options)
    try:
        print("[PREPARE]")
        project.prepare()
        print("[BUILD]")
        project.build()
    except StopBuild:
        print("Stopping build due to errors")
