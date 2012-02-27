#
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#


Name:           QTwikeAnalyzer
Version:	0.4
Release:	1
Summary:	Twike Application
Group:		Development/Tools/Other
License:	GPL
Url:		http://energex.origo.ethz.ch/wiki/energex
BuildRequires:  libqt4-devel qwt-devel libqextserialport1-devel
Source:		QTwikeAnalyzer-0.4.tar.gz
# Patch:
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
AutoReqProv:    on

%description	
Application to decode and analyze the Twike RS485 protocol.
Furthermore this application can be used to update the 
Mediator firmware.

Authors:
--------
    Walser Markus

%prep
%setup
qmake QTwikeAnalyzer.pro

%build
make %{?jobs:-j%jobs}

%install
%__mkdir_p %{buildroot}%{_bindir}
make INSTALL_ROOT=%{buildroot} install


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc ChangeLog README COPYING
%{_bindir}/QTwikeAnalyzer

%changelog
* Sun Mar 14 2010 markus.walser@gmail.com
- Initial C++ version.

