/*
	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>

#include <linux/dvb/audio.h>

#include <linux/soundcard.h>

#include "audio_cs.h"

#include <system/debug.h>


static const char * FILENAME = "[audio_cs.cpp]";

//ugly most functions are done in proc
cAudio * audioDecoder = NULL;


cAudio::cAudio()
{  
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);

	Muted = false;
	
	audio_fd = -1;
#if defined (ENABLE_DSP)
	clipfd = -1;
#endif	
	
#ifndef __sh__
	StreamType = AUDIO_STREAMTYPE_MPEG;
#endif

	volume = 0;
	
	m_pcm_delay = -1,
	m_ac3_delay = -1;
}

cAudio::~cAudio(void)
{  
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);	

	Close();
}

bool cAudio::Open(int num)
{
	audio_num = 0; //always 0
	
	char devname[32];

	// Open audio device
	sprintf(devname, "/dev/dvb/adapter0/audio%d", audio_num);

	audio_fd = open(devname, O_RDWR);

	if(audio_fd > 0)
	{
		dprintf(DEBUG_INFO, "cAudio::Open %s\n", devname);
		
		return true;
	}	

	return false;
}

bool cAudio::Close()
{  
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);	

	if (audio_fd >= 0)
		close(audio_fd);
	audio_fd = -1;
	
#if defined (ENABLE_DSP)
	if (clipfd >= 0)
		close(clipfd);
	clipfd = -1;
#endif	

	return true;
}

// shut up
int cAudio::SetMute(int enable)
{  
	dprintf(DEBUG_INFO, "%s:%s (%d)\n", FILENAME, __FUNCTION__, enable);	
	
	Muted = enable?true:false;
	
#ifdef __sh__	
	char sMuted[4];
	sprintf(sMuted, "%d", Muted);

	int fd = open("/proc/stb/audio/j1_mute", O_RDWR);
	write(fd, sMuted, strlen(sMuted));
	close(fd);
#else
	if( ioctl(audio_fd, AUDIO_SET_MUTE, enable) < 0 )
		perror("AUDIO_SET_MUTE");
#endif

	return 0;
}

/* volume, min = 0, max = 100 */
/* e2 sets 0 to 63 */
int cAudio::setVolume(unsigned int left, unsigned int right)
{ 
	dprintf(DEBUG_INFO, "%s:%s volume: %d\n", FILENAME, __FUNCTION__, left);
	
#ifdef __sh__	
	volume = left;
	
	if (volume < 0)
		volume = 0;
	else if (volume > 100)
		volume = 100;
	
	char sVolume[4];
	sprintf(sVolume, "%d", (int)(63-(int)(volume * 0.63)));

	int fd = open("/proc/stb/avs/0/volume", O_RDWR);
	write(fd, sVolume, strlen(sVolume));
	close(fd);
#else
	volume = left;
	
	// convert to -1dB steps
	left = 63 - left * 63 / 100;
	right = 63 - right * 63 / 100;
	//now range is 63..0, where 0 is loudest
	
	audio_mixer_t mixer;

	mixer.volume_left = left;
	mixer.volume_right = right;
	
	if( ioctl(audio_fd, AUDIO_SET_MIXER, &mixer) < 0 )
		perror("AUDIO_SET_MIXER");
#endif
	
	return 0;
}

/* start audio */
int cAudio::Start(void)
{ 
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);

	if (ioctl(audio_fd, AUDIO_PLAY) < 0)
		perror("AUDIO_PLAY");

	return 0;
}

int cAudio::Stop(void)
{ 
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);	
	
	if (ioctl(audio_fd, AUDIO_STOP) < 0)
		perror("AUDIO_STOP");

	return 0;
}

bool cAudio::Pause()
{  
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);	

	if (ioctl(audio_fd, AUDIO_PAUSE, 1) < 0)
		perror("AUDIO_PAUSE");

	return true;
}

bool cAudio::Resume()
{  
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);	

	if (ioctl(audio_fd, AUDIO_CONTINUE) < 0)
		perror("AUDIO_CONTINUE");
	
	return true;
}

/* set streamtype */
/*
 * List of possible container types - used to select demux..  If stream_source is VIDEO_SOURCE_DEMUX
 * then default is TRANSPORT, if stream_source is VIDEO_SOURCE_MEMORY then default is PES
 */
