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

%prep
rm -rf $RPM_BUILD_DIR/simapi
rm -rf $RPM_SOURCE_DIR/simapi
cd $RPM_SOURCE_DIR
git clone https://github.com/spacefreak18/simapi
cd simapi
cd ..
cp -r $RPM_SOURCE_DIR/simapi $RPM_BUILD_DIR/

%build
cd $RPM_BUILD_DIR/simapi
cmake -B build -DBUILD_SIMD=on
cd build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/simapi/build/simd/simd $RPM_BUILD_ROOT/usr/bin/simd

%files
/usr/bin/simd
