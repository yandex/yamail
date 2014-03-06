%define _builddir	.
%define _sourcedir	.
%define _specdir	.
%define _rpmdir		.

Name:		libyamail
Version:	%{yandex_mail_version}
Release:	%{yandex_mail_release}

Summary:	yamail library
License:	none
Group:		System Environment/Libraries
Packager:   none@none.ru  
Distribution:   Red Hat Enterprise Linux

Requires:	boost >= 1.43.0

BuildRequires:	boost-devel >= 1.43.0

BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root


%description
General utilities


%package	devel
Summary:	yamail development environment
Group:		System Environment/Libraries
Requires:	%{name} = %{version}-%{release}


%description	devel
General utilities

%build
cmake . -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_INSTALL_SYSCONFDIR=/etc \
        -DCMAKE_BUILD_TYPE=Release
%{__make} %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
%{__make} install DESTDIR=%{buildroot}

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-,root,root)
%{_libdir}/libyamail.so*

%files devel
%defattr(-,root,root)
%{_libdir}/libyamail.so*
%dir %{_includedir}/yamail
%{_includedir}/yamail/*.h
%{_includedir}/compat/*.h
%{_includedir}/data/*.h
%{_includedir}/data/config/*.h
%{_includedir}/data/config/detail/*.h
%{_includedir}/data/zerocopy/*.h
%{_includedir}/iterator/*.h
%{_includedir}/log/*.h
%{_includedir}/memory/*.h
%{_includedir}/traits/*.h
%{_includedir}/utility/*.h

%changelog