#ifdef __sh__
void cAudio::SetStreamType(stream_type_t type)
{ 
	const char * aSTREAMTYPE[] = {
		"STREAM_TYPE_NONE",     /* Deprecated */
		"STREAM_TYPE_TRANSPORT",/* Use latest PTI driver so it can be Deprecated */
		"STREAM_TYPE_PES",
		"STREAM_TYPE_ES",       /* Deprecated */
		"STREAM_TYPE_PROGRAM",  /* Deprecated */
		"STREAM_TYPE_SYSTEM",   /* Deprecated */
		"STREAM_TYPE_SPU",      /* Deprecated */
		"STREAM_TYPE_NAVI",     /* Deprecated */
		"STREAM_TYPE_CSS",      /* Deprecated */
		"STREAM_TYPE_AVI",      /* Deprecated */
		"STREAM_TYPE_MP3",      /* Deprecated */
		"STREAM_TYPE_H264",     /* Deprecated */
		"STREAM_TYPE_ASF",      /* Needs work so it can be deprecated */
		"STREAM_TYPE_MP4",      /* Deprecated */
		"STREAM_TYPE_RAW",     
	};
		
	dprintf(DEBUG_INFO, "%s:%s - Streamtype=%s\n", FILENAME, __FUNCTION__, aSTREAMTYPE[type]);	

	if (ioctl(audio_fd, AUDIO_SET_STREAMTYPE, type) < 0)
		perror("AUDIO_SET_STREAMTYPE");	
	
	StreamType = type;	
}

void cAudio::SetEncoding(audio_encoding_t type)
{
	const char *aENCODING[] = {
		"AUDIO_ENCODING_AUTO",
		"AUDIO_ENCODING_PCM",
		"AUDIO_ENCODING_LPCM",
		"AUDIO_ENCODING_MPEG1",
		"AUDIO_ENCODING_MPEG2",
		"AUDIO_ENCODING_MP3",
		"AUDIO_ENCODING_AC3",
		"AUDIO_ENCODING_DTS",
		"AUDIO_ENCODING_AAC",
		"AUDIO_ENCODING_WMA",
		"AUDIO_ENCODING_RAW",
		"AUDIO_ENCODING_LPCMA",
		"AUDIO_ENCODING_LPCMH",
		"AUDIO_ENCODING_LPCMB",
		"AUDIO_ENCODING_SPIDF",
		"AUDIO_ENCODING_DTS_LBR",
		"AUDIO_ENCODING_MLP",
		"AUDIO_ENCODING_RMA",
		"AUDIO_ENCODING_AVS",
		"AUDIO_ENCODING_NONE",
		"AUDIO_ENCODING_PRIVATE"
	};

	dprintf(DEBUG_INFO, "%s:%s - Encodingtype=%s\n", FILENAME, __FUNCTION__, aENCODING[type]);	

	if(ioctl(audio_fd, AUDIO_SET_ENCODING, type) < 0)
		perror("AUDIO_SET_ENCODING");	

	EncodingType = type;	
}
#else
void cAudio::SetStreamType(AUDIO_FORMAT type)
{
	const char *aAUDIOFORMAT[] = {
		"AUDIO_STREAMTYPE_AC3",
		"AUDIO_STREAMTYPE_MPEG",
		"AUDIO_STREAMTYPE_DTS"
		"AUDIO_STREAMTYPE_AAC",
		"AUDIO_STREAMTYPE_AACPLUS",
		"AUDIO_STREAMTYPE_LPCMDVD",
		"AUDIO_STREAMTYPE_MP3",
	};

	dprintf(DEBUG_INFO, "%s:%s - type=%s\n", FILENAME, __FUNCTION__, aAUDIOFORMAT[type]);

	if (ioctl(audio_fd, AUDIO_SET_BYPASS_MODE, type) < 0)
		perror("AUDIO_SET_BYPASS_MODE");

	StreamType = type;
}
#endif

void cAudio::SetSyncMode(int Mode)
{	
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);	

	if (::ioctl(audio_fd, AUDIO_SET_AV_SYNC, Mode) < 0)
		perror("AUDIO_SET_AV_SYNC");
}

int cAudio::Flush(void)
{  
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);

#ifdef __sh__	
	if (ioctl(audio_fd, AUDIO_FLUSH, NULL) < 0)
#else
	if (ioctl(audio_fd, AUDIO_CLEAR_BUFFER) < 0)
#endif
		perror("AUDIO_FLUSH");
	
	return 0;
}

/* select channels */
int cAudio::setChannel(int channel)
{
	const char * aAUDIOCHANNEL[] = {
		"STEREO",
		"MONOLEFT",
		"MONORIGHT",
	};
	 
	dprintf(DEBUG_INFO, "%s:%s %s\n", FILENAME, __FUNCTION__, aAUDIOCHANNEL[channel]);

	if (ioctl(audio_fd, AUDIO_CHANNEL_SELECT, (audio_channel_select_t)channel) < 0)
		perror("AUDIO_CHANNEL_SELECT");
	
	return 0;
}

