/*
 * $Id: pmt.cpp,v 1.40 2004/04/04 20:46:17 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
 * (C) 2002 by Frank Bormann <happydude@berlios.de>
 * (C) 2007-2012 Stefan Seyfried
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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* zapit */
#include <zapit/settings.h>
#include <zapit/descriptors.h>
#include <zapit/debug.h>
#include <zapit/pmt.h>
#include <zapit/scan.h>
#include <sectionsd/edvbstring.h>
#include <dmx.h>

#include <ca_cs.h>
//#include <linux/dvb/dmx.h>

#define PMT_SIZE 1024
#define RECORD_MODE 0x4

/*
 * Stream types
 * ------------
 * 0x01 ISO/IEC 11172 Video
 * 0x02 ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream
 * 0x03 ISO/IEC 11172 Audio
 * 0x04 ISO/IEC 13818-3 Audio
 * 0x05 ITU-T Rec. H.222.0 | ISO/IEC 13818-1 private_sections, e.g. MHP Application signalling stream
 * 0x06 ITU-T Rec. H.222.0 | ISO/IEC 13818-1 PES packets containing private data, e.g. teletext or ac3
 * 0x0b ISO/IEC 13818-6 type B
 * 0x81 User Private (MTV)
 * 0x90 User Private (Premiere Mail, BD_DVB)
 * 0xc0 User Private (Canal+)
 * 0xc1 User Private (Canal+)
 * 0xc6 User Private (Canal+)
 */

