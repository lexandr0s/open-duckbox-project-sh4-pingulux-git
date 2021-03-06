/*
 * $Id: pzapit.cpp,v 1.50 2004/02/02 13:34:39 obi Exp $
 *
 * simple commandline client for zapit
 *
 * Copyright (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h> /* sleep */

#include <zapit/client/zapitclient.h>

#include <video_cs.h>

int usage (const char * basename)
{
	std::cout << "Usage:" << std::endl 
		<< basename << " <options>" << std::endl
		<< "   options:" << std::endl
		//test
		<< "\t-gi\t\tget current channel id: " << " -gi" << std::endl
		<< "\t-gsi\t\tget current service info: " << " -gsi" << std::endl
		//
		<< "\t-ra\t\tbouquet list: " << std::endl
		<< "\t-ra <bouquet-number>\t\tchannel list: " << std::endl
		<< "\t-ra <bouquet-number> <channel-number\t\tzap by number: " << std::endl
		<< "\t-ra -n <channel-name\t\tzap by name: " << std::endl
		<< "\t-dt <type>\t\tset diseqc type: " << std::endl
		<< "\t-dr <count>\t\tset diseqc repeats: " << std::endl
		<< "\t-ra\t\ttoggles radio mode" << std::endl
		<< "\t-re\t\tswitch record mode on/off: " << std::endl
		<< "\t--pe\t\tswitch pip mode on/off: " << std::endl
		<< "\t-p\t\tstart/stop playback: " << std::endl
		<< "\t-a <audio-number>\t\tchange audio pid: " << std::endl
		<< "\t-c\t\treload channels bouquets: " << std::endl
		<< "\t-sb\t\tsave bouquets: " << std::endl
		<< "\t-sh\t\tshow satellites: " << std::endl
		<< "\t-rz\t\trezap" << std::endl
		<< "\t-se <satmask> <diseqc order>\t\tselect satellites: " << std::endl
		<< "\t-st <mode> <0:nit, 1:fast>\t\tstart transponderscan: " << std::endl
		<< "\t-mute\t\tmute audio: " << std::endl
		<< "\t-unmute\t\tunmute audio: " << std::endl
		<< "\t-vol <0..64>\t\tset volume: " << std::endl
		<< "\t-rn\t\tregister neutrino as event client: " << std::endl
		<< "\t-kill\t\tshutdown zapit: " << std::endl
		<< "\t-esb\t\tenter standby: " << std::endl
		<< "\t-lsb\t\tleave standby: " << std::endl
		//test
		<< "\t-var\t\tget aspect ratio: " << std::endl
		<< "\t-var <aspectratio>\tset aspect ratio: " << std::endl
#if 0
		//FIXME howto read aspect mode back?
		<< "\t-vm43\t\tget 4:3 mode: " << std::endl
#endif
		<< "\t-vm43 <4:3mode>\tset 4:3 mode: " << std::endl
		//
        	<< "\t--1080\t\tswitch to hd 1080i mode: " << std::endl
        	<< "\t--pal\t\tswitch to pal mode: " << std::endl
        	<< "\t--720\t\tswitch to hd 720p mode: " << std::endl
		<< "\t-m <cmdtype> <addr> <cmd> <number of parameters> <parameter 1> <parameter 2>\t\tsend diseqc 1.2 motor command: " << std::endl;

	return -1;
}

