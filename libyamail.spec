%define _builddir	.
%define _sourcedir	.
%define _specdir	.
%define _rpmdir		.

Name:		libyamail
Version:	1.0
Release:	2

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
%{_includedir}/yamail/compat/*.h
%{_includedir}/yamail/concurrency/*.h
%{_includedir}/yamail/concurrency/future/*.h
%{_includedir}/yamail/concurrency/coroutine/*.h
%{_includedir}/yamail/concurrency/coroutine/*.inl
%{_includedir}/yamail/concurrency/coroutine/detail/*.h
%{_includedir}/yamail/concurrency/future/*.html
%{_includedir}/yamail/concurrency/future/*.txt
%{_includedir}/yamail/data/*.h
%{_includedir}/yamail/data/config/*.h
%{_includedir}/yamail/data/config/detail/*.h
%{_includedir}/yamail/data/zerocopy/*.h
%{_includedir}/yamail/error/*.h
%{_includedir}/yamail/iterator/*.h
%{_includedir}/yamail/log/*.h
%{_includedir}/yamail/memory/*.h
%{_includedir}/yamail/traits/*.h
%{_includedir}/yamail/utility/*.h

%changelog

