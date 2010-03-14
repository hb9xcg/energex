#
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#

# norootforbuild

Name:           QTwikeAnalyzer
Version:	0.4
Release:	1
Summary:	Twike Application
Group:		Development/Tools/Other
License:	GPL
Url:		http://energex.origo.ethz.ch/wiki/energex
PreReq:		libqextserialport1
PreReq:		libqwt5
# BuildRequires:
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
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%post
%postun

%files
%defattr(-,root,root)
%doc ChangeLog README COPYING

%changelog
* Sun Mar 14 2010 markus.walser@gmail.com
Initial C++ version.

