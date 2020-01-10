
%if 0%{?fedora}
%global kscd 1
%endif

Name:    kdemultimedia
Epoch:   6
Version: 4.10.5
Release: 2%{?dist}
Summary: KDE Multimedia metapackage 

# see also: http://techbase.kde.org/Policies/Licensing_Policy
License: GPLv2+
URL:     http://www.kde.org/
BuildArch: noarch

BuildRequires: kde-filesystem 

# empty main meta-package to ease upgrades...
Requires: dragon >= %{version}
Requires: juk >= %{version}
Requires: audiocd-kio >= %{version}
Requires: kmix >= %{version}
%if 0%{?kscd}
Requires: kscd >= %{version}
%else
Obsoletes: kscd < %{version}-%{release}
%endif

%description
Kdemultimedia metapackage, to ease migration to split applications.

%package common
Summary: Common files for %{name}
%description common
%{summary}.

## not sure to have a real -libs pkg or to simply Obsoletes it
## we've done it both ways, kdeedu Obsoletes, kdegraphics includes it -- rex
%package libs
Summary: Runtime libraries for %{name}
Requires: %{name}-common = %{?epoch:%{epoch}:}%{version}-%{release}
Requires: audiocd-kio-libs >= %{version}
Requires: libkcddb >= %{version}
Requires: libkcompactdisc >= %{version}
%description libs
%{summary}.

%package devel
Summary:  Developer files for %{name}
Requires: %{name}-libs = %{?epoch:%{epoch}:}%{version}-%{release}
Requires: audiocd-kio-devel
Requires: libkcddb-devel
Requires: libkcompactdisc-devel
%description devel
%{summary}.


%prep
# empty

%build
# empty

%install
# empty


%files 
# empty

%files common
# empty

%files libs
# empty

%files devel
# empty


%changelog
* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 6:4.10.5-2
- Mass rebuild 2013-12-27

* Sun Jun 30 2013 Than Ngo <than@redhat.com> - 4.10.5-1
- 4.10.5

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 6:4.10.4-1
- 4.10.4

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Sun Mar 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 6:4.10.2-1
- 4.10.2

* Tue Mar 05 2013 Rex Dieter <rdieter@fedoraproject.org> - 6:4.10.1-1
- 4.10.1

* Fri Feb 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 6:4.10.0-1
- 4.10.0

* Tue Jan 22 2013 Rex Dieter <rdieter@fedoraproject.org> - 6:4.9.98-1
- 4.9.98

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 6:4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.9.95-1
- 4.9.95

* Tue Dec 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.9.90-1
- 4.9.90

* Sat Nov 03 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.9.3-1
- 4.9.3

* Sat Sep 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.9.2-1
- 4.9.2

* Wed Sep 05 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.9.1-1
- 4.9.1

* Fri Jul 27 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.9.0-1
- 4.9.0

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 6:4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Jul 11 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.8.97-1
- 4.8.97

* Thu Jun 28 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.8.95-1
- 4.8.95

* Fri Jun 08 2012 Rex Dieter <rdieter@fedoraproject.org> 6:4.8.90-1
- metapackage

* Fri Jun 08 2012 Rex Dieter <rdieter@fedoraproject.org> 6:4.8.4-3
- +Provides: libkcddb(-devel) libkcompactdisc(-devel)

* Thu Jun 07 2012 Lukáš Tinkl <ltinkl@redhat.com> - 6:4.8.4-2
- respin, re-enable dragonplayer docs

* Wed Jun 06 2012 Than Ngo <than@redhat.com> - 6:4.8.4-1
- 4.8.4

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 6:4.8.3-1
- 4.8.3

