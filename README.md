HOWTO: Get Azurewave (old Twinhan) PCTV DTV card working under Linux
====================================================================

Driver Status
-------------
Date     : 2007.04.27

Version  : beta v1.4.2(CI_bin_for_FC6)

Supported cards:
	VP1025    DVB-S
	VP1027    DVB-S
	VP1041    DVB-S/S2 + CI
	VP1034    DVB-S + CI
	VP2033    DVB-C + CI
	VP3030    DVB-T + CI

Features:
	This driver is fit in with linuxTV spec. It can support DVB-T, DVB-C and DVB-S with different card type. It also support muti-cards (upto 4 cards) and CI function.

System require:
	linux kernel up to 2.6.18-1.2798
	kernel devel package
	binutils - suggested version is 2.11.x.
	gcc - suggested versions are: 2.95.3 (maybe 2.95.4) and 3.2+.
	XFree86 - suggested version is always the newest (4.3).
	make - suggested version is always the newest (at least 3.79.x).


Package Status
--------------
dvb-apps-997424a1799e:
	Linux DVB API applications and utilities.

linuxdriver:
	Driver source code and install script.

script:
	Script files for lock channel and playback.


Installation
------------
	1) Install Linux DVB API applications and utilities
		$ cd dvb-apps-997424a1799e
		$ make clean
		$ make
		$ make install
		$ cd ..

	2) Install Azurewave card driver 
	  	$ cd linuxdriver
    		$ make rmmod
    		$ make insmod

	    for VP1034,VP1041,VP2033,VP3030 CI support
		insert CAM in card slot
				
	    Depending on the card's type, the device driver will expose some of the following device nodes for LinuxTV API.
                  * /dev/dvb/adapter0/ca0
                  * /dev/dvb/adapter0/demux0
                  * /dev/dvb/adapter0/dvr0
                  * /dev/dvb/adapter0/frontend0
                  * /dev/dvb/adapter0/net0

	3) Dvbs-s2 switch(only for VP1041)
	    We provide the dvbs-s2 switch tool in dvb-apps-997424a1799e. If you have installed the dvb-apps, you can run "switch" to know its usage. 
	    For example, you can run the following command to select the DVBS or DVBS2.
	    	$ switch -a 0 -f 0 -t dvbs
		or
		$ switch -a 0 -f 0 -t dvbs2

	    If you got "set DvbsMode ok!", it indicates you switch Dvbs-s2 OK.

Usage 
-----
	There is two usages of Azurewave PCTV DTV card. The first way can be used to play CI and FTA program by all supported cards.The second way is used to play free-to-air program by DVB-C and DVB-T cards.

	Way 1:
	1) Scan channel 
	    You may modify/add/delete the dvb-apps-997424a1799e/util/scan/dvb-s(dvb-c,dvb-t)/initial-tuning-data-file depend on your transponder settings. If you are lucky you can just use one of the supplied initial-tuning-data-file.
            We prepare script file for convenient usage. You can just modify the script for different initial-tuning-data-file.
		$ cd script/dvb-s (dvb-t, dvb-c)/
	    	$ ./scan.sh
	    You may modify the channel list(channels.conf_test)'s channels name,because chinese word can't be read.

	2) Lock channel
 	    We prepare script file for convenient usage. You can just modify the script for different lock parameters.
	    The following command can't be closed during program playback. You can run it at separate console or in background mode.
		$ ./locktv.sh
	    If you got "...FE_HAS_LOCK", it indicates you can lock the channel else you doesn't lock the channel.

	    Enable CI module (Not must)
		in other console
		$ cd script/dvb-s (dvb-t, dvb-c)/
		$ ./capmt.sh
	    If you got "...Send PMT to CAM ok!", it indicates you execute the capmt.sh successfully else you fail.

	    If you want to switch a channel, you need to modify the channel's name in locktv.sh and capmt.sh

	3) Play program
	    You need a software MPEG decoder. Mplayer or xine are known to work. We recommend that you had better use xine to play program.
	    We prepare script file for convenient usage.
 		$ ./play.sh

	Way 2:
	1) Scan the channel list
	    You may modify/add/delete the dvb-apps-997424a1799e/util/scan/dvb-c(dvb-t)/initial-tuning-data-file depend on your transponder settings. If you are lucky you can just use one of the supplied initial-tuning-data-file.
            We prepare script file for convenient usage. You can just modify the script for different initial-tuning-data-file.
		$ cd dvb-apps-997424a1799e/util/scan/
	    	$ scan -a 0 -d 0 -f 0 dvb-c(dvb-t)/initial-tuning-data-file >tw.list

	    You may modify the channel list(tw.list)'s channels name,because chinese word can't be read.

	    Copy the channel list(tw.list) to /root/.mplayer/channels.conf or /root/.xine/channels.conf.

	2) Run mplayer/xine to watch TV
		$ mplayer dvb://pst1(pst1 is the program name)
		or 
		$ xine dvb://pst1
 