unsigned short parse_ES_info(const unsigned char * const buffer, CZapitChannel * const channel, CCaPmt * const caPmt)
{
	unsigned short ES_info_length;
	unsigned short pos;
	unsigned char descriptor_tag;
	unsigned char descriptor_length;
	unsigned char i;

	bool isAc3 = false;
	bool isDts = false;
	bool isAac = false;
	bool descramble = false;
	std::string description = "";
	unsigned char componentTag = 0xFF;

	/* elementary stream info for ca pmt */
	CEsInfo *esInfo = new CEsInfo();

	esInfo->stream_type = buffer[0];
	esInfo->reserved1 = buffer[1] >> 5;
	esInfo->elementary_PID = ((buffer[1] & 0x1F) << 8) | buffer[2];
	esInfo->reserved2 = buffer[3] >> 4;

	ES_info_length = ((buffer[3] & 0x0F) << 8) | buffer[4];

	for (pos = 5; pos < ES_info_length + 5; pos += descriptor_length + 2) {
		descriptor_tag = buffer[pos];
		descriptor_length = buffer[pos + 1];
		unsigned char fieldCount = descriptor_length / 5;

		switch (descriptor_tag) {
			case 0x02:
				video_stream_descriptor(buffer + pos);
				break;

			case 0x03:
				audio_stream_descriptor(buffer + pos);
				break;

			case 0x05:
				if (descriptor_length >= 3)
					if (!strncmp((const char*)&buffer[pos + 2], "DTS", 3))
						isDts = true;
				break;

			case 0x09:
				esInfo->addCaDescriptor(buffer + pos);
				descramble = true;
				break;

			case 0x0A: /* ISO_639_language_descriptor */
#if 0
printf("descr 0x0A: %02X %02X %02X\n", buffer[pos+2], buffer[pos+3], buffer[pos+4]);
#endif
				/* FIXME cyfra+ radio -> 41 20 31 ?? */
				if (description != "" && buffer[pos + 3] == ' ') {
					description += buffer[pos + 3];
					description += buffer[pos + 4];
				} else {
				for (i = 0; i < 3; i++)
					description += tolower(buffer[pos + i + 2]);
				}
				break;

			case 0x13: /* Defined in ISO/IEC 13818-6 */
				break;

			case 0x0E:
				Maximum_bitrate_descriptor(buffer + pos);
				break;

			case 0x0F:
				Private_data_indicator_descriptor(buffer + pos);
				break;

			case 0x11:
				STD_descriptor(buffer + pos);
				break;

			case 0x45:
				VBI_data_descriptor(buffer + pos);
				break;

			case 0x52: /* stream_identifier_descriptor */
				componentTag = buffer[pos + 2];
				break;

			case 0x56: /* teletext descriptor */
				char tmp_Lang[4];
				//printf("[pmt] teletext pid %x: %s\n", esInfo->elementary_PID, tmp_Lang);
				printf("[pmt] teletext pid %x\n", esInfo->elementary_PID);
				for (unsigned char fIdx = 0; fIdx < fieldCount; fIdx++) {
					memmove(tmp_Lang, &buffer[pos + 5*fIdx + 2], 3);
					tmp_Lang[3] = '\0';
					unsigned char teletext_type=buffer[pos + 5*fIdx + 5]>> 3;
					unsigned char teletext_magazine_number = buffer[pos + 5*fIdx + 5] & 7;
					unsigned char teletext_page_number=buffer[pos + 5*fIdx + 6];
printf("[pmt] teletext type %d mag %d page %d lang %s\n", teletext_type, teletext_magazine_number, teletext_page_number, tmp_Lang);
					if (teletext_type==0x01)
						channel->setTeletextLang(tmp_Lang);
					if (teletext_type==0x02){
						channel->addTTXSubtitle(esInfo->elementary_PID,tmp_Lang,teletext_magazine_number,teletext_page_number);
					} else {
						if (teletext_type==0x05){
							channel->addTTXSubtitle(esInfo->elementary_PID,tmp_Lang,teletext_magazine_number,teletext_page_number,true);
						}
					}
				}

				channel->setTeletextPid(esInfo->elementary_PID);
				descramble = true;//FIXME ORF HD scramble txt ?
				break;

			case 0x59:
				if (esInfo->stream_type==0x06) {
					unsigned char fieldCount1=descriptor_length/8;
					for (unsigned char fIdx=0;fIdx<fieldCount1;fIdx++){
						char tmpLang[4];
						const unsigned char *p = buffer + pos + 8 * fIdx;
						memmove(tmpLang, p + 2, 3);
						tmpLang[3] = '\0';
						unsigned char subtitling_type = p[5];
						unsigned short composition_page_id = (p[6] << 8 | p[7]);
						unsigned short ancillary_page_id = (p[8] << 8 | p[9]);
						channel->addDVBSubtitle(esInfo->elementary_PID,tmpLang,subtitling_type,composition_page_id,ancillary_page_id);
					}
					descramble = true;//FIXME MGM / 10E scrambling subtitles ?
				}

				subtitling_descriptor(buffer + pos);
				break;

			case 0x5F:
				private_data_specifier_descriptor(buffer + pos);
				break;

			case 0x66:
				data_broadcast_id_descriptor(buffer + pos);
				break;

			case 0x6A: /* AC3 descriptor */
				isAc3 = true;
				break;

			case 0x6F: /* unknown, Astra 19.2E */
				break;

			case 0x7B:
				isDts = true;
				break;

			case 0x7C: //FIXME AAC
				isAac = true;
				break;

			case 0x90: /* unknown, Astra 19.2E */
				break;

			case 0xB1: /* unknown, Astra 19.2E */
				break;

			case 0xC0: /* unknown, Astra 19.2E */
				break;

			case 0xC1: /* unknown, Astra 19.2E */
				break;

			case 0xC2: /* User Private descriptor - Canal+ */
#if 0
				DBG("0xC2 dump:");
				for (i = 0; i < descriptor_length; i++) {
					printf("%c", buffer[pos + 2 + i]);
					if (((i+1) % 8) == 0)
						printf("\n");
				}
#endif
				break;

			case 0xC5: /* User Private descriptor - Canal+ Radio */
				//description = convertDVBUTF8((const char*)&buffer[pos+3], 24, 2, 1);
				description = convertDVBUTF8((const char*)&buffer[pos+3], 24, 2, channel->getTransportStreamId() << 16 | channel->getOriginalNetworkId());
#if 0
printf("descr 0xC5\n");
				for (i = 0; i < 24; i++) {
printf("%02X ", buffer[pos + i]);
					//description += buffer[pos + i + 3];
				}
printf("\n");
printf("[pmt] name %s\n", description.c_str());
#endif
				break;

			case 0xC6: /* unknown, Astra 19.2E */
				break;

			case 0xFD: /* unknown, Astra 19.2E */
				break;

			case 0xFE: /* unknown, Astra 19.2E */
				break;

			default:
				DBG("descriptor_tag: %02x\n", descriptor_tag);
				break;
		}
	}

	switch (esInfo->stream_type) {
		case 0x01:
		case 0x02:
		case 0x1b: // AVC Video Stream (MPEG4 H264)
			channel->setVideoPid(esInfo->elementary_PID);
			descramble = true;
			channel->type = (esInfo->stream_type == 0x1b); //FIXME
			printf("[pmt] vpid %x stream %d type %d\n", esInfo->elementary_PID, esInfo->stream_type, channel->type);
			break;

		case 0x03:
		case 0x04:
			if (description == "")
				description = " ";
			if(CServiceScan::getInstance()->Scanning()) {
				if(channel->getPreAudioPid() == 0)
					channel->setAudioPid(esInfo->elementary_PID);
			} else
				channel->addAudioChannel(esInfo->elementary_PID, CZapitAudioChannel::MPEG, description, componentTag);
			descramble = true;
			printf("[pmt] apid %x: %s\n", esInfo->elementary_PID, description.c_str());
			break;

		case 0x05:// private section
			{
				int tmp=0;
				// Houdini: shameless stolen from enigma dvbservices.cpp
				for (pos = 5; pos < ES_info_length + 5; pos += descriptor_length + 2) {
					descriptor_tag = buffer[pos];
					descriptor_length = buffer[pos + 1];

					switch (descriptor_tag) {
						case 0x5F: //DESCR_PRIV_DATA_SPEC:
							if ( ((buffer[pos + 2]<<24) | (buffer[pos + 3]<<16) | (buffer[pos + 4]<<8) | (buffer[pos + 5])) == 190 )
								tmp |= 1;
							break;
						case 0x90:
							{
								if ( descriptor_length == 4 && !buffer[pos + 2] && !buffer[pos + 3] && buffer[pos + 4] == 0xFF && buffer[pos + 5] == 0xFF )
									tmp |= 2;
							}
							//break;??
						default:
							break;
					}
				}
				if ( tmp == 3 ) {
					channel->setPrivatePid(esInfo->elementary_PID);
					DBG("channel->setPrivatePid(%x)\n", esInfo->elementary_PID);
				}
				descramble = true;
				break;
			}
		case 0x81:
			esInfo->stream_type = 0x6;
			if (description == "")
				description = "Unknown";
			description += " (AC3)";
			isAc3 = true;
			descramble = true;
			if(!CServiceScan::getInstance()->Scanning())
				channel->addAudioChannel(esInfo->elementary_PID, CZapitAudioChannel::AC3, description, componentTag);
			break;
		case 0x06:
			if ((isAc3) || (isDts) || (isAac)) {
				if (description == "") {
					description = esInfo->elementary_PID;
					if (isAc3)
						description += " (AC3)";
					else if (isDts)
						description += " (DTS)";
					else if (isAac)
						description += " (AAC)";
				}

				if(!CServiceScan::getInstance()->Scanning()) {
					CZapitAudioChannel::ZapitAudioChannelType Type;
					if (isAc3)
						Type = CZapitAudioChannel::AC3;
					else if (isDts)
						Type = CZapitAudioChannel::DTS;
					else if (isAac)
						Type = CZapitAudioChannel::AAC;
					else
						Type = CZapitAudioChannel::UNKNOWN;
					channel->addAudioChannel(esInfo->elementary_PID, Type, description, componentTag);
				}

				descramble = true;
			}
			break;

		case 0x0F: // AAC ADTS
		case 0x11: // AAC LATM
			if (description == "")
				description	= esInfo->elementary_PID;
			description	+= " (AAC)";
			isAac		= true;
			descramble	= true;
			if(!CServiceScan::getInstance()->Scanning())
				channel->addAudioChannel(esInfo->elementary_PID, CZapitAudioChannel::AAC, description, componentTag);
			break;
		case 0x0B:
			break;

		case 0x90:
			break;

		case 0x93:
			break;

		case 0xC0:
			break;

		case 0xC1:
			break;

		case 0xC6:
			break;

		default:
			DBG("stream_type: %02x\n", esInfo->stream_type);
			break;
	}

	if (descramble)
		caPmt->es_info.insert(caPmt->es_info.end(), esInfo);
	else
		delete esInfo;

	return ES_info_length;
}

