kernelpath="linux-sh4"

# auxiliary targets for model-specific builds
release_common_utils:
#       remove the slink to busybox
	rm -f $(prefix)/release/sbin/halt
	cp -f $(targetprefix)/sbin/halt $(prefix)/release/sbin/
	cp $(buildprefix)/root/release/umountfs $(prefix)/release/etc/init.d/
	cp $(buildprefix)/root/release/rc $(prefix)/release/etc/init.d/
	cp $(buildprefix)/root/release/sendsigs $(prefix)/release/etc/init.d/
	chmod 755 $(prefix)/release/etc/init.d/umountfs
	chmod 755 $(prefix)/release/etc/init.d/rc
	chmod 755 $(prefix)/release/etc/init.d/sendsigs
	chmod 755 $(prefix)/release/etc/init.d/halt
	mkdir -p $(prefix)/release/etc/rc.d/rc0.d
	ln -s ../init.d $(prefix)/release/etc/rc.d
	ln -fs halt $(prefix)/release/sbin/reboot
	ln -fs halt $(prefix)/release/sbin/poweroff
	ln -s ../init.d/sendsigs $(prefix)/release/etc/rc.d/rc0.d/S20sendsigs
	ln -s ../init.d/umountfs $(prefix)/release/etc/rc.d/rc0.d/S40umountfs
	ln -s ../init.d/halt $(prefix)/release/etc/rc.d/rc0.d/S90halt
	mkdir -p $(prefix)/release/etc/rc.d/rc6.d
	ln -s ../init.d/sendsigs $(prefix)/release/etc/rc.d/rc6.d/S20sendsigs
	ln -s ../init.d/umountfs $(prefix)/release/etc/rc.d/rc6.d/S40umountfs
	ln -s ../init.d/reboot $(prefix)/release/etc/rc.d/rc6.d/S90reboot

release_spark:
	echo "EvoLux" > $(prefix)/release/etc/hostname

	cp $(buildprefix)/root/release/umountfs $(prefix)/release/etc/init.d/
	cp $(buildprefix)/root/release/rc $(prefix)/release/etc/init.d/
	cp $(buildprefix)/root/release/sendsigs $(prefix)/release/etc/init.d/
	rm -f $(prefix)/release/sbin/halt
	cp -f $(targetprefix)/sbin/halt $(prefix)/release/sbin/
	cp $(buildprefix)/root/release/halt_spark_enigma2 $(prefix)/release/etc/init.d/halt
	chmod 755 $(prefix)/release/etc/init.d/umountfs
	chmod 755 $(prefix)/release/etc/init.d/rc
	chmod 755 $(prefix)/release/etc/init.d/sendsigs
	chmod 755 $(prefix)/release/etc/init.d/halt
	mkdir -p $(prefix)/release/usr/script
	mkdir -p $(prefix)/release/etc/rc.d/rc0.d
	ln -s ../init.d $(prefix)/release/etc/rc.d
	ln -fs halt $(prefix)/release/sbin/reboot
	ln -fs halt $(prefix)/release/sbin/poweroff
	ln -s ../init.d/sendsigs $(prefix)/release/etc/rc.d/rc0.d/S20sendsigs
	ln -s ../init.d/umountfs $(prefix)/release/etc/rc.d/rc0.d/S40umountfs
	ln -s ../init.d/halt $(prefix)/release/etc/rc.d/rc0.d/S90halt
	mkdir -p $(prefix)/release/etc/rc.d/rc6.d
	ln -s ../init.d/sendsigs $(prefix)/release/etc/rc.d/rc6.d/S20sendsigs
	ln -s ../init.d/umountfs $(prefix)/release/etc/rc.d/rc6.d/S40umountfs
	ln -s ../init.d/reboot $(prefix)/release/etc/rc.d/rc6.d/S90reboot
	ln -fs init $(prefix)/release/sbin/telinit

	cp -f $(buildprefix)/root/release/fstab_spark $(prefix)/release/etc/fstab
	cp $(buildprefix)/root/boot/startup.mp4 $(prefix)/release/boot/startup.mp4

	mkdir -p $(prefix)/release/usr/local/share/enigma2/picon
	cp -f $(buildprefix)/root/usr/local/share/enigma2/keymap_spark.xml $(prefix)/release/usr/local/share/enigma2/keymap.xml
	cp -f $(buildprefix)/root/usr/sbin/iw* $(prefix)/release/usr/sbin/
	cp -f $(buildprefix)/root/usr/sbin/wpa* $(prefix)/release/usr/sbin/
	cp -f $(targetprefix)/usr/sbin/exportfs $(prefix)/release/usr/sbin/
	cp -f $(targetprefix)/usr/sbin/showmount $(prefix)/release/usr/sbin/
	cp -f $(targetprefix)/usr/sbin/rpc* $(prefix)/release/usr/sbin/
	mkdir -p $(prefix)/release/var
	cp -RP $(targetprefix)/var/lib $(prefix)/release/var/
	cp -RP $(buildprefix)/root/etc/Wireless $(prefix)/release/etc/
	cp -RP $(targetprefix)/etc/exports $(prefix)/release/etc/
	cp -f $(buildprefix)/root/usr/lib/libiw.so.29 $(prefix)/release/usr/lib/
	cp -f $(buildprefix)/root/usr/lib/libgnu* $(prefix)/release/usr/lib/
	cp -f $(buildprefix)/root/usr/lib/libgcrypt* $(prefix)/release/usr/lib/
	cp -f $(buildprefix)/root/usr/lib/libgpg* $(prefix)/release/usr/lib/
	cp -RP $(buildprefix)/root/usr/lib/libusb* $(prefix)/release/usr/lib/
	cp -RP $(buildprefix)/root/usr/lib/libiconv* $(prefix)/release/usr/lib/
	cp -RP $(buildprefix)/root/usr/lib/libpng* $(prefix)/release/usr/lib/
	cp -RP $(buildprefix)/root/etc/init.d/setupETH.sh $(prefix)/release/etc/init.d/
	cp -RP $(buildprefix)/root/etc/init.d/nfs* $(prefix)/release/etc/init.d/