/* pcm writer needed for audioplayer */
#if defined (ENABLE_PCMSOFTDECODER)
static unsigned int SubFrameLen = 0;
static unsigned int SubFramesPerPES = 0;

static const unsigned char clpcm_pes[18] = {   0x00, 0x00, 0x01, 0xBD, //start code
					0x07, 0xF1,             //pes length
					0x81, 0x81, 0x09,       //fixed
					0x21, 0x00, 0x01, 0x00, 0x01, //PTS marker bits
					0x1E, 0x60, 0x0A,           //first pes only, 0xFF after
					0xFF
			};

static const unsigned char clpcm_prv[14] = {   0xA0,   //sub_stream_id
					0, 0,   //resvd and UPC_EAN_ISRC stuff, unused
					0x0A,   //private header length
					0, 9,   //first_access_unit_pointer
					0x00,   //emph,rsvd,stereo,downmix
					0x0F,   //quantisation word length 1,2
					0x0F,   //audio sampling freqency 1,2
					0,              //resvd, multi channel type
					0,              //bit shift on channel GR2, assignment
					0x80,   //dynamic range control
					0, 0    //resvd for copyright management
			};

static unsigned char lpcm_pes[18];
static unsigned char lpcm_prv[14];

static unsigned char breakBuffer[8192];
static unsigned int breakBufferFillSize = 0;
#endif

//Neutrino uses softdecoder to playback music, this has to be inserted as pcm in the player
int cAudio::PrepareClipPlay(int NoOfChannels, int SampleRate, int BitsPerSample, int LittleEndian)
{ 
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);
	dprintf(DEBUG_INFO, "%s rate: %d ch: %d bits: %d (%d bps)\n",FILENAME, SampleRate, NoOfChannels, BitsPerSample, (BitsPerSample / 8));	
	
#if defined (ENABLE_PCMSOFTDECODER)
	uNoOfChannels = NoOfChannels;
	uSampleRate = SampleRate;
	uBitsPerSample = BitsPerSample;
	bLittleEndian = LittleEndian;

	SubFrameLen = 0;
	SubFramesPerPES = 0;
	breakBufferFillSize = 0;

	memcpy(lpcm_pes, clpcm_pes, sizeof(lpcm_pes));
	memcpy(lpcm_prv, clpcm_prv, sizeof(lpcm_prv));

	//figure out size of subframe
	//and set up sample rate
	switch(uSampleRate) 
	{
		case 48000:
			SubFrameLen = 40;
			break;

		case 96000:
			lpcm_prv[8] |= 0x10;
			SubFrameLen = 80;
			break;

		case 192000:
			lpcm_prv[8] |= 0x20;
			SubFrameLen = 160;
			break;

		case 44100:
			lpcm_prv[8] |= 0x80;
			SubFrameLen = 40;
			break;

		case 88200:
			lpcm_prv[8] |= 0x90;
			SubFrameLen = 80;
			break;

		case 176400:
			lpcm_prv[8] |= 0xA0;
			SubFrameLen = 160;
			break;

		default:
			break;
	}

	SubFrameLen *= NoOfChannels;
	SubFrameLen *= (BitsPerSample / 8);

	//rewrite PES size to have as many complete subframes per PES as we can
	SubFramesPerPES = ((2048-sizeof(lpcm_pes))-sizeof(lpcm_prv))/SubFrameLen;
	SubFrameLen *= SubFramesPerPES;

	lpcm_pes[4] = ((SubFrameLen+(sizeof(lpcm_pes)-6)+sizeof(lpcm_prv))>>8) & 0xFF;
	lpcm_pes[5] =  (SubFrameLen+(sizeof(lpcm_pes)-6)+sizeof(lpcm_prv))     & 0xFF;

	//set number of channels
	lpcm_prv[10]  = NoOfChannels - 1;

	switch(BitsPerSample) 
	{
		case    16: 
			break;
		case    24: 
			lpcm_prv[7] |= 0x20;
			break;

		default:
			printf("inappropriate bits per sample (%d) - must be 16 or 24\n",uBitsPerSample);
			return 1;
	}
	
	// set audio source to memory this able us to inject data
	setSource(AUDIO_SOURCE_MEMORY);
	
#ifdef __sh__	
	SetStreamType(STREAM_TYPE_PROGRAM);
	SetEncoding( (audio_encoding_t)AUDIO_ENCODING_LPCMA );