int curpmtpid,curservice_id;
int pmt_caids[4][11] = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};

int parse_pmt(CZapitChannel * const channel)
{
	int pmtlen;
	int ia, dpmtlen, pos;
	unsigned char descriptor_length=0;

	unsigned char buffer[PMT_SIZE];

	/* current position in buffer */
	unsigned short i,j;
	for(j=0;j<4;j++){
		for(i=0;i<11;i++)
			pmt_caids[j][i] = 0;
	}
	/* length of elementary stream description */
	unsigned short ES_info_length;

	/* TS_program_map_section elements */
	unsigned short section_length;
	unsigned short program_info_length;

	unsigned char filter[DMX_FILTER_SIZE];
	unsigned char mask[DMX_FILTER_SIZE];

	printf("[zapit] parsing pmt pid 0x%X\n", channel->getPmtPid());

	if (channel->getPmtPid() == 0){
		return -1;
	}

	cDemux * dmx = new cDemux();
	dmx->Open(DMX_PSI_CHANNEL);

	memset(filter, 0x00, DMX_FILTER_SIZE);
	memset(mask, 0x00, DMX_FILTER_SIZE);

	filter[0] = 0x02;	/* table_id */
	filter[1] = channel->getServiceId() >> 8;
	filter[2] = channel->getServiceId();
	filter[3] = 0x01;	/* current_next_indicator */
	filter[4] = 0x00;	/* section_number */
	mask[0] = 0xFF;
	mask[1] = 0xFF;
	mask[2] = 0xFF;
	mask[3] = 0x01;
	mask[4] = 0xFF;

	if ((dmx->sectionFilter(channel->getPmtPid(), filter, mask, 5) < 0) || (dmx->Read(buffer, PMT_SIZE) < 0)) {
		delete dmx;
		return -1;
	}
	delete dmx;

	curservice_id = channel->getServiceId();
	curpmtpid = channel->getPmtPid();
	pmtlen= ((buffer[1]&0xf)<<8) + buffer[2] +3;

	dpmtlen=0;
	pos=10;
	short int ci0 = 0, ci1 = 0, ci2 = 0, ci3 = 0, ci4 = 0, ci5 = 0, ci6 = 0, ci7 = 0, ci8 = 0, ci9 = 0, ci10 = 0;
	if(!CServiceScan::getInstance()->Scanning()) {
		while(pos+2<pmtlen) {
			dpmtlen=((buffer[pos] & 0x0f) << 8) | buffer[pos+1];
			for ( ia=pos+2;ia<(dpmtlen+pos+2);ia +=descriptor_length+2 ) {
				descriptor_length = buffer[ia+1];
				if ( ia < pmtlen - 4 )
					if(buffer[ia]==0x09 && buffer[ia+1]>0) {
						switch(buffer[ia+2]) {
							case 0x06: pmt_caids[ci0][0] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci0 < 3) ci0++;
								   break;
							case 0x17: pmt_caids[ci1][1] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci1 < 3) ci1++;
								   break;
							case 0x01: pmt_caids[ci2][2] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci2 < 3) ci2++;
								   break;
							case 0x05: pmt_caids[ci3][3] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci3 < 3) ci3++;
								   break;
							case 0x18: pmt_caids[ci4][4] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci4 < 3) ci4++;
								   break;
							case 0x0B: pmt_caids[ci5][5] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci5 < 3) ci5++;
								   break;
							case 0x0D: pmt_caids[ci6][6] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci6 < 3) ci6++;
								   break;
							case 0x09: pmt_caids[ci7][7] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci7 < 3) ci7++;
								   break;
							case 0x26: pmt_caids[ci8][8] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci8 < 3) ci8++;
								   break;
							case 0x4a: pmt_caids[ci9][9] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci9 < 3) ci9++;
								   break;
							case 0x0E: pmt_caids[ci10][10] = (buffer[ia+2] & 0x1f) << 8 | buffer[ia+3];
								   if(ci10 < 3) ci10++;
								   break;
						} //switch
					} // if
			} // for
			pos+=dpmtlen+5;
		} // while
