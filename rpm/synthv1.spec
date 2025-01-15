#
# spec file for package synthv1
#
# Copyright (C) 2012-2025, rncbc aka Rui Nuno Capela. All rights reserved.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Summary:	An old-school polyphonic subtractive synthesizer
Name:		synthv1
Version:	1.3.0
Release:	7.1
License:	GPL-2.0-or-later
Group:		Productivity/Multimedia/Sound/Midi
Source: 	%{name}-%{version}.tar.gz
URL:		http://synthv1.sourceforge.net
#Packager:	rncbc.org

%if 0%{?fedora_version} >= 34 || 0%{?suse_version} > 1500 || ( 0%{?sle_version} >= 150200 && 0%{?is_opensuse} )
%define qt_major_version  6
%else
%define qt_major_version  5
%endif

%if %{defined fedora}
%global debug_package %{nil}
%endif

BuildRequires:	coreutils
BuildRequires:	pkgconfig
BuildRequires:	glibc-devel
BuildRequires:	cmake >= 3.15
%if 0%{?sle_version} >= 150200 && 0%{?is_opensuse}
BuildRequires:	gcc10 >= 10
BuildRequires:	gcc10-c++ >= 10
%define _GCC	/usr/bin/gcc-10
%define _GXX	/usr/bin/g++-10
%else
BuildRequires:	gcc >= 10
BuildRequires:	gcc-c++ >= 10
%define _GCC	/usr/bin/gcc
%define _GXX	/usr/bin/g++
%endif
%if 0%{qt_major_version} == 6
BuildRequires:	qtbase6.8-static >= 6.8
BuildRequires:	qttools6.8-static
BuildRequires:	qttranslations6.8-static
BuildRequires:	qtsvg6.8-static
%else
BuildRequires:	pkgconfig(Qt5Core)
BuildRequires:	pkgconfig(Qt5Gui)
BuildRequires:	pkgconfig(Qt5Widgets)
BuildRequires:	pkgconfig(Qt5Svg)
BuildRequires:	pkgconfig(Qt5Xml)
%endif
%if %{defined fedora}
BuildRequires:	jack-audio-connection-kit-devel
%else
BuildRequires:	pkgconfig(jack)
%endif
BuildRequires:	pkgconfig(alsa)

BuildRequires:	pkgconfig(liblo)
BuildRequires:	pkgconfig(lv2)

BuildRequires:	pkgconfig(egl)

%description
  An old-school all-digital 4-oscillator subtractive polyphonic synthesizer
  with stereo fx.


%package -n %{name}-jack
Summary:	An old-school polyphonic subtractive synthesizer - JACK standalone
Provides:	%{name}_jack
Obsoletes:	%{name}-common <= %{version}, %{name} <= %{version}

%description -n %{name}-jack
  An old-school all-digital 4-oscillator subtractive polyphonic synthesizer
  with stereo fx.

  This package provides the standalone JACK client application (synthv1_jack)


%package -n %{name}-lv2
Summary:	An old-school polyphonic subtractive synthesizer - LV2 plugin
Provides:	%{name}_lv2, %{name}_lv2ui
Obsoletes:	%{name}-common <= %{version}

%description -n %{name}-lv2
  An old-school all-digital 4-oscillator subtractive polyphonic synthesizer
  with stereo fx.

  This package provides the LV2 plugin (http://synthv1.sourceforge.net/lv2)


%prep
%setup -q

%build
%if 0%{qt_major_version} == 6
source /opt/qt6.8-static/bin/qt6.8-static-env.sh
%endif
CXX=%{_GXX} CC=%{_GCC} \
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -Wno-dev -B build
cmake --build build %{?_smp_mflags}

%install
DESTDIR="%{buildroot}" \
cmake --install build


%files -n %{name}-jack
%license LICENSE
%doc README ChangeLog
#dir %{_datadir}/applications
%dir %{_datadir}/metainfo
#dir %{_datadir}/mime
#dir %{_datadir}/mime/packages
%dir %{_datadir}/icons/hicolor
%dir %{_datadir}/icons/hicolor/32x32
%dir %{_datadir}/icons/hicolor/32x32/apps
%dir %{_datadir}/icons/hicolor/32x32/mimetypes
%dir %{_datadir}/icons/hicolor/scalable
%dir %{_datadir}/icons/hicolor/scalable/apps
%dir %{_datadir}/icons/hicolor/scalable/mimetypes
#dir %{_datadir}/man
#dir %{_datadir}/man/man1
#dir %{_datadir}/man/fr
#dir %{_datadir}/man/fr/man1
%dir %{_datadir}/%{name}
%dir %{_datadir}/%{name}/palette
%{_bindir}/%{name}_jack
%{_datadir}/metainfo/org.rncbc.%{name}.metainfo.xml
%{_datadir}/applications/org.rncbc.%{name}.desktop
%{_datadir}/mime/packages/org.rncbc.%{name}.xml
%{_datadir}/icons/hicolor/32x32/apps/org.rncbc.%{name}.png
%{_datadir}/icons/hicolor/scalable/apps/org.rncbc.%{name}.svg
%{_datadir}/icons/hicolor/32x32/mimetypes/org.rncbc.%{name}.application-x-%{name}*.png
%{_datadir}/icons/hicolor/scalable/mimetypes/org.rncbc.%{name}.application-x-%{name}*.svg
%{_datadir}/man/man1/%{name}.1.gz
%{_datadir}/man/fr/man1/%{name}.1.gz
%{_datadir}/%{name}/palette/*.conf

%files -n %{name}-lv2
%dir %{_libdir}/lv2
%dir %{_libdir}/lv2/%{name}.lv2
%{_libdir}/lv2/%{name}.lv2/manifest.ttl
%{_libdir}/lv2/%{name}.lv2/%{name}.ttl
%{_libdir}/lv2/%{name}.lv2/%{name}.so
%{_libdir}/lv2/%{name}.lv2/%{name}_ui.ttl


%changelog
* Thu Jan 16 2025 Rui Nuno Capela <rncbc@rncbc.org> 1.3.0
- A New-Year'25 Release.
* Sun Dec 15 2024 Rui Nuno Capela <rncbc@rncbc.org> 1.2.0
- An End-of-Year'24 Release.
* Thu Oct 31 2024 Rui Nuno Capela <rncbc@rncbc.org> 1.1.3
- A Halloween'24 Release.
* Wed Oct  2 2024 Rui Nuno Capela <rncbc@rncbc.org> 1.1.2
- An Early-Fall'24 Release.
* Fri Sep 20 2024 Rui Nuno Capela <rncbc@rncbc.org> 1.1.1
- An End-of-Summer'24 Release.
* Wed Aug 28 2024 Rui Nuno Capela <rncbc@rncbc.org> 1.1.0
- A Mid-Summer'24 Release.
* Thu Jun 20 2024 Rui Nuno Capela <rncbc@rncbc.org> 1.0.0
- An Unthinkable Release.