if ENABLE_MULTI_YAFFS2
	cp -RP $(buildprefix)/root/sbin/mkyaffs2 $(prefix)/release/sbin/
	cp -RP $(buildprefix)/root/sbin/unspare2 $(prefix)/release/sbin/
endif

if ENABLE_P0209
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/smartcard/smartcard.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rtl8192cu/8192cu.ko $(prefix)/release/lib/modules/rt8192cu.ko
	cp -f $(buildprefix)/root/lib/modules/rt8712u_stm24-209.ko $(prefix)/release/lib/modules/rt8712u.ko
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rt2870sta/rt2870sta.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rt3070sta/rt3070sta.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rt5370sta/rt5370sta.ko $(prefix)/release/lib/modules/
endif
if ENABLE_P0210
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/smartcard/smartcard.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rtl871x/8712u.ko $(prefix)/release/lib/modules/rt8712u.ko
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rtl8192cu/8192cu.ko $(prefix)/release/lib/modules/rt8192cu.ko
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rt2870sta/rt2870sta.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rt3070sta/rt3070sta.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/wireless/rt5370sta/rt5370sta.ko $(prefix)/release/lib/modules/
endif

	[ -d $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/drivers/media/common/tuners ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/drivers/media/common/tuners/* $(prefix)/release/lib/modules/
	[ -d $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/drivers/media/dvb/dvb-usb ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/drivers/media/dvb/dvb-usb/* $(prefix)/release/lib/modules/
	[ -d $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/drivers/media/dvb/frontends ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/drivers/media/dvb/frontends/* $(prefix)/release/lib/modules/
	[ -f $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/drivers/net/tun.ko ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/drivers/net/tun.ko $(prefix)/release/lib/modules/

	cp -f $(buildprefix)/root/sbin/flash* $(prefix)/release/sbin
	cp -f $(buildprefix)/root/sbin/nand* $(prefix)/release/sbin

	cp -dp $(buildprefix)/root/etc/lircrc $(prefix)/release/etc/
if !ENABLE_SPARK7162
	cp -dp $(buildprefix)/root/etc/lircd_spark.conf $(prefix)/release/etc/lircd.conf
else
	cp -dp $(buildprefix)/root/etc/lircd_spark7162.conf $(prefix)/release/etc/lircd.conf
endif
#	cp -dp $(buildprefix)/root/etc/lircd_spark.conf.amiko $(prefix)/release/etc/lircd.conf.amiko
#	cp -dp $(buildprefix)/root/bin/evremote2.amiko $(prefix)/release/bin/
	cp -dp $(buildprefix)/root/usr/bin/functions.sh $(prefix)/release/usr/bin/
	cp -dp $(targetprefix)/usr/bin/lircd $(prefix)/release/usr/bin/
	cp -dp $(targetprefix)/usr/bin/irexec $(prefix)/release/usr/bin/

	$(INSTALL_DIR) $(prefix)/release/usr/share/fonts
	cp $(targetprefix)/usr/local/share/fonts/* $(prefix)/release/usr/share/fonts/

	cp $(kernelprefix)/linux-sh4/drivers/usb/serial/ftdi_sio.ko $(prefix)/release/lib/modules/ftdi_sio.ko
	cp $(kernelprefix)/linux-sh4/drivers/usb/serial/pl2303.ko $(prefix)/release/lib/modules
	cp $(kernelprefix)/linux-sh4/drivers/usb/serial/usbserial.ko $(prefix)/release/lib/modules

	[ -e $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/cpu_frequ/cpu_frequ.ko ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/cpu_frequ/cpu_frequ.ko $(prefix)/release/lib/modules || true
	[ -f $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/cec/cec.ko ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/cec/cec.ko $(prefix)/release/lib/modules/

#	cp -f $(buildprefix)/root/release/auto.usb $(prefix)/release/etc/
#	echo 'sda    -fstype=auto,noatime,nodiratime          :/dev/sda' >> $(prefix)/release/etc/auto.usb
#	echo 'sda1   -fstype=auto,noatime,nodiratime          :/dev/sda1' >> $(prefix)/release/etc/auto.usb
#	echo 'sda2   -fstype=auto,noatime,nodiratime          :/dev/sda2' >> $(prefix)/release/etc/auto.usb
#	echo 'sda3   -fstype=auto,noatime,nodiratime          :/dev/sda3' >> $(prefix)/release/etc/auto.usb

	rm -f $(prefix)/release/lib/firmware/dvb-fe-avl2108.fw
	rm -f $(prefix)/release/lib/firmware/dvb-fe-stv6306.fw
	rm -f $(prefix)/release/lib/firmware/dvb-fe-cx24116.fw
	rm -f $(prefix)/release/lib/firmware/dvb-fe-cx21143.fw
	rm -f $(prefix)/release/bin/evremote
	rm -f $(prefix)/release/bin/gotosleep
	rm -f $(prefix)/release/bin/vdstandby

	mv $(prefix)/release/lib/firmware/component_7111_mb618.fw $(prefix)/release/lib/firmware/component.fw
	rm $(prefix)/release/lib/firmware/component_7105_pdk7105.fw
	cp -RP $(buildprefix)/root/lib/firmware/* $(prefix)/release/lib/firmware/

	cp -f $(buildprefix)/root/usr/local/share/enigma2/po/de/LC_MESSAGES/enigma2.mo.pingulux $(prefix)/release/usr/local/share/enigma2/po/de/LC_MESSAGES/enigma2.mo
	cp -f $(buildprefix)/root/usr/bin/mkfs.jffs2 $(prefix)/release/usr/bin/
	cp -f $(buildprefix)/root/sbin/mkyaffs2 $(prefix)/release/sbin/
	( cd $(prefix) && cp -RP ../flash/spark/spark_oob.img $(prefix)/release/sbin/ )
	cp -RP $(buildprefix)/root/usr/lib/liblzo2.so.2* $(prefix)/release/usr/lib/
	cp -RP $(buildprefix)/root/lib/libproc* $(prefix)/release/lib/
	cp -RP $(targetprefix)/usr/lib/libevent*.so* $(prefix)/release/usr/lib/
	cp -RP $(targetprefix)/usr/lib/libnfsi*.so* $(prefix)/release/usr/lib/
	cp -RP $(targetprefix)/usr/lib/libopkg*.so* $(prefix)/release/usr/lib/
#	cp -RP $(buildprefix)/root/lib/libwrap* $(prefix)/release/lib/
	cp -RP $(buildprefix)/root/usr/bin/rpcinfo $(prefix)/release/usr/bin/
	cp -RP $(buildprefix)/root/sbin/ntpdate $(prefix)/release/sbin/
	rm $(prefix)/release/bin/ps
	mkdir -p $(prefix)/release/usr/lib/opkg/info
	touch $(prefix)/release/usr/lib/opkg/info/opkg.list
	mkdir -p $(prefix)/release/etc/opkg
	touch $(prefix)/release/etc/opkg/opkg.conf
	echo "src/gz graugans http://packages.evolux.yourweb.de" > $(prefix)/release/etc/opkg/opkg.conf
	cp -RP $(buildprefix)/root/bin/ps $(prefix)/release/bin/
	cp -RP $(buildprefix)/root/bin/dropbear $(prefix)/release/bin/
	cp -RP $(buildprefix)/root/bin/dropbearkey $(prefix)/release/bin/
	cp -RP $(buildprefix)/root/etc/init.d/dropbear $(prefix)/release/etc/init.d/
	mkdir -p $(prefix)/release/etc/dropbear
	cp -RP $(buildprefix)/root/usr/lib/python2.6/site-packages/* $(prefix)/release/usr/lib/python2.6/site-packages/
	cp -f $(buildprefix)/root/usr/bin/backup_pingulux_image.sh $(prefix)/release/usr/bin/
	cp -f $(buildprefix)/root/etc/init.d/Swap.sh $(prefix)/release/etc/init.d/
	cp -f $(buildprefix)/root/etc/init.d/ntpupdate.sh $(prefix)/release/etc/init.d/
	( cd $(prefix) && cd ../flash/spark/orig-spark-plugin/root/plugin/var/etc && cp * $(prefix)/release/etc/ )
	cp -f $(buildprefix)/root/bin/fw_printenv $(prefix)/release/bin/
	cp -f $(buildprefix)/root/bin/fw_setenv $(prefix)/release/bin/
	touch $(prefix)/release/etc/.fsck
	if [ ! -e $(prefix)/release/etc/rc.d/rc3.d ]; then \
		mkdir -p $(prefix)/release/etc/rc.d/rc3.d; \
	fi; 
	ln -sf ../init.d/Swap.sh $(prefix)/release/etc/rc.d/rc3.d/S51swap
	ln -sf ../init.d/ntpupdate.sh $(prefix)/release/etc/rc.d/rc3.d/S52ntpupdate
	cp -RP $(buildprefix)/root/etc/mumudvb $(prefix)/release/etc/
	cp -RP $(buildprefix)/root/bin/mumudvb $(prefix)/release/bin/
### del libcoolstream+libeplayer2 stuff as not needed for evolux-E2 ###
	rm -f $(prefix)/release/lib/libcool*
	rm -f $(prefix)/release/lib/libeplayer2*
# copy evo mc files ####
	rm -rf $(prefix)/release/usr/local/share/enigma2/skin_default
	cp -RP $(buildprefix)/root/usr/local/share/enigma2/skin_default $(prefix)/release/usr/local/share/enigma2/
	cp -RP $(buildprefix)/root/usr/local/share/enigma2/skin_default.xml $(prefix)/release/usr/local/share/enigma2/
	cp -RP $(buildprefix)/root/usr/local/share/enigma2/skin.xml $(prefix)/release/usr/local/share/enigma2/
	cp -RP $(buildprefix)/root/usr/local/share/enigma2/prev.png $(prefix)/release/usr/local/share/enigma2/
	cp -RP $(buildprefix)/root/usr/local/share/enigma2/EVO-blackGlass-HD $(prefix)/release/usr/local/share/enigma2/
	cp -RP $(buildprefix)/root/usr/local/share/enigma2/Vali.HD.nano $(prefix)/release/usr/local/share/enigma2/

#
# The main target depends on the model.
# IMPORTANT: it is assumed that only one variable is set. Otherwise the target name won't be resolved.
#
$(DEPDIR)/min-release $(DEPDIR)/std-release $(DEPDIR)/max-release $(DEPDIR)/ipk-release $(DEPDIR)/release: \
$(DEPDIR)/%release: release_base release_$(SPARK)
	touch $@


release-clean:
	rm -f $(DEPDIR)/release
	rm -f $(DEPDIR)/release_base
	rm -f $(DEPDIR)/release_$(SPARK)
	rm -f $(DEPDIR)/release_common_utils
	rm -f $(DEPDIR)/release_cube_common


# the following target creates the common file base
release_base:
	rm -rf $(prefix)/release || true
	$(INSTALL_DIR) $(prefix)/release && \
	$(INSTALL_DIR) $(prefix)/release/bin && \
	$(INSTALL_DIR) $(prefix)/release/sbin && \
	$(INSTALL_DIR) $(prefix)/release/boot && \
	$(INSTALL_DIR) $(prefix)/release/dev && \
	$(INSTALL_DIR) $(prefix)/release/dev.static && \
	$(INSTALL_DIR) $(prefix)/release/etc && \
	$(INSTALL_DIR) $(prefix)/release/etc/fonts && \
	$(INSTALL_DIR) $(prefix)/release/etc/init.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-down.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-post-down.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-pre-up.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-up.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/tuxbox && \
	$(INSTALL_DIR) $(prefix)/release/etc/enigma2 && \
	$(INSTALL_DIR) $(prefix)/release/media/hdd && \
	$(INSTALL_DIR) $(prefix)/release/media/hdd/movie && \
	$(INSTALL_DIR) $(prefix)/release/media/hdd/music && \
	$(INSTALL_DIR) $(prefix)/release/media/hdd/picture && \
	$(INSTALL_DIR) $(prefix)/release/lib && \
	$(INSTALL_DIR) $(prefix)/release/lib/modules && \
	$(INSTALL_DIR) $(prefix)/release/ram && \
	$(INSTALL_DIR) $(prefix)/release/var && \
	$(INSTALL_DIR) $(prefix)/release/var/opkg && \
	export CROSS_COMPILE=$(target)- && \
		$(MAKE) install -C @DIR_busybox@ CONFIG_PREFIX=$(prefix)/release && \
	ln -s  ../etc $(prefix)/release/var/etc && \
	cp -a $(targetprefix)/bin/* $(prefix)/release/bin/ && \
	ln -s /bin/showiframe $(prefix)/release/usr/bin/showiframe && \
	cp -dp $(targetprefix)/bin/hotplug $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/sbin/init $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/sbin/killall5 $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/sbin/portmap $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/sbin/mke2fs $(prefix)/release/sbin/ && \
	ln -sf /sbin/mke2fs $(prefix)/release/sbin/mkfs.ext2 && \
	ln -sf /sbin/mke2fs $(prefix)/release/sbin/mkfs.ext3 && \
	ln -sf /sbin/mke2fs $(prefix)/release/sbin/mkfs.ext4 && \
	ln -sf /sbin/mke2fs $(prefix)/release/sbin/mkfs.ext4dev && \
	cp -dp $(targetprefix)/sbin/fsck $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/sbin/e2fsck $(prefix)/release/sbin/ && \
	ln -sf /sbin/e2fsck $(prefix)/release/sbin/fsck.ext2 && \
	ln -sf /sbin/e2fsck $(prefix)/release/sbin/fsck.ext3 && \
	ln -sf /sbin/e2fsck $(prefix)/release/sbin/fsck.ext4 && \
	ln -sf /sbin/e2fsck $(prefix)/release/sbin/fsck.ext4dev && \
	cp -dp $(targetprefix)/sbin/jfs_fsck $(prefix)/release/sbin/ && \
	ln -sf /sbin/jfs_fsck $(prefix)/release/sbin/fsck.jfs && \
	cp -dp $(targetprefix)/sbin/jfs_mkfs $(prefix)/release/sbin/ && \
	ln -sf /sbin/jfs_mkfs $(prefix)/release/sbin/mkfs.jfs && \
	cp -dp $(targetprefix)/sbin/jfs_tune $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/sbin/fsck.nfs $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/sbin/sfdisk $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/sbin/tune2fs $(prefix)/release/sbin/ && \
	cp -dp $(targetprefix)/etc/init.d/portmap $(prefix)/release/etc/init.d/ && \
	cp -dp $(buildprefix)/root/etc/init.d/udhcpc $(prefix)/release/etc/init.d/ && \
	cp -a $(targetprefix)/dev/* $(prefix)/release/dev/ && \
	cp -dp $(targetprefix)/etc/fstab $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/group $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/host.conf $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/hostname $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/hosts $(prefix)/release/etc/ && \
	echo "127.0.0.1       localhost.localdomain   localhost" > $(prefix)/release/etc/hosts && \
	touch $(prefix)/release/etc/nsswitch.conf && \
	echo "hosts: files dns" > $(prefix)/release/etc/nsswitch.conf && \
	cp -dp $(targetprefix)/etc/inittab $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/localtime $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/mtab $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/passwd $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/profile $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/protocols $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/resolv.conf $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/services $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/shells $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/shells.conf $(prefix)/release/etc/ && \
	cp -dp $(buildprefix)/root/etc/timezone.xml $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/vsftpd.conf $(prefix)/release/etc/ && \
	sed "s@listen=YES@listen=NO@g" -i $(prefix)/release/etc/vsftpd.conf && \
	echo "8001 stream tcp nowait root /bin/streamproxy streamproxy" > $(prefix)/release/etc/inetd.conf && \
	echo "ftp stream tcp nowait root /usr/bin/vsftpd vsftpd" >> $(prefix)/release/etc/inetd.conf && \
	echo "telnet stream tcp nowait root /usr/sbin/telnetd telnetd -i -l /bin/login" >> $(prefix)/release/etc/inetd.conf && \
	echo "ssh stream tcp nowait root /bin/dropbear dropbear -i" >> $(prefix)/release/etc/inetd.conf && \
	cp -dp $(targetprefix)/etc/image-version $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/network/interfaces $(prefix)/release/etc/network/ && \
	cp -dp $(targetprefix)/etc/network/options $(prefix)/release/etc/network/ && \
	cp -dp $(targetprefix)/etc/init.d/umountfs $(prefix)/release/etc/init.d/ && \
	cp -dp $(targetprefix)/etc/init.d/sendsigs $(prefix)/release/etc/init.d/ && \
	cp -dp $(targetprefix)/etc/init.d/halt $(prefix)/release/etc/init.d/ && \
	cp $(buildprefix)/root/release/reboot $(prefix)/release/etc/init.d/ && \
	cp $(targetprefix)/etc/tuxbox/satellites.xml $(prefix)/release/etc/tuxbox/ && \
	cp $(targetprefix)/etc/tuxbox/cables.xml $(prefix)/release/etc/tuxbox/ && \
	cp $(targetprefix)/etc/tuxbox/terrestrial.xml $(prefix)/release/etc/tuxbox/ && \
	echo "576i50" > $(prefix)/release/etc/videomode && \
	cp -R $(targetprefix)/etc/fonts/* $(prefix)/release/etc/fonts/ && \
if !ENABLE_SPARK7162
	cp $(buildprefix)/root/release/rcS_stm23_24_spark $(prefix)/release/etc/init.d/rcS && \
else
	cp $(buildprefix)/root/release/rcS_stm23_24_spark7162 $(prefix)/release/etc/init.d/rcS && \
endif
	chmod 755 $(prefix)/release/etc/init.d/rcS && \
	cp $(buildprefix)/root/release/mountvirtfs $(prefix)/release/etc/init.d/ && \
	cp $(buildprefix)/root/release/mme_check $(prefix)/release/etc/init.d/ && \
	cp $(buildprefix)/root/release/mountall $(prefix)/release/etc/init.d/ && \
	cp $(buildprefix)/root/release/hostname $(prefix)/release/etc/init.d/ && \
	cp $(buildprefix)/root/release/vsftpd $(prefix)/release/etc/init.d/ && \
	cp $(buildprefix)/root/release/bootclean.sh $(prefix)/release/etc/init.d/ && \
	cp $(buildprefix)/root/release/networking $(prefix)/release/etc/init.d/ && \
	mkdir -p $(prefix)/release/var/run/lirc && \
	cp -rd $(targetprefix)/lib/* $(prefix)/release/lib/ && \
	$(USERS) chmod 755 -R $(prefix)/release/lib
	find $(prefix)/release/lib/ -name '*.so*' -exec sh4-linux-strip --strip-unneeded {} \;

	cp -dp $(targetprefix)/sbin/mkfs $(prefix)/release/sbin/

	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/stgfb/stmfb/stm_v4l2.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/stgfb/stmfb/stmvbi.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/stgfb/stmfb/stmvout.ko $(prefix)/release/lib/modules/
	cd $(targetprefix)/lib/modules/$(KERNELVERSION)/extra && \
	for mod in \
		sound/pseudocard/pseudocard.ko \
		sound/silencegen/silencegen.ko \
		stm/mmelog/mmelog.ko \
		stm/monitor/stm_monitor.ko \
		media/dvb/stm/dvb/stmdvb.ko \
		sound/ksound/ksound.ko \
		media/dvb/stm/mpeg2_hard_host_transformer/mpeg2hw.ko \
		media/dvb/stm/backend/player2.ko \
		media/dvb/stm/h264_preprocessor/sth264pp.ko \
		media/dvb/stm/allocator/stmalloc.ko \
		stm/platform/platform.ko \
		stm/platform/p2div64.ko \
		media/sysfs/stm/stmsysfs.ko \
	;do \
		echo `pwd` player2/linux/drivers/$$mod; \
		if [ -e player2/linux/drivers/$$mod ]; then \
			cp player2/linux/drivers/$$mod $(prefix)/release/lib/modules/; \
			sh4-linux-strip --strip-unneeded $(prefix)/release/lib/modules/`basename $$mod`; \
		else \
			touch $(prefix)/release/lib/modules/`basename $$mod`; \
		fi; \
		echo "."; \
	done
	echo "touched";

	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/avs/avs.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/boxtype/boxtype.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/simu_button/simu_button.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/e2_proc/e2_proc.ko $(prefix)/release/lib/modules/

	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/stgfb/stmfb/stmfb.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/multicom/embxshell/embxshell.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/multicom/embxmailbox/embxmailbox.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/multicom/embxshm/embxshm.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/multicom/mme/mme_host.ko $(prefix)/release/lib/modules/

	[ -e $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/pti/pti.ko ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/pti/pti.ko $(prefix)/release/lib/modules || true
	[ -e $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/pti_np/pti.ko ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/pti_np/pti.ko $(prefix)/release/lib/modules || true

	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/compcache/lzo-kmod/lzo1x_compress.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/compcache/lzo-kmod/lzo1x_decompress.ko $(prefix)/release/lib/modules/
#	cp $(buildprefix)/root/lib/modules/stm24/* $(prefix)/release/lib/modules/

	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/compcache/ramzswap.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/bpamem/bpamem.ko $(prefix)/release/lib/modules/
if !ENABLE_SPARK7162
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/stgfb/stmfb/stmcore-display-sti7111.ko $(prefix)/release/lib/modules/
else
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/stgfb/stmfb/stmcore-display-sti7105.ko $(prefix)/release/lib/modules/
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/frontends/*.ko $(prefix)/release/lib/modules/
endif
	cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/frontcontroller/aotom/aotom.ko $(prefix)/release/lib/modules/
	find $(prefix)/release/lib/modules/ -name '*.ko' -exec sh4-linux-strip --strip-unneeded {} \;

	rm -rf $(prefix)/release/lib/autofs
	rm -rf $(prefix)/release/lib/modules/$(KERNELVERSION)

	$(INSTALL_DIR) $(prefix)/release/media
	ln -s /media/hdd $(prefix)/release/hdd
	$(INSTALL_DIR) $(prefix)/release/media/dvd

	$(INSTALL_DIR) $(prefix)/release/mnt
	$(INSTALL_DIR) $(prefix)/release/mnt/usb
	$(INSTALL_DIR) $(prefix)/release/mnt/hdd
	$(INSTALL_DIR) $(prefix)/release/mnt/nfs

	$(INSTALL_DIR) $(prefix)/release/root

	$(INSTALL_DIR) $(prefix)/release/proc
	$(INSTALL_DIR) $(prefix)/release/sys
	$(INSTALL_DIR) $(prefix)/release/tmp

	$(INSTALL_DIR) $(prefix)/release/usr
	$(INSTALL_DIR) $(prefix)/release/usr/bin
	cp -p $(targetprefix)/usr/sbin/vsftpd $(prefix)/release/usr/bin/
	cp -p $(targetprefix)/usr/bin/python $(prefix)/release/usr/bin/

	cp -p $(targetprefix)/usr/bin/killall $(prefix)/release/usr/bin/
	cp -p $(targetprefix)/usr/bin/opkg-cl $(prefix)/release/usr/bin/opkg
	cp -p $(targetprefix)/usr/bin/ffmpeg $(prefix)/release/sbin/
	$(INSTALL_DIR) $(prefix)/release/usr/tuxtxt
	cp -p $(targetprefix)/usr/bin/tuxtxt $(prefix)/release/usr/bin/
	cp $(buildprefix)/root/usr/tuxtxt/tuxtxt2.conf $(prefix)/release/usr/tuxtxt/
	if [ -e $(buildprefix)/.deps/neutrino ]; then \
		cp -p $(targetprefix)/usr/bin/grab $(prefix)/release/usr/bin/;fi
	cp -p $(targetprefix)/usr/sbin/ethtool $(prefix)/release/usr/sbin/

	$(INSTALL_DIR) $(prefix)/release/usr/share

	ln -s /usr/local/share/enigma2 $(prefix)/release/usr/share/enigma2

	$(INSTALL_DIR) $(prefix)/release/usr/share/fonts
	cp $(targetprefix)/usr/share/fonts/ae_AlMateen.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/lcd.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/md_khmurabi_10.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/nmsbd.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/tuxtxt.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/goodtime.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/valis_enigma.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/valis_lcd.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/seg.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/seg_internat.ttf $(prefix)/release/usr/share/fonts/
	cp $(targetprefix)/usr/share/fonts/Symbols.ttf $(prefix)/release/usr/share/fonts/

#       Font libass
	cp $(buildprefix)/root/usr/share/fonts/FreeSans.ttf $(prefix)/release/usr/share/fonts/

	$(INSTALL_DIR) $(prefix)/release/usr/share/zoneinfo
	cp -aR $(buildprefix)/root/usr/share/zoneinfo/* $(prefix)/release/usr/share/zoneinfo/

	$(INSTALL_DIR) $(prefix)/release/usr/share/udhcpc
	cp -aR $(buildprefix)/root/usr/share/udhcpc/* $(prefix)/release/usr/share/udhcpc/

	if [ -e $(targetprefix)/usr/share/alsa ]; then \
		mkdir $(prefix)/release/usr/share/alsa/; \
		mkdir $(prefix)/release/usr/share/alsa/cards/; \
		mkdir $(prefix)/release/usr/share/alsa/pcm/; \
		cp $(targetprefix)/usr/share/alsa/alsa.conf $(prefix)/release/usr/share/alsa/alsa.conf; \
		cp $(targetprefix)/usr/share/alsa/cards/aliases.conf $(prefix)/release/usr/share/alsa/cards/; \
		cp $(targetprefix)/usr/share/alsa/pcm/default.conf $(prefix)/release/usr/share/alsa/pcm/; \
		cp $(targetprefix)/usr/share/alsa/pcm/dmix.conf $(prefix)/release/usr/share/alsa/pcm/; fi

	ln -s /usr/local/share/keymaps $(prefix)/release/usr/share/keymaps

	$(INSTALL_DIR) $(prefix)/release/usr/local
	$(INSTALL_DIR) $(prefix)/release/usr/local/bin
	if [ -e $(targetprefix)/usr/bin/enigma2 ]; then \
		cp -f $(targetprefix)/usr/bin/enigma2 $(prefix)/release/usr/local/bin/enigma2;fi
	if [ -e $(targetprefix)/usr/local/bin/enigma2 ]; then \
		cp -f $(targetprefix)/usr/local/bin/enigma2 $(prefix)/release/usr/local/bin/enigma2;fi
	find $(prefix)/release/usr/local/bin/ -name  enigma2 -exec sh4-linux-strip --strip-unneeded {} \;

	ln -s /etc $(prefix)/release/usr/local/etc

	$(INSTALL_DIR) $(prefix)/release/usr/local/share
	$(INSTALL_DIR) $(prefix)/release/usr/local/share/keymaps
	$(INSTALL_DIR) $(prefix)/release/usr/local/share/enigma2
	cp -a $(targetprefix)/usr/local/share/enigma2/* $(prefix)/release/usr/local/share/enigma2
	cp -a $(targetprefix)/etc/enigma2/* $(prefix)/release/etc/enigma2

	ln -s /usr/share/fonts $(prefix)/release/usr/local/share/fonts

	$(INSTALL_DIR) $(prefix)/release/usr/lib
	cp -R $(targetprefix)/usr/lib/* $(prefix)/release/usr/lib/
	rm -rf $(prefix)/release/usr/lib/libnfsidmap
	rm -rf $(prefix)/release/usr/lib/engines
#	[ -e $(prefix)/release/usr/lib/tuxbox ] && rm -rf $(prefix)/release/usr/lib/tuxbox
	rm -rf $(prefix)/release/usr/lib/enigma2
	rm -rf $(prefix)/release/usr/lib/gconv
	rm -rf $(prefix)/release/usr/lib/libxslt-plugins
	rm -rf $(prefix)/release/usr/lib/pkgconfig
	rm -rf $(prefix)/release/usr/lib/python2.6
	rm -rf $(prefix)/release/usr/lib/sigc++-1.2
	rm -rf $(prefix)/release/usr/lib/X11
	chmod 755 -R $(prefix)/release/usr/lib/
	find $(prefix)/release/usr/lib/ -name '*.so*' -exec sh4-linux-strip --strip-unneeded {} \;

	$(INSTALL_DIR) $(prefix)/release/usr/lib/enigma2
	cp -a $(targetprefix)/usr/lib/enigma2/* $(prefix)/release/usr/lib/enigma2/
	if test -d $(targetprefix)/usr/local/lib/enigma2; then \
		cp -a $(targetprefix)/usr/local/lib/enigma2/* $(prefix)/release/usr/lib/enigma2/; fi

	cp -RP $(targetprefix)/usr/lib/enigma2/python/Plugins/SystemPlugins/Tuxtxt $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/
	cp -RP $(buildprefix)/root/usr/lib/enigma2/python $(prefix)/release/usr/lib/enigma2/

#	Dont remove pyo files, remove pyc instead
	find $(prefix)/release/usr/lib/enigma2/ -name '*.pyc' -exec rm -f {} \;
	find $(prefix)/release/usr/lib/enigma2/ -name '*.pyo' -exec rm -f {} \;
	find $(prefix)/release/usr/lib/enigma2/ -name '*.so*' -exec sh4-linux-strip --strip-unneeded {} \;

#	Delete unnecessary plugins
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/DemoPlugins
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/FrontprocessorUpgrade
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/NFIFlash
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/Extensions/FileManager
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/Extensions/DVDBurn
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/Extensions/DVDPlayer
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/Extensions/CutListEditor
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/Extensions/MediaScanner
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/Extensions/TuxboxPlugins
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/SoftwareManager
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/CrashlogAutoSubmit
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/TempFanControl
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/DefaultServicesScanner
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/DiseqcTester
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/SystemPlugins/CommonInterfaceAssignment

	$(INSTALL_DIR) $(prefix)/release/usr/lib/python2.6
	cp -a $(targetprefix)/usr/lib/python2.6/* $(prefix)/release/usr/lib/python2.6/
	rm -rf $(prefix)/release/usr/lib/python2.6/test
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/lxml
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/lxml-2.0.5-py2.6.egg-info
	rm -f $(prefix)/release/usr/lib/python2.6/site-packages/libxml2mod.so
	rm -f $(prefix)/release/usr/lib/python2.6/site-packages/libxsltmod.so
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/OpenSSL/test
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/setuptools
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/setuptools-0.6c8-py2.6.egg-info
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/zope.interface-3.3.0-py2.6.egg-info
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/test
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/conch
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/mail
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/manhole
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/names
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/news
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/trial
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/words
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/application
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/enterprise
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/flow
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/lore
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/pair
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/runner
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/scripts
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/tap
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/twisted/topfiles
	rm -rf $(prefix)/release/usr/lib/python2.6/site-packages/Twisted-8.2.0-py2.6.egg-info
	rm -rf $(prefix)/release/usr/lib/python2.6/bsddb
	rm -rf $(prefix)/release/usr/lib/python2.6/compiler
	rm -rf $(prefix)/release/usr/lib/python2.6/config
	rm -rf $(prefix)/release/usr/lib/python2.6/ctypes
	rm -rf $(prefix)/release/usr/lib/python2.6/curses
	rm -rf $(prefix)/release/usr/lib/python2.6/distutils
	rm -rf $(prefix)/release/usr/lib/python2.6/email

#	Dont remove pyo files, remove pyc instead
	find $(prefix)/release/usr/lib/python2.6/ -name '*.pyc' -exec rm -f {} \;
	find $(prefix)/release/usr/lib/python2.6/ -name '*.pyo' -exec rm -f {} \;
	find $(prefix)/release/usr/lib/python2.6/ -name '*.so*' -exec sh4-linux-strip --strip-unneeded {} \;

	cp -RP $(buildprefix)/root/etc/modules.available $(prefix)/release/etc/
	if [ ! -e $(prefix)/release/usr/script ]; then \
		mkdir -p $(prefix)/release/usr/script; \
	fi
	cp -RP $(buildprefix)/root/usr/script/user_script.sh.example $(prefix)/release/usr/script/
	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/Extensions/FlashBackup
#### displaytime plugin not longer needed so we remove it #####
#	rm -rf $(prefix)/release/usr/lib/enigma2/python/Plugins/Extensions/PPDisplayTime
	cp -RP $(buildprefix)/root/etc/enigma2/* $(prefix)/release/etc/enigma2/
	cp -RP $(buildprefix)/root/etc/tuxbox/satellites.xml $(prefix)/release/etc/tuxbox/
	touch $(prefix)/release/etc/.start_enigma2
#	cp -RP $(buildprefix)/root/usr/lib/enigma2/python/Components/Converter $(prefix)/release/usr/lib/enigma2/python/Components/
	rm $(prefix)/release/usr/local/share/enigma2/keymap_*.xml
	touch $(prefix)/release/etc/changelog.txt
	cp -a $(buildprefix)/root/usr/bin/amixer $(prefix)/release/usr/bin/
	find $(prefix)/release/usr/bin/ -name 'amixer' -exec sh4-linux-strip --strip-unneeded {} \;
######## FOR YOUR OWN CHANGES use these folder in cdk/own_build/enigma2 #############
	rm $(prefix)/release/bin/showiframe
	rm $(prefix)/release/usr/local/share/enigma2/radio.mvi
	rm $(prefix)/release/bin/stslave

	cp -RP $(buildprefix)/own_build/enigma2/* $(prefix)/release/
	cp -RP $(buildprefix)/root/bin/fbshot $(prefix)/release/bin/

#	cp $(kernelprefix)/linux-sh4/arch/sh/boot/uImage $(prefix)/release/boot/

	[ -e $(kernelprefix)/linux-sh4/drivers/net/wireless/ath/ath.ko ] && cp $(kernelprefix)/linux-sh4/drivers/net/wireless/ath/ath.ko $(prefix)/release/lib/modules/ath.ko || true
	[ -e $(kernelprefix)/linux-sh4/drivers/net/wireless/zd1201.ko ] && cp $(kernelprefix)/linux-sh4/drivers/net/wireless/zd1201.ko $(prefix)/release/lib/modules/zd1201.ko || true
	[ -e $(kernelprefix)/linux-sh4/drivers/usb/serial/ftdi_sio.ko ] && cp $(kernelprefix)/linux-sh4/drivers/usb/serial/ftdi_sio.ko $(prefix)/release/lib/modules/ftdi_sio.ko || true
	[ -e $(kernelprefix)/linux-sh4/drivers/usb/serial/pl2303.ko ] && cp $(kernelprefix)/linux-sh4/drivers/usb/serial/pl2303.ko $(prefix)/release/lib/modules || true
	[ -e $(kernelprefix)/linux-sh4/drivers/usb/serial/usbserial.ko ] && cp $(kernelprefix)/linux-sh4/drivers/usb/serial/usbserial.ko $(prefix)/release/lib/modules || true
	[ -e $(kernelprefix)/linux-sh4/fs/ntfs/ntfs.ko ] && cp $(kernelprefix)/linux-sh4/fs/ntfs/ntfs.ko $(prefix)/release/lib/modules || true
	[ -e $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/cpu_frequ/cpu_frequ.ko ] && cp $(targetprefix)/lib/modules/$(KERNELVERSION)/extra/cpu_frequ/cpu_frequ.ko $(prefix)/release/lib/modules || true
	[ -e cp $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/fs/autofs4/autofs4.ko ] && $(targetprefix)/lib/modules/$(KERNELVERSION)/kernel/fs/autofs4/autofs4.ko $(prefix)/release/lib/modules/ || true

	find $(prefix)/release/ -name '*.a' -exec rm -f {} \;
	find $(prefix)/release/ -name '*.o' -exec rm -f {} \;
	find $(prefix)/release/ -name '*.la' -exec rm -f {} \;

#
# AUTOFS
#
#	cp -f $(targetprefix)/usr/sbin/automount $(prefix)/release/usr/sbin/; \
#	ln -s /usr/lib/autofs/mount_ext2.so $(prefix)/release/usr/lib/autofs/mount_ext3.so; \
#	cp $(kernelprefix)/$(kernelpath)/fs/autofs4/autofs4.ko $(prefix)/release_neutrino/lib/modules; \
#	cp -f $(buildprefix)/root/release/auto.usb $(prefix)/release/etc/;
	cp -f $(buildprefix)/root/release/autofs $(prefix)/release/etc/init.d/
	cp -f $(buildprefix)/root/usr/sbin/automount $(prefix)/release/usr/sbin/
	cp -f $(buildprefix)/root/release/auto.hotplug $(prefix)/release/etc/
	cp -f $(buildprefix)/root/release/auto.network $(prefix)/release/etc/

#
# GSTREAMER
#
	if [ -d $(prefix)/release/usr/lib/gstreamer-0.10 ]; then \
		rm -rf $(prefix)/release/usr/lib/libgstfft*; \
		rm -rf $(prefix)/release/usr/lib/gstreamer-0.10/*; \
		cp -a $(targetprefix)/usr/bin/gst-launch* $(prefix)/release/usr/bin/; \
		sh4-linux-strip --strip-unneeded $(prefix)/release/usr/bin/gst-launch*; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstalsa.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstapp.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstasf.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstassrender.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstaudioconvert.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstaudioparsersbad.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstaudioresample.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstautodetect.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstavi.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstcdxaparse.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstcoreelements.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstcoreindexers.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstdecodebin.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstdecodebin2.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstdvbaudiosink.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstdvbvideosink.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstdvdsub.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstflac.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstflv.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstfragmented.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgsticydemux.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstid3demux.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstisomp4.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstmad.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstmatroska.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstmpegaudioparse.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstmpegdemux.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstmpegstream.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstogg.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstplaybin.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstrtmp.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstrtp.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstrtpmanager.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstrtsp.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstsouphttpsrc.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstsubparse.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgsttypefindfunctions.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstudp.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstvcdsrc.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstwavparse.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		if [ -e $(prefix)/release/usr/lib/gstreamer-0.10/libgstffmpeg.so ]; then \
			cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstffmpeg.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
			cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstffmpegscale.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
			cp -a $(targetprefix)/usr/lib/gstreamer-0.10/libgstpostproc.so $(prefix)/release/usr/lib/gstreamer-0.10/; \
		fi; \
		sh4-linux-strip --strip-unneeded $(prefix)/release/usr/lib/gstreamer-0.10/*; \
	fi

#
# DIRECTFB
#
	if [ -d $(prefix)/release/usr/lib/directfb-1.4-5 ]; then \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/gfxdrivers/*.a; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/gfxdrivers/*.la; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/gfxdrivers/*.o; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/inputdrivers/*; \
		cp -a $(targetprefix)/usr/lib/directfb-1.4-5/inputdrivers/libdirectfb_enigma2remote.so $(prefix)/release/usr/lib/directfb-1.4-5/inputdrivers/; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/systems/*.a; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/systems/*.la; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/systems/*.o; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/systems/libdirectfb_dummy.so; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/systems/libdirectfb_fbdev.so; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/wm/*.a; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/wm/*.la; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/wm/*.o; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBFont/*.a; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBFont/*.la; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBFont/*.o; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBImageProvider/*.a; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBImageProvider/*.la; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBImageProvider/*.o; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBVideoProvider/*.a; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBVideoProvider/*.la; \
		rm -rf $(prefix)/release/usr/lib/directfb-1.4-5/interfaces/IDirectFBVideoProvider/*.o; \
	fi
	if [ -d $(prefix)/release/usr/lib/icu ]; then \
		rm -rf $(prefix)/release/usr/lib/icu; \
	fi
	if [ -d $(prefix)/release/usr/lib/glib-2.0 ]; then \
		rm -rf $(prefix)/release/usr/lib/glib-2.0; \
	fi
	if [ -d $(prefix)/release/usr/lib/gio ]; then \
		rm -rf $(prefix)/release/usr/lib/gio; \
	fi
	if [ -d $(prefix)/release/usr/lib/enchant ]; then \
		rm -rf $(prefix)/release/usr/lib/enchant; \
	fi

#graphlcd Stuff
	if [ -e $(prefix)/release/usr/lib/libglcddrivers.so ]; then \
		( cd $(prefix)/release/usr/lib && ln -sf libglcddrivers.so.2.1.0 libglcddrivers.so && ln -sf libglcddrivers.so.2.1.0 libglcddrivers.so.2 && ln -sf libglcdgraphics.so.2.1.0 libglcdgraphics.so && ln -sf libglcdgraphics.so.2.1.0 libglcdgraphics.so.2 &&  ln -sf libglcdskin.so.2.1.0 libglcdskin.so &&  ln -sf libglcdskin.so.2.1.0 libglcdskin.so.2 ); \
		cp -f $(targetprefix)/etc/graphlcd.conf $(prefix)/release/etc/graphlcd.conf; \
	fi