#if 0
		fout=fopen("/tmp/caids.info","w");
		if(fout) {
			fprintf(fout, "%d %d %d %d %d %d %d %d %d %d\n", caids[0],caids[1],caids[2],caids[3],caids[4],caids[5], caids[6], caids[7], caids[8], caids[9]);
			fclose(fout);
		}

#endif
	} /* if !CServiceScan::getInstance()->Scanning() */
	CCaPmt *caPmt = new CCaPmt();

	/* ca pmt */
	caPmt->program_number = (buffer[3] << 8) + buffer[4];
	caPmt->reserved1 = buffer[5] >> 6;
	caPmt->version_number = (buffer[5] >> 1) & 0x1F;
	caPmt->current_next_indicator = buffer[5] & 0x01;
	caPmt->reserved2 = buffer[10] >> 4;

	printf("[pmt] pcr pid: old 0x%x new 0x%x\n", channel->getPcrPid(), ((buffer[8] & 0x1F) << 8) + buffer[9]);

	if(channel->getCaPmt() != 0) {
		if(channel->getCaPmt()->version_number != caPmt->version_number)
			channel->resetPids();
	}
	/* pmt */
	section_length = ((buffer[1] & 0x0F) << 8) + buffer[2];

	//if(!channel->getPidsFlag())
		channel->setPcrPid(((buffer[8] & 0x1F) << 8) + buffer[9]);

	program_info_length = ((buffer[10] & 0x0F) << 8) | buffer[11];

	if (program_info_length)
		for (i = 12; i < 12 + program_info_length; i += buffer[i + 1] + 2)
			switch (buffer[i]) {
				case 0x09:
					caPmt->addCaDescriptor(buffer + i);
					break;
				default:
					DBG("decriptor_tag: %02x\n", buffer[i]);
					break;
			}

	/* pmt */
	for (i = 12 + program_info_length; i < section_length - 1; i += ES_info_length + 5)
		ES_info_length = parse_ES_info(buffer + i, channel, caPmt);

	if(CServiceScan::getInstance()->Scanning()) {
		channel->setCaPmt(NULL);
		channel->setRawPmt(NULL);
		delete caPmt;
	} else {
		channel->setCaPmt(caPmt);
		unsigned char * p = new unsigned char[pmtlen];
		memmove(p, buffer, pmtlen);
		channel->setRawPmt(p, pmtlen);
	}
