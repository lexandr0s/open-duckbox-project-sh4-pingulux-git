#!/bin/bash
CURDIR=`pwd`
if [ ! -e /usr/bin/gcc-4.5 ]; then
	sudo apt-get install -y gcc-4.5 \
	gcc-4.5-base \
	gcc-4.5-multilib
fi
cd /usr/bin
sudo ln -sf gcc-4.5 gcc
#cd "$CURDIR/Patches"
#ln -sf stm-cross-gcc.spec.4.5.2-78-ubu11.10-64bit.diff stm-cross-gcc.spec.4.5.2-78.diff
cd "$CURDIR"
./make.sh
echo "Targets:"
echo " 1) Build yaud-neutrino"
echo " 2) Build yaud-enigma2-nightly"
if [ "$REPLY" == "1" ]; then
	make yaud-neutrino
elif [ "$REPLY" == "2" ]; then
	make yaud-enigma2-nightly
else
	exit
fi
cd /usr/bin
sudo ln -sf gcc-4.6 gcc
#cd "$CURDIR/Patches"
#ln -sf stm-cross-gcc.spec.4.5.2-78-default.diff stm-cross-gcc.spec.4.5.2-78.diff
cd "$CURDIR"
exit

