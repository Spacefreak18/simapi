Summary: Racing Simulator Libraries
Name: libsimapi
Version: 0.0.5
Release: 1
License: GPL
Group: Applications/Sound
Source: https://github.com/simapi
URL: https://spacefreak18.github.io/simapi
Distribution: Fedora Linux
Vendor: spacefreak18
Packager: Paul Jones <paul@spacefreak18.xyz>

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
cmake -B build -DBUILD_SIMD=off
cd build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/lib64
cp $RPM_BUILD_DIR/simapi/build/libsimapi* $RPM_BUILD_ROOT/usr/lib64/

%files
/usr/lib64/libsimapi.so
/usr/lib64/libsimapi.so.1
/usr/lib64/libsimapi.so.1.0.1