#else	
	
	SetStreamType(AUDIO_STREAMTYPE_LPCMDVD);
#endif

	Start();
#elif defined (ENABLE_DSP)
	int fmt;
	
	if (LittleEndian)
		fmt = AFMT_S16_BE;
	else
		fmt = AFMT_S16_LE;
	
#ifdef __sh__
	clipfd = open("/dev/dsp", O_WRONLY);
#else
	clipfd = open("/dev/sound/dsp", O_WRONLY);
#endif	
	
	if(clipfd <= 0)
	{
		printf("%s: clipfd open failed...(%m)\n", __FUNCTION__);
		return -1;
	}
	
	if (ioctl(clipfd, SNDCTL_DSP_RESET))
		perror("SNDCTL_DSP_RESET");
	
	if (ioctl(clipfd, SNDCTL_DSP_SETFMT, &fmt))
		perror("SNDCTL_DSP_SETFMT");
	
	if (ioctl(clipfd, SNDCTL_DSP_CHANNELS, &NoOfChannels))
		perror("SNDCTL_DSP_CHANNELS");
	
	if (ioctl(clipfd, SNDCTL_DSP_SPEED, &SampleRate))
		perror("SNDCTL_DSP_SPEED");

	//setVolume(volume, volume);
#endif	

	return 0;
}

int cAudio::WriteClip(unsigned char * buffer, int size)
{
#if defined (ENABLE_PCMSOFTDECODER)
	//unsigned int qty;
	unsigned int n;
	unsigned int injectBufferSize = sizeof(lpcm_pes)+sizeof(lpcm_prv)+SubFrameLen;
	unsigned char * injectBuffer = (unsigned char *)malloc(sizeof(unsigned char)*injectBufferSize);
	unsigned char * injectBufferDataPointer = &injectBuffer[sizeof(lpcm_pes)+sizeof(lpcm_prv)];

	for(int pos = 0; pos < size; )
	{
		//printf("%s:%s - Position=%d\n", FILENAME, __FUNCTION__, pos);

		if((size - pos) < SubFrameLen)
		{
			breakBufferFillSize = size - pos;
			memcpy(breakBuffer, &buffer[pos], sizeof(unsigned char)*breakBufferFillSize);

			//printf("%s:%s - Unplayed=%d\n", FILENAME, __FUNCTION__, breakBufferFillSize);
			break;
		}

                //get first PES's worth
		if(breakBufferFillSize > 0)
		{
			memcpy(injectBufferDataPointer, breakBuffer, sizeof(unsigned char)*breakBufferFillSize);
			memcpy(&injectBufferDataPointer[breakBufferFillSize], &buffer[pos], sizeof(unsigned char)*(SubFrameLen - breakBufferFillSize));
			pos += (SubFrameLen - breakBufferFillSize);
			breakBufferFillSize = 0;
		} else
		{
		        memcpy(injectBufferDataPointer, &buffer[pos], sizeof(unsigned char)*SubFrameLen);
			pos += SubFrameLen;
		}

		//write the PES header
		memcpy(injectBuffer, lpcm_pes, sizeof(lpcm_pes));

		//write the private data area
		memcpy(&injectBuffer[sizeof(lpcm_pes)], lpcm_prv, sizeof(lpcm_prv));

		//write the PCM data
		if(uBitsPerSample == 16) 
		{
			for(n=0; n<SubFrameLen; n+=2) 
			{
				unsigned char tmp;
				tmp=injectBufferDataPointer[n];
				injectBufferDataPointer[n]=injectBufferDataPointer[n+1];
				injectBufferDataPointer[n+1]=tmp;
			}
		} 
		else 
		{
			//A1cA1bA1a-B1cB1bB1a-A2cA2bA2a-B2cB2bB2a to A1aA1bB1aB1b.A2aA2bB2aB2b-A1cB1cA2cB2c
			for(n=0; n<SubFrameLen; n+=12) 
			{
				unsigned char tmp[12];
				tmp[ 0]=injectBufferDataPointer[n+2];
				tmp[ 1]=injectBufferDataPointer[n+1];
				tmp[ 8]=injectBufferDataPointer[n+0];
				tmp[ 2]=injectBufferDataPointer[n+5];
				tmp[ 3]=injectBufferDataPointer[n+4];
				tmp[ 9]=injectBufferDataPointer[n+3];
				tmp[ 4]=injectBufferDataPointer[n+8];
				tmp[ 5]=injectBufferDataPointer[n+7];
				tmp[10]=injectBufferDataPointer[n+6];
				tmp[ 7]=injectBufferDataPointer[n+11];
				tmp[ 8]=injectBufferDataPointer[n+10];
				tmp[11]=injectBufferDataPointer[n+9];
				
				memcpy(&injectBufferDataPointer[n], tmp, 12);
			}
		}

		//increment err... subframe count?
		lpcm_prv[1] = ((lpcm_prv[1]+SubFramesPerPES) & 0x1F);

	        //disable PES to save calculating correct values
	        lpcm_pes[7] = 0x01;

	        //kill off first A_PKT only fields in PES header
	        lpcm_pes[14] = 0xFF;
	        lpcm_pes[15] = 0xFF;
	        lpcm_pes[16] = 0xFF;

		/* inject Data in audio device */
		write(audio_fd, injectBuffer, injectBufferSize);
	}

	free(injectBuffer);
	
	return size;
#elif defined (ENABLE_DSP)
	int ret;
	
	if (clipfd <= 0) 
	{
		printf("%s: clipfd not yet opened\n", __FUNCTION__);
		return -1;
	}
	
	ret = write(clipfd, buffer, size);
	
	if (ret < 0)
		printf("%s: write error (%m)\n", __FUNCTION__);
	
	return ret;
#endif
}