int main (int argc, char** argv)
{
	int i;
	uint32_t  j;
	//uint32_t  k;
	uint64_t ii;

	int bouquet = -1;
	unsigned int channel = 0;
	unsigned int count;
	int diseqcRepeats = -1;
	int diseqcType = -1;
	uint64_t satmask = 0xFFFF;
	int audio = 0;
	int mute = -1;
	int volume = -1;
	int nvod = -1;
	//test
	int arat = -1;
	int m43 = -1;
	//
	const char * channelName = NULL;

	bool playback = false;
	bool recordmode = false;
	bool pipmode = false;
	bool radio = false;
	bool reload = false;
	bool register_neutrino = false;
	bool savebouquets = false;
	bool show_satellites = false;
        bool set_pal = false;
        int  set_hd = 0;
	bool scan = false;
	bool rezap = false;
	bool zapByName = false;
	bool killzapit = false;
	bool enterStandby = false;
	bool leaveStandby = false;
	bool sendMotorCommand = false;
	bool quiet = false;

	//test
	bool getchannel = false;
	bool aspectratio = false;
	bool mode43 = false;
	bool getserviceinfo = false;
	//
	uint8_t motorCmdType = 0;
	uint8_t motorCmd = 0;
	uint8_t motorNumParameters = 0;
	uint8_t motorParam1 = 0;
	uint8_t motorParam2 = 0;
	uint8_t motorAddr = 0;
	uint32_t  diseqc[5];
	unsigned int tmp;
	int scan_mode = 1;

	/* command line */
	for (i = 1; i < argc; i++)
	{
		if (!strncmp(argv[i], "-a", 2))
		{
			if (i < argc - 1)
			{
				sscanf(argv[++i], "%d", &audio);
				continue;
			}
		}
		else if (!strncmp(argv[i], "-dr", 3))
		{
			if (i < argc - 1)
			{
				sscanf(argv[++i], "%d", &diseqcRepeats);
				continue;
			}
		}
		else if (!strncmp(argv[i], "-dt", 3))
		{
			if (i < argc - 1)
			{
				sscanf(argv[++i], "%d", &diseqcType);
				continue;
			}
		}
		else if (!strncmp(argv[i], "-q", 2))
		{
			quiet = true;
			continue;
		}
		else if (!strncmp(argv[i], "-c", 2))
		{
			reload = true;
			continue;
		}
		else if (!strncmp(argv[i], "-esb", 4))
		{
			enterStandby = true;
			continue;
		}
		else if (!strncmp(argv[i], "-kill", 5))
		{
			killzapit = true;
			continue;
		}
		else if (!strncmp(argv[i], "-lsb", 4))
		{
			leaveStandby = true;
			continue;
		}
		else if (!strncmp(argv[i], "-rz", 3))
		{
			rezap = true;
			continue;
		}
		else if (!strncmp(argv[i], "-mute", 5))
		{
			mute = 1;
			continue;
		}
		else if (!strncmp(argv[i], "-m", 2))
		{
			if (i < argc - 6)
			{
				sscanf(argv[++i], "%x", &tmp);
				motorCmdType = tmp;
				sscanf(argv[++i], "%x", &tmp);
				motorAddr = tmp;
				sscanf(argv[++i], "%x", &tmp);
				motorCmd = tmp;
				sscanf(argv[++i], "%x", &tmp);
				motorNumParameters = tmp;
				sscanf(argv[++i], "%x", &tmp);
				motorParam1 = tmp;
				sscanf(argv[++i], "%x", &tmp);
				motorParam2 = tmp;
				printf("[pzapit] motor command = %x %x %x %x %x %x\n", motorCmdType, motorAddr, motorCmd, motorNumParameters, motorParam1, motorParam2);
				sendMotorCommand = true;
				continue;
			}
		}
		else if (!strncmp(argv[i], "-rn", 3))
		{
			register_neutrino = true;
			continue;
		}
		else if (!strncmp(argv[i], "-nvod", 5))
		{
			if (i < argc - 1)
			{
				sscanf(argv[++i], "%d", &nvod);
				continue;
			}
		}
		else if (!strncmp(argv[i], "-n", 2))
		{
			if (i < argc - 1)
			{
				zapByName = true;
				channelName = argv[++i];
				continue;
			}
		}
		else if (!strncmp(argv[i], "-p", 2))
		{
			playback = true;
			continue;
		}
		else if (!strncmp(argv[i], "-ra", 3))
		{
			radio = true;
			continue;
		}
		else if (!strncmp(argv[i], "-re", 3))
		{
			recordmode = true;
			continue;
		}
		else if (!strncmp(argv[i], "--pe", 4))
		{
			pipmode = true;
			continue;
		}
		else if (!strncmp(argv[i], "-var", 4))
		{
			aspectratio = true;
			if (i < argc - 1)
				sscanf(argv[++i], "%d", &arat);
			continue;
		}
		else if (!strncmp(argv[i], "-vm43", 5))
		{
			mode43 = true;
			if (i < argc - 1)
			{
				sscanf(argv[++i], "%d", &m43);
				continue;
			}
#if 0 
			//FIXME howto read aspect mode back?
			continue;
#endif
		}
		//
		else if (!strncmp(argv[i], "-sb", 3))
		{
			savebouquets = true;
			continue;
		}
		else if (!strncmp(argv[i], "-se", 3))
		{
			if (i < argc - 2)
			{
				sscanf(argv[++i], "%lld", &satmask);
				sscanf(argv[++i], "%d", &diseqc[0]);
/*
				diseqc[0] = strlen(argv[i+1]);
				for (i++, j = 0; j <= diseqc[0]; j++)
				{
					diseqc[j+1] = argv[i][j] - 48;
				}
*/
				continue;
			}
		}
		else if (!strncmp(argv[i], "-sh", 3))
		{
			show_satellites = true;
			continue;
		}
		else if (!strncmp(argv[i], "-st", 3))
		{
			scan = true;
			if (i < argc - 1)
			{
				sscanf(argv[++i], "%d", &scan_mode);
			}
			continue;
		}
                else if (!strncmp(argv[i], "--pal", 5))
                {
                        set_pal = true;
                        continue;
                }
        
                else if (!strncmp(argv[i], "--1080", 6))
                {
                        set_hd = VIDEO_STD_1080I50;
                        continue;
                }
                #if 0
                else if (!strncmp(argv[i], "--1083", 6))
                {
                        set_hd = 9;
                        continue;
                }
                else if (!strncmp(argv[i], "--1082", 6))
                {
                        set_hd = 10;
                        continue;
		}
		#endif
                else if (!strncmp(argv[i], "--720", 5))
                {
                        set_hd = VIDEO_STD_720P50;
                        continue;
		}
		else if (!strncmp(argv[i], "-unmute", 7))
		{
			mute = 0;
			continue;
		}
		else if (!strncmp(argv[i], "-vol", 4))
		{
			if (i < argc - 1)
			{
				sscanf(argv[++i], "%d", &volume);
				continue;
			}
		}
		//test
		else if (!strncmp(argv[i], "-gi", 3))
		{
			getchannel = true;
			continue;
		}
		else if(!strncmp(argv[i], "-gsi", 4))
		{
			getserviceinfo = true;
			continue;
		}
		//
		else if (i < argc - 1)
		{
			if ((sscanf(argv[i], "%d", &bouquet) > 0) && (sscanf(argv[++i], "%u", &channel) > 0))
				continue;
		}
		else if (sscanf(argv[i], "%d", &bouquet) > 0)
			continue;

		return usage(argv[0]);
	}

	/* create zapit client */
	CZapitClient zapit;

#if 0
	TP_params TP;
	TP.TP_id = 12345;
        TP.polarization = 1;
        TP.feparams.Frequency = 11727000;
        TP.feparams.u.qpsk.SymbolRate = 27500000;
        TP.feparams.u.qpsk.FEC_inner = (CodeRate) 3;

	zapit.scan_TP(TP);
	exit(0);
#endif

	/* send diseqc 1.2 motor command */
	if (sendMotorCommand)
	{
		zapit.sendMotorCommand(motorCmdType, motorAddr, motorCmd, motorNumParameters, motorParam1, motorParam2);
		return 0;
	}
	
	/* kill zapit*/
	if (killzapit)
	{
		zapit.shutdown();
		std::cout << "zapit shot down :)" << std::endl;
		return 0;
	}

	if (enterStandby)
	{
		zapit.setStandby(true);
		return 0;
	}

	if (leaveStandby)
	{
		zapit.setStandby(false);
		return 0;
	}

	/* audio mute */
	if (mute != -1)
	{
		std::cout << "mute/unmute" << std::endl;
		zapit.muteAudio(mute);
		return 0;
	}

	if (volume != -1)
	{
		std::cout << "set volume" << std::endl;
		zapit.setVolume(volume, volume);
		return 0;
	}
	
	if (rezap)
	{
		zapit.Rezap();
		return 0;
	}

	/* reload services */
	if (reload)
	{
		std::cout << "reloading channels" << std::endl;
		zapit.reinitChannels();
		return 0;
	}

	if (register_neutrino)
	{
#define NEUTRINO_UDS_NAME "/tmp/neutrino.sock"
		std::cout << "registering neutrino" << std::endl;
		for (int i = CZapitClient::FIRST_EVENT_MARKER; i < CZapitClient::LAST_EVENT_MARKER; i++)
			zapit.registerEvent(i, 222, NEUTRINO_UDS_NAME);
		return 0;
	}

	if (diseqcType != -1)
	{
		zapit.setDiseqcType((diseqc_t) diseqcType);

		if (diseqcRepeats == -1)
			return 0;
	}

	if (diseqcRepeats != -1)
	{
		zapit.setDiseqcRepeat(diseqcRepeats);
		return 0;
	}

	if (playback)
	{
		if (zapit.isPlayBackActive())
			zapit.stopPlayBack();
		else
			zapit.startPlayBack();

		if (!recordmode)
			return 0;
	}

	if (recordmode)
	{
		zapit.setRecordMode(!zapit.isRecordModeActive());
		return 0;
	}
	
	if (pipmode)
	{
		zapit.setPipMode(!zapit.isPipModeActive());
		return 0;
	}
	
	//aspectratio
	if (aspectratio)
	{
		if(arat >= 0)
			zapit.setAspectRatio(arat);
		else
		{
			zapit.getAspectRatio(&arat);
			printf("%d\n", arat);
		}
		return 0;
	}
	
	//mode34
	if (mode43)
	{
		if(m43 >= 0)
			zapit.setMode43(m43);
		else
		{
			zapit.getMode43(&m43);
			printf("%d\n",m43);
		}
		return 0;
	}

	if (savebouquets)
	{
		zapit.saveBouquets();
		return 0;
	}
 
	if (show_satellites)
	{
		std::vector<CZapitClient::responseGetSatelliteList> satelliteList;
		zapit.getScanSatelliteList(satelliteList);
		
		printf("    Name  pos  feindex\n");

		std::vector<CZapitClient::responseGetSatelliteList>::const_iterator rI;
		for ( ii = 0, rI = satelliteList.begin(); rI != satelliteList.end(); ii++, rI++)
		{
			printf("%lld : %s %d fe(%d)\n", ii, rI->satName, rI->satPosition, rI->feindex);
			//std::cout << (1 << ii) << ": " << rI->satName << std::endl;
		}

		return 0;
	}
	else if (satmask != 0xFFFF)
	{
		std::vector<CZapitClient::responseGetSatelliteList> satelliteList;
		zapit.getScanSatelliteList(satelliteList);

		std::vector<CZapitClient::commandSetScanSatelliteList> newSatelliteList;
		CZapitClient::commandSetScanSatelliteList item;

		for (j = 0; j < satelliteList.size(); j++)
		{
			if (satmask == j)
			{
				std::cout << "diseqc " << diseqc[0] << ": " << satelliteList[j].satName << std::endl;

				strcpy(item.satName, satelliteList[j].satName);
				item.position = diseqc[0];
				newSatelliteList.push_back(item);
				break;
			}
		}

		zapit.setScanSatelliteList(newSatelliteList);

		return 0;
	}

	/* transponderscan */
	if (scan)
	{
		unsigned int satellite;
		unsigned int processed_transponder;
		unsigned int transponder;
		unsigned int services;
		printf("[pzapit] Start scan, mode %d\n", scan_mode);
		zapit.startScan(scan_mode);

		while (zapit.isScanReady(satellite, processed_transponder, transponder, services) == false)
		{
			std::cout << "satellite: " << satellite << ", transponder: " << processed_transponder <<", of: " << transponder << ", services: " << services << std::endl;
			sleep(1);
		}

		return 0;
	}

        if (set_pal)
        {
                //zapit.stopPlayBack();
                zapit.setVideoSystem(VIDEO_STD_PAL);
                //zapit.startPlayBack();
                return 0;
        }
                 
        if (set_hd)
        {
                //zapit.stopPlayBack();
                zapit.setVideoSystem(set_hd);
                //zapit.startPlayBack();
                return 0;
        }

	//test
	if (getchannel)
	{
		t_channel_id channelid = zapit.getCurrentServiceID();
		printf("%llx (%s)\n", channelid, (zapit.getChannelName(channelid)).c_str());
		return 0;
	}
	
	//test
	if(getserviceinfo)
	{
		CZapitClient::CCurrentServiceInfo si;
		si = zapit.getCurrentServiceInfo();

		printf("frequency = %d.%d MHz\n", si.tsfrequency/1000, si.tsfrequency%1000);

		#if 0
		if (si.polarisation != 2) /* only satellite has polarisation */
		{
			printf(" (%c)\n", (si.polarisation == HORIZONTAL) ? 'h' : 'v');
			//satellite
			printf("diseqc = %d\n", si.diseqc);
		}
		else
		{
			printf("\n");
		}
		#endif
		
		printf("onid = 0x%04x\n", si.onid);
		printf("sid = 0x%04x\n", si.sid);
		printf("tsid = 0x%04x\n", si.tsid);
		printf("pmtpid = 0x%04x\n", si.pmtpid);
		printf("vpid = 0x%04x\n", si.vpid);
		printf("apid = 0x%04x\n", si.apid);
		//printf("spid = 0x%04x\n", si.spid);
		//printf("spage = 0x%04x\n", si.spage);
		printf("pcrpid = 0x%04x\n", si.pcrpid);
		printf("vtxtpid = 0x%04x\n", si.vtxtpid);
		
		printf("fe-index = %d\n", si.FeIndex);
		return 0;
	}
	//


	/* choose source mode */
	zapit.setMode(radio ? CZapitClient::MODE_RADIO : CZapitClient::MODE_TV);

	/* set audio channel */
	if (audio)
	{
		zapit.setAudioChannel(audio - 1);
		return 0;
	}

	if (nvod != -1)
	{
		zapit.zaptoNvodSubService(nvod);
		return 0;
	}
	else
	{
		std::vector<CZapitClient::responseGetBouquetChannels> channels;

		if (zapByName)
		{
			zapit.getChannels(channels);

			std::vector<CZapitClient::responseGetBouquetChannels>::const_iterator ch_resp;
			for (ch_resp = channels.begin(), channel = 1; ch_resp != channels.end(); ch_resp++, channel++)
			{
				if (!strcasecmp(ch_resp->name, channelName))
				{
					std::cout << "found channel number: " << channel << std::endl;
					goto channel_found;
				}
			}
			
			std::cout << "channel not found." << std::endl;
			return 0;
		}
		else /* zap by bouquet number and channel number */
		{
			/* read channel list */
			if (bouquet != -1)
				zapit.getBouquetChannels(bouquet - 1, channels, CZapitClient::MODE_CURRENT, true);

			/* display bouquet list */
			else
			{
				std::vector<CZapitClient::responseGetBouquets> bouquets;
				std::vector<CZapitClient::responseGetBouquets>::const_iterator b_resp;

				zapit.getBouquets(bouquets, false);

				for (b_resp = bouquets.begin(); b_resp != bouquets.end(); b_resp++)
					std::cout << (b_resp->bouquet_nr + 1) << ": " << b_resp->name << std::endl;

				return 0;
			}

			/* display channel list */
			if (!channel)
			{
				std::vector<CZapitClient::responseGetBouquetChannels>::const_iterator ch_resp;
				for (ch_resp = channels.begin(), channel = 1; ch_resp != channels.end(); ch_resp++, channel++)
					//std::cout << channel << ": " << ch_resp->name << ": " << ch_resp->channel_id<< std::endl;
					printf("%3d: %s (%04x)\n", channel, ch_resp->name, (short) (ch_resp->channel_id &0xFFFF));
				return 0;
			}
		}

		/* zap */
		if (channel > channels.size())
		{
			std::cout << "Only " << channels.size() << " channels in bouquet " << bouquet << std::endl;
			return 0;
		}

	channel_found:
		zapit.zapTo(channels[channel-1].nr);
		std::cout << "[pzapit] zapped to " << channels[channel-1].name << std::endl;
	}

	
	if(!quiet)
	{	
		CZapitClient::responseGetPIDs pids;
		zapit.getPIDS(pids);

		if (pids.PIDs.vpid)
			std::cout << "[pzapit] video: 0x" << std::hex << pids.PIDs.vpid << std::endl;

		if (pids.PIDs.vtxtpid)
			std::cout << "[pzapit] teletext: 0x" << std::hex << pids.PIDs.vtxtpid << std::endl;

		if (pids.PIDs.pcrpid)
			std::cout << "[pzapit] pcr: 0x" << std::hex << pids.PIDs.pcrpid << std::endl;

		for (count = 0; count < pids.APIDs.size(); count++)
		{
			std::cout << "[pzapit] audio " << std::dec << count + 1 << ": 0x" << std::hex << pids.APIDs[count].pid << " (" << pids.APIDs[count].desc;
			if (pids.APIDs[count].is_ac3)
				std::cout << ", ac3";
			std::cout << ")" << std::endl;
		}
	}

	return 0;
}