* Tue Apr 24 2012 Rex Dieter <rdieter@fedoraproject.org> 6:4.8.2-4
- kmix crashes when pulseaudio exits (#804363)
- kmix is crashing after multiple volume changes (kde#290742)

* Fri Apr 13 2012 Rex Dieter <rdieter@fedoraproject.org> 6:4.8.2-3
- -dragonplayer: Provides: dragon

* Sat Apr 07 2012 Rex Dieter <rdieter@fedoraproject.org> 6:4.8.2-2
- 0100-kmixctrl-running-with-PulseAudio.patch from mageia

* Fri Mar 30 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.8.2-1
- 4.8.2

* Mon Mar 05 2012 Jaroslav Reznik <jreznik@redhat.com> - 6:4.8.1-1
- 4.8.1

* Tue Feb 28 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 6:4.8.0-2
- Rebuilt for c++ ABI breakage

* Sun Jan 22 2012 Rex Dieter <rdieter@fedoraproject.org> - 6:4.8.0-1
- 4.8.0

* Wed Jan 04 2012 Radek Novacek <rnovacek@redhat.com> - 6:4.7.97-1
- 4.7.97
- drop kmix patch (upstreamed)

* Fri Dec 30 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.7.95-2
- kmix chewing up cpu and memory (#766678,kde#288675)

* Wed Dec 21 2011 Radek Novacek <rnovacek@redhat.com> - 6:4.7.95-1
- 4.7.95

* Sun Dec 04 2011 Rex Dieter <rdieter@fedoraproject.org> - 6:4.7.90-1
- 4.7.90

* Mon Nov 28 2011 Jaroslav Reznik <jreznik@redhat.com> 6:4.7.80-1
- 4.7.80 (beta 1)

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.7.3-1
- 4.7.3

* Tue Oct 04 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.7.2-1
- 4.7.2

* Tue Sep 20 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.7.1-5
- drop monolithic conditional
- use pkgconfig deps

* Fri Sep 16 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> 6:4.7.1-4
- make Dragon Player use Phonon instead of xine-lib for DVD menus

* Wed Sep 14 2011 Radek Novacek <rnovacek@redhat.com> 6:4.7.1-2
- Fixed typo

* Tue Sep 06 2011 Than Ngo <than@redhat.com> - 6:4.7.1-1
- 4.7.1

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> 6:4.7.0-1
- 4.7.0

* Tue Jul 12 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.6.95-2
- more kmix_pulse_memleak work (kde#264089)

* Mon Jul 11 2011 Jaroslav Reznik <jreznik@redhat.com> - 6:4.6.95-1
- 4.6.95 (rc2)

* Mon Jun 27 2011 Than Ngo <than@redhat.com> - 6:4.6.90-1
- 4.6.90 (rc1)

* Tue Jun 14 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.6.80-2
- drop xine-lib support (f16+, #723935)

* Tue Jun 14 2011 Jaroslav Reznik <jreznik@redhat.com> 6:4.6.80-1
- 4.6.80 (beta1)

* Fri Apr 29 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.6.3-1
- 4.6.3

* Mon Apr 25 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.6.2-4
- re-enable kscd

* Tue Apr 19 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.6.2-3
- fix typo with %%_kde4_version macro usage

* Mon Apr 18 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.6.2-2
- modularize packaging (#623996, f16+)

* Wed Apr 06 2011 Than Ngo <than@redhat.com> - 6:4.6.2-1
- 4.6.2

* Mon Feb 28 2011 Rex Dieter <rdieter@fedoraproject.org> 6:4.6.1-1
- 4.6.1

* Fri Jan 21 2011 Jaroslav Reznik <jreznik@redhat.com> - 6:4.6.0-1
- 4.6.0

* Thu Jan 06 2011 Jaroslav Reznik <jreznik@redhat.com> - 6:4.5.95-1
- 4.5.95 (4.6rc2)

* Wed Dec 22 2010 <rdieter@fedoraproject.org> 6:4.5.90-1
- 4.5.90 (4.6rc1)

* Sat Dec 04 2010 Thomas Janssen <thomasj@fedoraproject.org> 6:4.5.85-1
- 4.5.85 (4.6beta2)

* Sun Nov 21 2010 Rex Dieter <rdieter@fedoraproject.org> 6:4.5.80-1
- 4.5.80 (4.6beta1)

* Sun Oct 31 2010 Than Ngo <than@redhat.com> - 6:4.5.3-1
- 4.5.3

* Sat Oct 02 2010 Rex Dieter <rdieter@fedoraproject.org> - 6:4.5.2-1
- 4.5.2

* Fri Aug 27 2010 Jaroslav Reznik <jreznik@redhat.com> - 6:4.5.1-1
- 4.5.1
- unconditionally omit mplayerthumbs (#627722)

* Tue Aug 03 2010 Than Ngo <than@redhat.com> - 6:4.5.0-1
- 4.5.0

* Sat Jul 25 2010 Rex Dieter <rdieter@fedoraproject.org> - 6:4.4.95-1
- 4.5 RC3 (4.4.95)

* Wed Jul 07 2010 Rex Dieter <rdieter@fedoraproject.org> - 6:4.4.92-1
- 4.5 RC2 (4.4.92)

* Fri Jun 25 2010 Jaroslav Reznik <jreznik@redhat.com> - 6:4.4.90-1
- 4.5 RC1 (4.4.90)

* Mon Jun 07 2010 Jaroslav Reznik <jreznik@redhat.com> - 6:4.4.85-1
- 4.5 Beta 2 (4.4.85)

* Fri May 21 2010 Jaroslav Reznik <jreznik@redhat.com> - 6:4.4.80-1
- 4.5 Beta 1 (4.4.80)

* Fri Apr 30 2010 Jaroslav Reznik <jreznik@redhat.com> - 6:4.4.3-1
- 4.4.3

* Sat Apr 03 2010 Rex Dieter <rdieter@fedoraproject.org> - 6:4.4.2-2
- sync kmix-pulse.patch : connect without a QEventLoop which can cause
  some strange issues

* Mon Mar 29 2010 Lukas Tinkl <ltinkl@redhat.com> - 6:4.4.2-1
- 4.4.2

* Fri Mar 12 2010 Rex Dieter <rdieter@fedoraproject.org> - 6:4.4.1-2
- sync kmix-pulse.patch from mdv

* Sat Feb 27 2010 Rex Dieter <rdieter@fedoraproject.org> - 6:4.4.1-1
- 4.4.1

* Fri Feb 05 2010 Than Ngo <than@redhat.com> - 6:4.4.0-1
- 4.4.0

* Sun Jan 31 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.98-1
- KDE 4.3.98 (4.4rc3)

* Fri Jan 29 2010 Rex Dieter <rdieter@fedoraproject.org. - 4.3.95-2
- respin kmix_pa patch

* Wed Jan 20 2010 Lukas Tinkl <ltinkl@redhat.com> - 4.3.95-1
- KDE 4.3.95 (4.4rc2)

* Sun Jan 17 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.90-4
- respin kmix_pa patch

* Thu Jan 14 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.90-3
- (re)enable mplayerthumbs (fedora-only)

* Mon Jan 11 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.90-2
- respin kmix_pa patch

* Wed Jan 06 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.90-1
- kde-4.3.90 (4.4rc1)

* Wed Jan 06 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.85-2
- (re)enable kmix/pa support, with patches from coling/mandriva
- tighten lib deps with %%{?_isa}
- deprecate Obsoletes: dragonplayer

* Fri Dec 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.85-1
- kde-4.3.85 (4.4beta2)

* Wed Dec 16 2009 Jaroslav Reznik <jreznik@redhat.com> - 4.3.80-2
- Repositioning the KDE Brand (#547361)

* Tue Dec  1 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.3.80-1
- KDE 4.4 beta1 (4.3.80)

* Mon Nov 23 2009 Ben Boeckel <MathStuf@gmail.com> - 4.3.75-0.1.svn1048496
- Update to 4.3.75 snapshot

* Tue Nov 10 2009 Than Ngo <than@redhat.com> - 4.3.3-2
- rhel cleanup
- don't install kscd doc if it's not built

* Sat Oct 31 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.3-1
- 4.3.3

* Mon Oct 05 2009 Than Ngo <than@redhat.com> - 4.3.2-1
- 4.3.2

* Fri Aug 28 2009 Than Ngo <than@redhat.com> - 4.3.1-1
- 4.3.1

* Thu Jul 30 2009 Than Ngo <than@redhat.com> - 4.3.0-1
- 4.3.0

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 6:4.2.98-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Wed Jul 22 2009 Than Ngo <than@redhat.com> - 4.2.98-1
- 4.3rc3

* Sat Jul 11 2009 Than Ngo <than@redhat.com> - 4.2.96-1
- 4.3rc2

* Fri Jun 26 2009 Than Ngo <than@redhat.com> - 4.2.95-1
- 4.3rc1

* Fri Jun 05 2009 Than Ngo <than@redhat.com> - 4.2.90-2
- fix build issue without xine-lib

* Thu Jun 04 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.90-1
- KDE-4.3 beta2 (4.2.90)

* Mon May 18 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.2.85-2
- Disable BR pulseaudio-libs-devel, KMix PA integration does not work yet.

* Wed May 13 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.85-1
- KDE 4.3 beta 1

* Wed Apr 01 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.2-2
- optimize scriptlets

* Tue Mar 31 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.2-1
- KDE 4.2.2

* Fri Feb 27 2009 Than Ngo <than@redhat.com> - 4.2.1-1
- 4.2.1

* Sun Feb 22 2009 Tejas Dinkar <tejas@gja.in> - 4.2.0-3
- fix kde#165249 No sound after second video file in Dragon Player (from 4.2.1)

* Sat Jan 31 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.2.0-2
- unowned dirs (#483516)

* Thu Jan 22 2009 Than Ngo <than@redhat.com> - 4.2.0-1
- 4.2.0

* Wed Jan 07 2009 Than Ngo <than@redhat.com> - 4.1.96-1
- 4.2rc1

* Sat Dec 13 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.85-2
- restore BR libtunepimp-devel libmusicbrainz-devel for now, needed by Kscd

* Fri Dec 12 2008 Than Ngo <than@redhat.com> 4.1.85-1
- 4.2beta2

* Fri Nov 28 2008 Lorenzo Villani <lvillani@binaryhelix.net> - 6:4.1.80-3
- dragon documentation disappeared (at least with my mock build), update file
  lists
- add kioslave documentation to file lists

* Thu Nov 20 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.80-3
- -devel: drop Req: kdebase-workspace-devel

* Thu Nov 20 2008 Than Ngo <than@redhat.com> 4.1.80-2
- merged

* Thu Nov 20 2008 Lorenzo Villani <lvillani@binaryhelix.net> - 6:4.1.80-1
- 4.1.80
- BR cmake >= 2.6.2
- make install/fast

* Wed Nov 12 2008 Than Ngo <than@redhat.com> 4.1.3-1
- 4.1.3

* Mon Sep 29 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-2
- make VERBOSE=1
- respin against new(er) kde-filesystem

* Fri Sep 26 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-1
- 4.1.2

* Wed Sep 17 2008 Than Ngo <than@redhat.com> 4.1.1-2
- backport from trunk to fix dragon kpart crash in the embedding application

* Fri Aug 29 2008 Than Ngo <than@redhat.com> 4.1.1-1
- 4.1.1

* Wed Jul 23 2008 Than Ngo <than@redhat.com> 4.1.0-1
- 4.1.0

* Fri Jul 18 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.99-1
- 4.0.99

* Fri Jul 11 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.98-1
- 4.0.98

* Sun Jul 06 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.85-1
- 4.0.85

* Fri Jun 27 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.84-1
- 4.0.84

* Thu Jun 19 2008 Than Ngo <than@redhat.com> 4.0.83-1
- 4.0.83 (beta2)

* Sun Jun 15 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.82-1
- 4.0.82

* Thu Jun 05 2008 Tom "spot" Callaway <tcallawa@redhat.com> 4.0.80-3
- some of the libraries are clearly LGPLv2+, correct license tags for libs and devel

* Thu Jun 05 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.80-2
- License: GPLv2+

* Mon May 26 2008 Than Ngo <than@redhat.com> 4.0.80-1
- 4.1 beta1

* Sat May 10 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.72-1
- update to 4.0.72
- drop backported kmix-systray patch
- Obsoletes/Provides dragonplayer, add it to description and file list
- add BR xine-lib-devel for dragonplayer

* Thu Apr 03 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-3
- rebuild (again) for the fixed %%{_kde4_buildtype}

* Tue Apr 01 2008 Lukáš Tinkl <ltinkl@redhat.com>
- fix kmix systray volume control tooltip

* Mon Mar 31 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-2
- Rebuild for NDEBUG

* Fri Mar 28 2008 Than Ngo <than@redhat.com> 4.0.3-1
- 4.0.3

* Thu Feb 28 2008 Than Ngo <than@redhat.com> 4.0.2-1
- 4.0.2

* Wed Jan 31 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.1-1
- kde-4.0.1

* Tue Jan 08 2008 Rex Dieter <rdieter[AT]fedoraproject.org> 4.0.0-1
- kde-4.0.0
- drop upstreamed cdparanoia patch

* Wed Jan 02 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-4
- don't mention kaudiocreator in description, it's not actually there
- apply patch by Alex Merry to support FLAC 1.1.3+ in kio_audiocd
- apply patch by Allen Winter to fix cdparanoia detection
- list audiocdencoder.h in file list (-devel)

* Fri Dec 14 2007 Rex Dieter <rdieter[AT]fedoraproject.org> 3.97.0-3
- -libs: Obsoletes: -extras(-libs)
- cleanup BR's
- omit parallel-install symlink hacks

* Wed Dec 12 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 3.97.0-2
- rebuild for changed _kde4_includedir

* Mon Dec 10 2007 Than Ngo <than@redhat.com> 3.97.0-1
- 3.97.0

* Fri Nov 30 2007 Sebastian Vahl <fedora@deadbabylon.de> 6:3.96.2-1
- kde-3.96.2

* Sat Nov 24 2007 Sebastian Vahl <fedora@deadbabylon.de> 6:3.96.1-1
- kde-3.96.1

* Fri Nov 23 2007 Sebastian Vahl <fedora@deadbabylon.de> 6:3.96.0-4
- libs subpkg
- also use epoch in changelog (also backwards)

* Mon Nov 19 2007 Sebastian Vahl <fedora@deadbabylon.de> 6:3.96.0-3
- BR: kde-filesystem >= 4

* Mon Nov 19 2007 Sebastian Vahl <fedora@deadbabylon.de> 6:3.96.0-2
- Requires: kdebase-workspace-devel
- BR: libXScrnSaver-devel
- BR: libXtst-devel

* Fri Nov 16 2007 Sebastian Vahl <fedora@deadbabylon.de> 6:3.96.0-1
- Initial version for Fedora
