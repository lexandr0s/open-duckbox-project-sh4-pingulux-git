
URLDIV1=ftp://mirrors.kernel.org/fedora/core/6/i386/os/Fedora/RPMS
URL=ftp://ftp.stlinux.com/pub/stlinux/2.0/ST_Linux_2.0/RPM_Distribution/sh4-target-glibc-packages
URLU=ftp://ftp.stlinux.com/pub/stlinux/2.0/ST_Linux_2.0/RPM_Distribution/sh4-updates
URL2=ftp://ftp.stlinux.com/pub/stlinux/2.2/STLinux/sh4
URL2U=ftp://ftp.stlinux.com/pub/stlinux/2.2/updates/RPMS/sh4
URL3=ftp://ftp.stlinux.com/pub/stlinux/2.3/STLinux/sh4
URL3U=ftp://ftp.stlinux.com/pub/stlinux/2.3/updates/RPMS/sh4
URL4=ftp://ftp.stlinux.com/pub/stlinux/2.4/STLinux/sh4
URL4U=ftp://ftp.stlinux.com/pub/stlinux/2.4/updates/RPMS/sh4


URLSH=http://ftp.stlinux.com/pub/stlinux/2.0/ST_Linux_2.0/SRPM_Distribution/host-SRPMS
URLSHU=http://ftp.stlinux.com/pub/stlinux/2.0/ST_Linux_2.0/SRPM_Distribution/sh4-SRPMS-updates
URL2SH=ftp://ftp.stlinux.com/pub/stlinux/2.2/SRPMS/
URL2SHU=ftp://ftp.stlinux.com/pub/stlinux/2.2/updates/SRPMS
URLS=ftp://ftp.stlinux.com/pub/stlinux/2.0/ST_Linux_2.0/SRPM_Distribution/sh4-SRPMS
URLSU=ftp://ftp.stlinux.com/pub/stlinux/2.0/ST_Linux_2.0/SRPM_Distribution/sh4-SRPMS-updates
URL2S=ftp://ftp.stlinux.com/pub/stlinux/2.2/SRPMS
URL2SU=ftp://ftp.stlinux.com/pub/stlinux/2.2/updates/SRPMS
URL3S=ftp://ftp.stlinux.com/pub/stlinux/2.3/SRPMS
URL3SU=ftp://ftp.stlinux.com/pub/stlinux/2.3/updates/SRPMS
URL4S=ftp://ftp.stlinux.com/pub/stlinux/2.4/SRPMS
URL4SU=ftp://ftp.stlinux.com/pub/stlinux/2.4/updates/SRPMS

Archive/bash-3.1-16.1.i386.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URLDIV1)/$(notdir $@)) || true
Archive/stlinux20-sh4-%.sh4.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL)/$(notdir $@) || $(WGET) $(URLU)/$(notdir $@)) || true
Archive/stlinux22-sh4-%.sh4.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL2)/$(notdir $@) || $(WGET) $(URL2U)/$(notdir $@)) || true
Archive/stlinux23-sh4-%.sh4.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL3)/$(notdir $@) || $(WGET) $(URL3U)/$(notdir $@)) || true
Archive/stlinux24-sh4-%.sh4.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL4)/$(notdir $@) || $(WGET) $(URL4U)/$(notdir $@)) || true

Archive/stlinux20-host-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URLSH)/$(notdir $@) || $(WGET) $(URLSHU)/$(notdir $@)) || true
Archive/stlinux22-host-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL2SH)/$(notdir $@) || $(WGET) $(URL2SHU)/$(notdir $@)) || true
Archive/stlinux23-host-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL3S)/$(notdir $@) || $(WGET) $(URL3SU)/$(notdir $@)) || true
Archive/stlinux24-host-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL4S)/$(notdir $@) || $(WGET) $(URL4SU)/$(notdir $@)) || true
Archive/stlinux20-cross-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URLSH)/$(notdir $@) || $(WGET) $(URLSHU)/$(notdir $@)) || true
Archive/stlinux22-cross-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL2SH)/$(notdir $@) || $(WGET) $(URL2SHU)/$(notdir $@)) || true
Archive/stlinux23-cross-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL3S)/$(notdir $@) || $(WGET) $(URL3SU)/$(notdir $@)) || true
Archive/stlinux24-cross-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URL4S)/$(notdir $@) || $(WGET) $(URL4SU)/$(notdir $@)) || true
Archive/stlinux20-target-%.src.rpm:
	[ ! -f Archive/$(notdir $@) ] && \
	(cd Archive && $(WGET) $(URLS)/$(notdir $@) || $(WGET) $(URLSU)/$(notdir $@)) || true
Archive/stlinux22-target-%.src.rpm:
	[ ! -f Archive/$@ ] && \
	(cd Archive && $(WGET) $(URL2S)/$(notdir $@) || $(WGET) $(URL2SU)/$(notdir $@)) || true
Archive/stlinux23-target-%.src.rpm:
	[ ! -f Archive/$@ ] && \
	(cd Archive && $(WGET) $(URL3S)/$(notdir $@) || $(WGET) $(URL3SU)/$(notdir $@)) || true
Archive/stlinux24-target-%.src.rpm:
	[ ! -f Archive/$@ ] && \
	(cd Archive && $(WGET) $(URL4S)/$(notdir $@) || $(WGET) $(URL4SU)/$(notdir $@)) || true
