#dnf install argtable-devel libconfig-devel libuv-devel
%global __brp_check_rpaths %{nil}
Summary: Racing Simulator Libraries
Name: simd
Version: 0.0.5
Release: 1
License: GPL
Group: Applications/Sound
Source: https://github.com/simapi
URL: https://spacefreak18.github.io/simapi
Distribution: Fedora Linux
Vendor: spacefreak18
Packager: Paul Jones <paul@spacefreak18.xyz>
Requires: argtable libconfig libuv libsimapi libyder

%description
Racing Simulator Telemetry Libraries

# Builds whatever tree has been staged at $RPM_SOURCE_DIR/simapi, cloning
# upstream only if nothing is staged. The unconditional clone this replaced
# meant an rpm's contents tracked the default branch rather than the tag being
# built, so a fix on the branch being released was absent from its own release.
# CI stages the checked-out tree; a bare `rpmbuild -ba` still works as before.
%prep
rm -rf $RPM_BUILD_DIR/simapi
if [ ! -d $RPM_SOURCE_DIR/simapi ]; then
    cd $RPM_SOURCE_DIR
    git clone https://github.com/spacefreak18/simapi
fi
cp -r $RPM_SOURCE_DIR/simapi $RPM_BUILD_DIR/

%build
cd $RPM_BUILD_DIR/simapi
# The two *_DIR variables default under $ENV{HOME}; a package must never
# write into a user's home. Redirected rather than patched upstream, since
# both are already CACHE PATH variables.
# CMAKE_SKIP_RPATH: simd links libsimapi out of the build tree, so CMake bakes
# that path in as a RUNPATH and rpm's check-rpaths rejects the resulting
# binary outright ("contains an invalid runpath ... /rpmbuild/BUILD/..."). The
# spec's %%global __brp_check_rpaths %%{nil} no longer suppresses that check on
# Fedora 43/44. Not emitting the RUNPATH is the deterministic fix, and nothing
# needs it: libsimapi installs to a standard system library directory.
cmake -B build -DBUILD_SIMD=on -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_SKIP_RPATH=ON \
  -DSYSTEMD_USER_UNIT_DIR=/usr/lib/systemd/user \
  -DSIMD_CONFIG_DIR=/usr/share/simd
cd build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/simapi/build/simd/simd $RPM_BUILD_ROOT/usr/bin/simd
# Upstream's simd/conf/simd.service sets Type=simple against a daemon that
# double-forks -- systemd takes the parent's exit as the service failing while
# the real daemon carries on unsupervised -- and hardcodes
# ExecStart=%%h/.local/bin/simd. The packaged unit runs /usr/bin/simd -n.
install -Dm644 $RPM_BUILD_DIR/simapi/tools/distro/simd.service \
    $RPM_BUILD_ROOT/usr/lib/systemd/user/simd.service
# An example, not a live config: packages must not write into $HOME, which is
# where simd looks for it (built from getpwuid(), not $HOME or XDG_CONFIG_HOME).
install -Dm644 $RPM_BUILD_DIR/simapi/simd/conf/simd.config \
    $RPM_BUILD_ROOT/usr/share/simd/simd.config

%files
/usr/bin/simd
/usr/lib/systemd/user/simd.service
/usr/share/simd/simd.config
