Name:       sticker-panel
Summary:    Sticker Panel
Version:    0.1.0
Release:    0
Group:      Application
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires: cmake
BuildRequires: edje-tools
BuildRequires: gettext-tools

BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-base-common)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(feedback)
BuildRequires: pkgconfig(isf)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(bundle)

%description
Description: Sticker Panel

%prep
%setup -q

%package devel
Summary:    Sticker panel library (devel)
Group:      Application
Requires:   %{name} = %{version}-%{release}

%description devel
Development files needed to build software that needs Sticker panel.

%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif

%cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DSHAREDIR=%{_datadir}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post
sqlite3 /opt/dbspace/.sticker_panel.db "CREATE TABLE IF NOT EXISTS db_checksum (version INT);"
sqlite3 /opt/dbspace/.sticker_panel.db "CREATE TABLE IF NOT EXISTS recent_icon (
			id TEXT
			, type INTEGER
			, time INTEGER
			, PRIMARY KEY(id, type)
			);"
sqlite3 /opt/dbspace/.sticker_panel.db "CREATE TABLE IF NOT EXISTS group_icon (
			id TEXT PRIMARY KEY
			, name TEXT
			, repository TEXT
			, ordering INTEGER
			, category INTEGER
			);"
chsmack -a "sticker-panel::db" /opt/dbspace/.sticker_panel.db*
chmod 666 /opt/dbspace/.sticker_panel.db*

%files
%manifest %{name}.manifest
%{_prefix}/lib/*.so*
%{_datadir}/sticker-panel/edje/*.edj
%{_datadir}/sticker-panel/images/*
%{_datadir}/sticker-panel/sample/*
%{_datadir}/icons/*.png

%files devel
%defattr(-,root,root,-)
%{_includedir}/sticker-panel/sticker_panel.h
%{_libdir}/pkgconfig/%{name}.pc