int cAudio::StopClip()
{
	dprintf(DEBUG_INFO, "%s:%s\n", FILENAME, __FUNCTION__);	
	
#if defined (ENABLE_PCMSOFTDECODER)
	breakBufferFillSize = 0;
	
	Stop();
#elif defined (ENABLE_DSP)
	if (clipfd <= 0) 
	{
		printf("%s: clipfd not yet opened\n", __FUNCTION__);
		return -1;
	}
	close(clipfd);
	clipfd = -1;
	
	//setVolume(volume, volume);
#endif

	return 0;
}

void cAudio::SetHdmiDD(int ac3)
{
	const char *aHDMIDD[] = {
		"passthrough",
		"downmix",
	};
	
	dprintf(DEBUG_INFO, "%s:%s %s\n", FILENAME, __FUNCTION__, aHDMIDD[ac3]);	

	int fd_ac3 = open("/proc/stb/audio/ac3", O_RDWR);
	
	write(fd_ac3, aHDMIDD[ac3], strlen(aHDMIDD[ac3]));

	close(fd_ac3);
	
#ifdef __sh__
	const char *aHDMIDDSOURCE[] = {
		"spdif",
		"pcm",
	};
	
	int fd = open("/proc/stb/hdmi/audio_source", O_RDWR);
	  
	write(fd, aHDMIDDSOURCE[ac3], strlen(aHDMIDDSOURCE[ac3]));
		
	close(fd);
#endif
}

/* set source */
int cAudio::setSource(audio_stream_source_t source)
{
	const char *aAUDIOSTREAMSOURCE[] = {
		"AUDIO_SOURCE_DEMUX",
		"AUDIO_SOURCE_MEMORY",
	};
		
	dprintf(DEBUG_INFO, "%s:%s - source=%s\n", FILENAME, __FUNCTION__, aAUDIOSTREAMSOURCE[source]);

	return ioctl(audio_fd, AUDIO_SELECT_SOURCE, source);
}

/* get source */
audio_stream_source_t cAudio::getSource(void)
{
	struct audio_status status;

	if ( ioctl(audio_fd, AUDIO_GET_STATUS, &status) < 0)
		perror("AUDIO_GET_STATUS");

	return status.stream_source;
}

int cAudio::setHwPCMDelay(int delay)
{
	dprintf(DEBUG_INFO, "%s:%s - delay=%d\n", FILENAME, __FUNCTION__, delay);
	
	if (delay != m_pcm_delay )
	{
		FILE *fp = fopen("/proc/stb/audio/audio_delay_pcm", "w");
		if (fp)
		{
			fprintf(fp, "%x", delay*90);
			fclose(fp);
			m_pcm_delay = delay;
			return 0;
		}
	}
	
	return -1;
}

int cAudio::setHwAC3Delay(int delay)
{
	dprintf(DEBUG_INFO, "%s:%s - delay=%d\n", FILENAME, __FUNCTION__, delay);
	
	if ( delay != m_ac3_delay )
	{
		FILE *fp = fopen("/proc/stb/audio/audio_delay_bitstream", "w");
		if (fp)
		{
			fprintf(fp, "%x", delay*90);
			fclose(fp);
			m_ac3_delay = delay;
			return 0;
		}
	}
	
	return -1;
}