#if 0
	//Quick&Dirty Hack to support Premiere's EPG not only on the portal but on the subchannels as well
	if (channel->getOriginalNetworkId() == 0x0085) {
		if (channel->getTransportStreamId() ==0x0003)
			channel->setPrivatePid(0x0b12);
		if (channel->getTransportStreamId() ==0x0004)
			channel->setPrivatePid(0x0b11);
	}
#endif

#ifdef MARTII
#define PID_CONFIG_FILE CONFIGDIR "/zapit/supplemental_pids.conf"
	// This file is maintained manually and is currently used for adding TTX subtitle pids on ARD/ZDF only. --martii
	//
	// channel_id descriptor_tag teletext_type type-specific-data
	// channel_id           0x56             1 language
	// channel_id           0x56             2 pid magazine page
	// channel_id           0x56             5 pid magazine page

	FILE  *SUPPIDS = fopen(PID_CONFIG_FILE, "r");
	if (SUPPIDS) {
		t_channel_id curChan =  channel->getChannelID();
		char buf[128];
		char tmp_Lang[4];
		memset(tmp_Lang, 0, sizeof(tmp_Lang));
		while (fgets(buf, sizeof(buf), SUPPIDS)) {
			t_channel_id chan;
			unsigned int desc, ty;
			char typespecific[sizeof(buf)];
			if ((buf[0] == '#') || !buf[0])
				continue;
			if (4 == sscanf(buf, "%llx %x %d %[^\n]", &chan, &desc, &ty, typespecific)) {
				if (chan == curChan) {
					switch(desc) {
						case 0x56: {
							switch(ty) {
								case 1:
									strncpy(tmp_Lang, typespecific, 3);
									break;
								case 2:
								case 5: {
									unsigned int elementary_PID;
									unsigned int teletext_magazine_number;
									unsigned int teletext_page_number;
									if (3 == sscanf(typespecific, "%x %x %x", &elementary_PID,
										&teletext_magazine_number, &teletext_page_number))
										channel->addTTXSubtitle(elementary_PID, tmp_Lang,
											(u_char) teletext_magazine_number,
											(u_char) teletext_page_number, (ty == 5));
										break;
								}
							}
							break;

						}
					}
				}
			}
		}
		fclose(SUPPIDS);
	}
#endif

	channel->setPidsFlag();

	return 0;
}

int scan_parse_pmt(int pmtpid, int service_id )
{
	if((pmtpid < 1 ) || (curpmtpid == pmtpid && service_id != curservice_id))
		return -1;
	if(curpmtpid == pmtpid && service_id == curservice_id){
		 for(int i=0;i<11;i++){
			if(pmt_caids[0][i] > 0)
			  return 1;
		 }
		 return 0;
	}

	int pmtlen;
	int ia, dpmtlen, pos;
	unsigned char descriptor_length=0;
	const short pmt_size = 1024;

	unsigned char buffer[pmt_size];

	unsigned char filter[DMX_FILTER_SIZE];
	unsigned char mask[DMX_FILTER_SIZE];

	cDemux * dmx = new cDemux();
	dmx->Open(DMX_PSI_CHANNEL);

	memset(filter, 0x00, DMX_FILTER_SIZE);
	memset(mask, 0x00, DMX_FILTER_SIZE);

	filter[0] = 0x02;	/* table_id */
	filter[1] = service_id >> 8;
	filter[2] = service_id;
	filter[3] = 0x01;	/* current_next_indicator */
	filter[4] = 0x00;	/* section_number */
	mask[0] = 0xFF;
	mask[1] = 0xFF;
	mask[2] = 0xFF;
	mask[3] = 0x01;
	mask[4] = 0xFF;

	if ((dmx->sectionFilter(pmtpid, filter, mask, 5) < 0) || (dmx->Read(buffer, pmt_size) < 0)) {
		delete dmx;
		return -1;
	}
	delete dmx;
	pmtlen= ((buffer[1]&0xf)<<8) + buffer[2] +3;

	dpmtlen=0;
	pos=10;
	if(service_id == ((buffer[3] << 8) | buffer[4]) ){
		while(pos+2<pmtlen) {
			dpmtlen=((buffer[pos] & 0x0f) << 8) | buffer[pos+1];
			for ( ia=pos+2;ia<(dpmtlen+pos+2);ia +=descriptor_length+2 ) {
				descriptor_length = buffer[ia+1];
				if ( ia < pmtlen - 4 )
					if(buffer[ia]==0x09 && buffer[ia+1]>0) {
						switch(buffer[ia+2]) {
							case 0x06: 
							case 0x17: 
							case 0x01: 
							case 0x05: 
							case 0x18: 
							case 0x0B: 
							case 0x0D: 
							case 0x09: 
							case 0x26: 
							case 0x4a: 
							case 0x0E: 
					 		return 1;
						} //switch
					} // if
			} // for
			pos+=dpmtlen+5;
		} // while
		return 0;
	}
	return -1;

}
#ifndef DMX_SET_NEGFILTER_MASK
        #define DMX_SET_NEGFILTER_MASK   _IOW('o',48,uint8_t *)
#endif

cDemux * pmtDemux;

int pmt_set_update_filter(CZapitChannel * const channel, int * fd)
{
	unsigned char filter[DMX_FILTER_SIZE];
	unsigned char mask[DMX_FILTER_SIZE];
	unsigned char mode[DMX_FILTER_SIZE];

	if(pmtDemux == NULL) {
		pmtDemux = new cDemux(0);
		pmtDemux->Open(DMX_PSI_CHANNEL);
	}

	if (channel->getPmtPid() == 0)
		return -1;

	memset(filter, 0x00, DMX_FILTER_SIZE);
	memset(mask, 0x00, DMX_FILTER_SIZE);
	memset(mode, 0x00, DMX_FILTER_SIZE);

	filter[0] = 0x02;	/* table_id */
	filter[1] = channel->getServiceId() >> 8;
	filter[2] = channel->getServiceId();
	filter[4] = 0x00;	/* section_number */

	mask[0] = 0xFF;
	mask[1] = 0xFF;
	mask[2] = 0xFF;
	mask[4] = 0xFF;

	printf("[pmt] set update filter, sid 0x%x pid 0x%x version %x\n", channel->getServiceId(), channel->getPmtPid(), channel->getCaPmt()->version_number);
#if HAVE_COOL_HARDWARE
	filter[3] = (((channel->getCaPmt()->version_number + 1) & 0x01) << 1) | 0x01;
	mask[3] = (0x01 << 1) | 0x01;
	pmtDemux->sectionFilter(channel->getPmtPid(), filter, mask, 5);
#else
	filter[3] = (channel->getCaPmt()->version_number << 1) | 0x01;
	mask[3] = (0x1F << 1) | 0x01;
	mode[3] = 0x1F << 1;
	pmtDemux->sectionFilter(channel->getPmtPid(), filter, mask, 5, 0, mode);
#endif

	*fd = 1;
	return 0;
}

int pmt_stop_update_filter(int * fd)
{
	printf("[pmt] stop update filter\n");
#if HAVE_TRIPLEDRAGON
	if (pmtDemux)
		delete pmtDemux;
	/* apparently a close/reopen is needed on TD... */
	pmtDemux = NULL;
#else
	if(pmtDemux)
		pmtDemux->Stop();
#endif

	*fd = -1;
        return 0;
}
