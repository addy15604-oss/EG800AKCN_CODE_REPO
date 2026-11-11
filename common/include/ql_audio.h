/*================================================================
  Copyright (c) 2022, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#ifndef _QL_AUDIO_H_
#define _QL_AUDIO_H_

 typedef enum
 {
	 AUDIOHAL_DTMF_0 = 0,
	 AUDIOHAL_DTMF_1,
	 AUDIOHAL_DTMF_2,
	 AUDIOHAL_DTMF_3,
	 AUDIOHAL_DTMF_4,
	 AUDIOHAL_DTMF_5,
	 AUDIOHAL_DTMF_6,
	 AUDIOHAL_DTMF_7,
	 AUDIOHAL_DTMF_8,
	 AUDIOHAL_DTMF_9,
	 AUDIOHAL_DTMF_A,
	 AUDIOHAL_DTMF_B,
	 AUDIOHAL_DTMF_C,
	 AUDIOHAL_DTMF_D,
	 AUDIOHAL_DTMF_S,		 //* key
	 AUDIOHAL_DTMF_P,		 //# key
	 AUDIOHAL_COMFORT_425,
	 AUDIOHAL_COMFORT_950,
	 AUDIOHAL_COMFORT_1400,
	 AUDIOHAL_COMFORT_1800,
 } AUDIOHAL_TONE_TYPE_T;

 typedef enum
 {
	 AUDIOHAL_SPK_MUTE		   = 0,
	 AUDIOHAL_SPK_VOL_1,
	 AUDIOHAL_SPK_VOL_2,
	 AUDIOHAL_SPK_VOL_3,
	 AUDIOHAL_SPK_VOL_4,
	 AUDIOHAL_SPK_VOL_5,
	 AUDIOHAL_SPK_VOL_6,
	 AUDIOHAL_SPK_VOL_7,
	 AUDIOHAL_SPK_VOL_8,
	 AUDIOHAL_SPK_VOL_9,
	 AUDIOHAL_SPK_VOL_10,
	 AUDIOHAL_SPK_VOL_11,	 // 11
 
	 AUDIOHAL_SPK_VOL_QTY	 // 12
 } AUDIOHAL_SPK_LEVEL_T;

 typedef enum
 {
	 AUD_INTERNAL_CODEC,
	 AUD_EXTERNAL_CODEC,
 }codec_choose_state;

 typedef enum
 {
	 PCM_FS_32,
	 PCM_FS_64,
	 PCM_FS_128,
	 PCM_FS_256,
	 PCM_FS_512,
	 PCM_FS_1024,
	 PCM_FS_2048,
	 PCM_FS_MAX,
 }ql_pcm_bclk;

 typedef enum
 {
	 PCM_CODEC_SAMPLE_8000 = 8000,
	 PCM_CODEC_SAMPLE_16000 = 16000,
	 PCM_CODEC_SAMPLE_32000 = 32000,
	 PCM_CODEC_SAMPLE_48000 = 48000,
 }ql_pcm_codec_sample;

 typedef struct ql_codecpcm_config
 {
	 ql_pcm_bclk pcmbclk;
	 ql_pcm_codec_sample sample;
	 char is_i2s;//为1则使用i2c协议
 }ql_codecpcm_config;

 typedef enum audio_track_event {
     /** dummy event */
     AUDIO_TRACK_EVENT_NONE,
     /** tick event, 20 ms typically */
     AUDIO_TRACK_EVENT_TICK,
     /** start event, current track is involved in schedule*/
     AUDIO_TRACK_EVENT_STARTED,
     /** waiting event, current track is preempted by other track*/
     AUDIO_TRACK_EVENT_WAITING,
     /** running event, current track is recovered from waiting*/
     AUDIO_TRACK_EVENT_RUNNING,
     /** underrun event, current track has played out all the pcm data*/
     AUDIO_TRACK_EVENT_UNDERRUN,
     /** overrun event, current track can not hold more pcm data*/
     AUDIO_TRACK_EVENT_OVERRUN,
     /** fast up event, current track need more pcm data*/
     AUDIO_TRACK_EVENT_NEARLY_UNDERRUN,
     AUDIO_TRACK_EVENT_FASTUP = AUDIO_TRACK_EVENT_NEARLY_UNDERRUN,
     /** slow down event, current track need less pcm data*/
     AUDIO_TRACK_EVENT_NEARLY_OVERRUN,
     AUDIO_TRACK_EVENT_SLOWDOWN = AUDIO_TRACK_EVENT_NEARLY_OVERRUN,
     /** close event, current track is closed*/
     AUDIO_TRACK_EVENT_CLOSED,
     /** draining event, current track is in draining*/
     AUDIO_TRACK_EVENT_DRAINING,
     /** terminate event, current track is terminated in an unexpected way*/
     AUDIO_TRACK_EVENT_TERMINATED,
     /** consume event, current track has consumed vendor buffer*/
     AUDIO_TRACK_EVENT_CONSUMED,
     /** prepare event, current track has sent actual data to hardware cache*/
     AUDIO_TRACK_EVENT_PREAPARED,
        
     AUDIO_TRACK_EVENT_ENUM_32_BIT = 0x7FFFFFFF //32bit enum compiling enforcement
 }acm_audio_track_event_t;
 /** audio record event to indicate the internal capture status*/
 typedef enum audio_record_event {
     /** dummy event */
     AUDIO_RECORD_EVENT_NONE,
     /** tick event, 20 ms typically*/
     AUDIO_RECORD_EVENT_TICK,
     /** underrun event, current record has no pcm data*/
     AUDIO_RECORD_EVENT_UNDERRUN,
     /** overrun event, current record can not hold more pcm data*/
     AUDIO_RECORD_EVENT_OVERRUN,
     /** close event, current record is closed*/
     AUDIO_RECORD_EVENT_CLOSED,
     /** nearly underrun event, current record will underrun soon*/
     AUDIO_RECORD_EVENT_NEARLY_UNDERRUN,
     /** nearly overrun event, current record will overrun soon*/
     AUDIO_RECORD_EVENT_NEARLY_OVERRUN,
 
     AUDIO_RECORD_EVENT_ENUM_32_BIT = 0x7FFFFFFF //32bit enum compiling enforcement
 }acm_audio_record_event_t;
     /** audio record control supported*/
    typedef enum audio_record_control {
        /** suspend current record*/
        AUDIO_RECORD_CTRL_SUSPEND,
        /** resume current record*/
        AUDIO_RECORD_CTRL_RESUME,
        /** mute on current record*/
        AUDIO_RECORD_CTRL_MUTE_ON,
        /** mute off current record*/
        AUDIO_RECORD_CTRL_MUTE_OFF,
        /** extra gain for current record*/
        AUDIO_RECORD_CTRL_GAIN,
        /** ramp level for current record*/
        AUDIO_RECORD_CTRL_RAMP,
        /** private data for current record*/
        AUDIO_RECORD_CTRL_PRIVATE,

        AUDIO_RECORD_CTRL_ENUM_32_BIT = 0x7FFFFFFF //32bit enum compiling enforcement
    }acm_audio_record_control_t;
 typedef unsigned int acm_audio_track_handle;
  /** audio track event callback, inform audio track user of internal state*/
 typedef void(*acm_audio_track_event_callback_t)(acm_audio_track_handle, acm_audio_track_event_t);  
  typedef void(*audio_track_callback)(acm_audio_track_handle handle, acm_audio_track_event_t);
  typedef void(*audio_record_callback)(acm_audio_track_handle handle, acm_audio_record_event_t);
 
 /** audio track interaction method supported*/
 typedef enum audio_track_write_mode {
     /** asynchronous write mode, fail when too much pcm data to copy*/
     AUDIO_TRACK_WRITE_MODE_ASYNC,
     /** synchronous write mode, block when too much pcm data to copy*/
     AUDIO_TRACK_WRITE_MODE_SYNC,
     /** asynchronous vendor mode, do not block, do not fail*/
     AUDIO_TRACK_WRITE_MODE_VENDOR,
 
     AUDIO_TRACK_WRITE_MODE_ENUM_32_BIT = 0x7FFFFFFF //32bit enum compiling enforcement
 }acm_audio_track_write_mode_t; 
 /** audio track direction to specify near or far or both*/
 typedef enum audio_track_direction {
     /** direction near, current track will play to near side*/
     AUDIO_TRACK_DIRECTION_NEAR,
     AUDIO_TRACK_DIRECTION_SPEAKER = AUDIO_TRACK_DIRECTION_NEAR,
     /** direction far, current track will play to far side*/
     AUDIO_TRACK_DIRECTION_FAR,
     AUDIO_TRACK_DIRECTION_MIC = AUDIO_TRACK_DIRECTION_FAR,
     /** direction both, current track will play to near and far side*/
     AUDIO_TRACK_DIRECTION_BOTH,
 
     AUDIO_TRACK_DIRECTION_ENUM_32_BIT = 0x7FFFFFFF //32bit enum compiling enforcement
 }acm_audio_track_direction_t;
 /** audio track control supported*/
typedef enum audio_track_control {
    /** suspend current track*/
    AUDIO_TRACK_CTRL_SUSPEND,
    /** resume current track*/
    AUDIO_TRACK_CTRL_RESUME,
    /** mute on current track*/
    AUDIO_TRACK_CTRL_MUTE_ON,
    /** mute off current track*/
    AUDIO_TRACK_CTRL_MUTE_OFF,
    /** extra gain for current track*/
    AUDIO_TRACK_CTRL_GAIN,
    /** ramp level for current track*/
    AUDIO_TRACK_CTRL_RAMP,
    /** private data for current track*/
    AUDIO_TRACK_CTRL_PRIVATE,

    AUDIO_TRACK_CTRL_ENUM_32_BIT = 0x7FFFFFFF //32bit enum compiling enforcement
}acm_audio_track_control_t;
 /** audio track schedule mode*/
 typedef enum audio_track_mode {
     /** combine mode, current track will mix with other tracks which have the same mode*/
     AUDIO_TRACK_MODE_COMBINE,
     /** exclusive mode, current track will not mix with other tracks, can be preempted*/
     AUDIO_TRACK_MODE_EXCLUSIVE,
     /** preempt mode, current track will not mix with other tracks, can not be preempted**/
     AUDIO_TRACK_MODE_PREEMPT,
     /** terminate mode, current track will mix with other tracks which have the same mode, terminate others if possible*/
     AUDIO_TRACK_MODE_TERMINATE,
 
     AUDIO_TRACK_MODE_ENUM_32_BIT = 0x7FFFFFFF //32bit enum compiling enforcement
 }acm_audio_track_mode_t;
  /** audio track pcm format supported*/
 typedef enum audio_track_format {
     /** Signed, 8-bit */
     AUDIO_TRACK_FORMAT_S8 = 1,
     /** Signed 16-bit, little endian */
     AUDIO_TRACK_FORMAT_S16_LE = 0,
     /** Signed, 16-bit, big endian */
     AUDIO_TRACK_FORMAT_S16_BE = 2,
     /** float point 32-bit */
     AUDIO_TRACK_FORMAT_F32,
     /** Unsigned 8-bit,a-law */
     AUDIO_TRACK_FORMAT_U8_ALAW,
     /** Unsigned 8-bit,u-law */
     AUDIO_TRACK_FORMAT_U8_ULAW,
 
     AUDIO_TRACK_FORMAT_ENUM_32_BIT = 0x7FFFFFFF //32bit enum compiling enforcement
 }acm_audio_track_format_t;
 /** audio track configuration to specify playback property*/
 typedef struct audio_track_config {
     /** pcm channel, non-zero expected*/
     unsigned int channels;
     /** pcm sample rate, non-zero expected*/
     unsigned int rate;
     /** audio track capacity, how many microsecond of pcm can be held*/
     unsigned int capacity;
     /** audio track start threshold, how many microsecond of pcm is held before involving in schedule*/
     unsigned int start_threshold;
     /** audio track prepare threshold, how many microsecond of pcm is held before playing out*/
     unsigned int prepare_threshold;
     /** initial gain in q8 format, 0db by default*/
     int gain_value;
     /** initial speed factor in q8 format, x1 by default*/
     int speed_factor;
     /** eq effect index, 0 for none, minus 1 for lookup*/
     int eq;
     /** playback property in bitmap format, refer @ref AUDIO_PLAY_OPTION*/
     unsigned long long option;
     /** enhance in dsp further more if any*/
     int dsp_mode;
     /** override voice call data if any*/
     int override_voice;
     /** pcm format, 16 bit depth little endian by default*/
     acm_audio_track_format_t format;
     /** schedule mode, combine mode by default*/
     acm_audio_track_mode_t mode;
     /** audio track direction, near side by default*/
     acm_audio_track_direction_t direction;
     /** audio track event callback*/
     acm_audio_track_event_callback_t event_cb;
     /** audio track write mode, 0 for non-block*/
     acm_audio_track_write_mode_t write_mode;
     /** turn off audio effect for gain etc*/
     unsigned int effect_off;
 }acm_audio_track_config_t;
   /** audio playback property in bitmap format, set bitfield and get the option field in int64 */
   typedef struct AUDIO_PLAY_OPTION {
       union {
           long long option;
           struct {
               /** bit0~bit1, dest play end, 0->near,1->far,2->both, refer @ref acm_audio_track_direction_t*/
               unsigned int dest_end : 2;
               /** bit2, override voice if any, 0->false,1->true */
               unsigned int override : 1;
               /** bit3, near codec or vocoder, 0->codec,1->vocoder */
               unsigned int active_point : 1;
               /** bit4, reserved */
               unsigned int : 1;
               /** bit5, play current file in cyclic mode, 0->false,1->true */
               unsigned int cyclic : 1;
               /** bit6~bit7, schedule mode, 0->combine,1->exclusive,2->preempt, refer @ref acm_audio_track_mode_t */
               unsigned int mode : 2;
               /** bit8~bit11, eq effect index, 0 for EQ off */
               unsigned int eq : 4;
               /** bit12~bit20, speed factor in Q8, 0 for normal*/
               unsigned int speed : 9;
               /** bit21~bit23, write mode, refer @ref acm_audio_track_write_mode_t*/
               unsigned int write_mode : 3;
               /** bit24~bit27, pcm format, refer @ref acm_audio_track_format_t*/
               unsigned int fmt : 4;
               /** bit28, turn off audio effect */
               unsigned int effect_off : 1;
               /** bit29~bit31, reserved */
               unsigned int : 3;
               /** bit32~bit41, how many frames can be held*/
               unsigned int capacity : 10;
               /** bit42~bit45, how many frames to keep dynamically, level 0~7*/
               unsigned int latency : 3;
               /** bit46~bit48, how many mixed frames to keep dynamically, level 0~7*/
               unsigned int latency2 : 3;
           };
       };
   }AUDIO_PLAY_OPTION;
   
   /** audio record property in bitmap format, set bitfield and get the option field in int64 */
   typedef struct AUDIO_RECORD_OPTION {
       union {
           long long option;
           struct {
               /** bit0~bit3, record mode, refer @ref acm_audio_record_mode_t */
               unsigned int mode : 4;
               /** bit4~bit9, record gain in Q0 */
               int gain : 6;
               /** bit10, turn off audio effect */
               unsigned int effect_off : 1;
               /** bit11, do not operate audio path */
               unsigned int observer : 1;
           };
       };
   }AUDIO_RECORD_OPTION;

 typedef enum Mp3PlayEventType {
    /** mp3 playback status, value with type @ref Mp3PlayEventValue*/
    MP3_PLAYBACK_EVENT_STATUS,
    /** id3 header size, value with type int */
    MP3_FILE_EVENT_ID3OFFSET,
    /** sample rate, value with type int*/
    MP3_FILE_EVENT_SAMPLERATE,
    /** channel, value with type int*/
    MP3_FILE_EVENT_CHANNEL,
    /** bit rat, value with type int*/
    MP3_FILE_EVENT_BITRATE,
    /** file name request, value with type char**/
    MP3_FILE_EVENT_FILENAME,
}Mp3PlayEventType;

 typedef enum Mp3PlayEventValue {
     /** play out all for file or stream*/
     MP3_PLAYBACK_STATUS_ENDED = 0,
     /** indicate current playback is started*/
     MP3_PLAYBACK_STATUS_STARTED,
     /** indicate output device is opened*/
     MP3_PLAYBACK_STATUS_OPENED,
     /** reach file end*/
     MP3_PLAYBACK_STATUS_FILE_READED,
     /** reach timeout setting*/
     MP3_PLAYBACK_STATUS_TIMEOUT,
     /** indicate current playback is started for pcm processing*/
     MP3_PLAYBACK_STATUS_PCM_STARTED,
        
     MP3_PLAYBACK_STATUS_STREAM = 100,
     /** indicate mp3 data producer should slow down to avoid overrun*/
     MP3_STREAM_STATUS_NEARLY_OVERRUN,
     MP3_STREAM_STATUS_SLOW_DOWN = MP3_STREAM_STATUS_NEARLY_OVERRUN,
     /** indicate mp3 data producer should fast up to avoid underrun*/
     MP3_STREAM_STATUS_NEARLY_UNDERRUN,
     MP3_STREAM_STATUS_FAST_UP = MP3_STREAM_STATUS_NEARLY_UNDERRUN,
 }Mp3PlayEventValue;

 typedef unsigned int Mp3PlaybackHandle;
 typedef void(*Mp3PlaybackEventCallback)(Mp3PlayEventType, int);
 typedef void(*Mp3PlaybackEventCallback2)(Mp3PlayEventType, int, Mp3PlaybackHandle);
 
 typedef struct Mp3PlayConfigInfo {
     /** mp3 playback property in bitmap format, refer @ref AUDIO_PLAY_OPTION in @ref acm_audio_def.h*/
     unsigned long long option;
     /** block size for file only, 0 for frame mode*/
     int block_size;
     /** how many mp3 frames will be skipped before playback*/
     int skip_frames;
     /** how many mp3 frames will be held before playback*/
     int cache_frames;
     /** user registers callback to get playback event such as file end etc */
     Mp3PlaybackEventCallback listener;
     /** user registers callback to get playback event with handle */
     Mp3PlaybackEventCallback2 listener2;
	 /** how many microseconds before trigger auto stop*/
     int timeout;
     /** start with sync mode*/
     int sync_mode;
 }Mp3PlayConfigInfo; 
 typedef unsigned int audio_track_handle;
 typedef unsigned int audio_record_handle;
 typedef unsigned int Mp3PlaybackHandle;
 typedef int (*ql_cb_dtmf_detect)(unsigned short DTMFCode);
 typedef void (*ql_cb_on_speakerpa) (unsigned int on);
 typedef void(*audio_track_callback)(audio_track_handle handle, acm_audio_track_event_t);
 typedef void(*audio_record_callback)(audio_record_handle handle, acm_audio_record_event_t);
 typedef enum AmrPlaybackEventValue {
     /** play out all for file or stream*/
     AMR_PLAYBACK_STATUS_ENDED = 0,
     /** indicate current playback is started*/
     AMR_PLAYBACK_STATUS_STARTED,
     /** indicate current file can not be recognized*/
     AMR_FILE_STATUS_WRONG_FORMAT,
     /** indicate current playback is started for pcm processing*/
     AMR_PLAYBACK_STATUS_PCM_STARTED,
        
     AMR_PLAYBACK_STATUS_STREAM = 100,
     /** indicate amr data producer should slow down to avoid overrun*/
     AMR_STREAM_STATUS_NEARLY_OVERRUN,
     AMR_STREAM_STATUS_SLOW_DOWN = AMR_STREAM_STATUS_NEARLY_OVERRUN,
     /** indicate amr data producer should fast up to avoid underrun*/
     AMR_STREAM_STATUS_NEARLY_UNDERRUN,
     AMR_STREAM_STATUS_FAST_UP = AMR_STREAM_STATUS_NEARLY_UNDERRUN,
 }AmrPlaybackEventValue;
     typedef enum AmrFileStatus {
        /** indicate current file has been readed and played out */
        AMR_FILE_STATUS_ENDED = 0,
        /** indicate output device is opened*/
        AMR_FILE_STATUS_STARTED,
    }AmrFileStatus;
  typedef enum AmrPlaybackEventType {
     /** status related event type, refer @ref AmrFileStatus for event value*/
     AMR_PLAYBACK_EVENT_STATUS,
     /** amr rate event type, value ranges from 0 to 7*/
     AMR_PLAYBACK_EVENT_RATE,
     /** file name request, value with type <tt>char*<tt>*/
     AMR_PLAYBACK_EVENT_FILENAME,
 }AmrPlaybackEventType;
 typedef AmrPlaybackEventType AmrFileEventType;
 typedef void(*AmrPlaybackEventCallback)(AmrPlaybackEventType, int);
  typedef struct AmrPlaybackConfigInfo {
      /** playback option in bitmap format, refer @ref AUDIO_PLAY_OPTION in @ref acm_audio_def.h */
      unsigned long long option;
      /** block size for file only, 0 for frame mode*/
      int block_size;
      /** user registers callback to get playback event such as file end etc */
      AmrPlaybackEventCallback listener;
  }AmrPlaybackConfigInfo;
 typedef AmrPlaybackConfigInfo AmrFileConfigInfo;
 typedef void(*AmrStreamCallback)(const unsigned char*, unsigned int);
 typedef struct AmrEncConfigInfo {
     /** encoder mode, refer @ref acm_audio_record_mode_t*/
     int mode;
     /** extra digital gain for recorded pcm before encoding, unit:dB, default:0 */
     int gain;
     /** encoder rate, 0~7 for nb, 0~8 for wb, 0 for lowest rate*/
     int rate;
     /** dtx on or off, 0:off, 1:on*/
     int dtx;
     /** wb-amr mode*/
     int wb_mode;
     /** record timeout in microsecond*/
     int timeout;
     /** user registers callback to get encoded amr stream */
     AmrStreamCallback callback;
 }AmrEncConfigInfo;

 typedef unsigned int AmrPlaybackHandle;
 typedef unsigned int AmrEncodeHandle;
typedef struct
{    
    unsigned short  drcGain;
    unsigned short  cutOffFreqIdx;
    unsigned short  resv1;
    unsigned short  resv2;
    
} QL_MediaDRCParaT;

typedef struct
{    
    unsigned short  maxRms;
    unsigned short  Alpha;
    unsigned short  N2cos;
    unsigned short resv;
    
} QL_SPKProParaT;

typedef struct
{
    short a[3];    // a[0], a[1], a[2]
    short b[3];   // b[0], b[1], b[2]
} QL_BiQuadCoeffsStruct;

typedef struct
{
    unsigned short BiquadEnControl;     //bit 0 ~bit 4 is on/off of each filter.
    //unsigned short reserved0;
    //short     reserve1;
    QL_BiQuadCoeffsStruct Tuningmem[5];     //get from user input or read workspace log
}QL_EqParamIn;

typedef struct
{    
    unsigned int    index;
    unsigned int    onoffCtl; // bit0:eq,  bit1: drc, bit2: spkPro
    QL_EqParamIn        eqParam;
    QL_MediaDRCParaT drcParam;
    QL_SPKProParaT spkProParam;
    unsigned short          Reserved[7];
    unsigned short          Tag;        //Fixed as 0xBEEF
} QL_MediaVEarametersT;

audio_track_handle ql_audio_track_open(unsigned int rate, unsigned int channels, unsigned long long option, audio_track_callback event_cb);
int ql_audio_track_write(audio_track_handle handle, char* data, unsigned int size);
int ql_audio_track_close(audio_track_handle handle,int force);
int ql_audio_track_ctrl(audio_track_handle handle, acm_audio_track_control_t ctrl_type, int ctrl_value);
audio_record_handle  ql_audio_record_open(unsigned int rate, unsigned int channels, unsigned long long option, audio_record_callback event_cb);
int ql_audio_record_read(audio_record_handle handle, char*data, unsigned int* count);
int ql_audio_record_close(audio_record_handle handle,int force);
int ql_audio_record_ctrl(audio_record_handle handle, acm_audio_record_control_t ctrl_type, int ctrl_value); 
int ql_wav_play(const char* file_name, struct audio_track_config* track_config, audio_track_handle* handle);
int ql_wav_play_stop(audio_track_handle handle,char force);
int ql_get_audio_state(void);
void ql_codec_choose(codec_choose_state codec_choose, ql_codecpcm_config* pcm_config);
void ql_set_audio_path_receiver(void);
void ql_set_volume(AUDIOHAL_SPK_LEVEL_T vol);
int ql_play_mp3(char *file_name,Mp3PlaybackHandle* handle,Mp3PlayConfigInfo* mp3_configInfo);
int ql_play_mp3_stream_buffer(Mp3PlaybackHandle handle,unsigned char *mp3_buff, int mp3_size);
int ql_stop_mp3_play(Mp3PlaybackHandle handle,int drain);
int ql_mp3_get_pcm_handle(Mp3PlaybackHandle handle, unsigned int* pcm_handle);
int ql_get_volume(void);
void ql_bind_speakerpa_cb(ql_cb_on_speakerpa cb);
void ql_set_audio_path_earphone(void);
void ql_set_audio_path_speaker(void);
void ql_set_dtmf_volume(int vo);
int ql_aud_dtmf_play(AUDIOHAL_TONE_TYPE_T dtmf, int onoff);
int ql_aud_tone_play(int f1, int f2, int onoff);
void ql_set_txdspgain(signed char gain);
signed char ql_get_txdspgain(void);
void ql_set_txcodecgain(short gain);
short ql_get_txcodecgain(void);
int ql_set_dsp_gain_table(unsigned int profile_id, signed short *gain);
int ql_set_txcodec_gain_table(unsigned int profile_id, unsigned int PGA_Stage1, unsigned int PGA_Stage2, unsigned int diggain);
int ql_set_rxcodec_gain_table(unsigned int profile_id, signed int level, unsigned int data1,  unsigned int data2, unsigned int data3, unsigned int data4, unsigned int data5);
int ql_set_eqcoeff_table(unsigned int profile_id, signed int tx_rx, unsigned int rate, signed short * data);
int ql_set_mediave_coeff_table(QL_MediaVEarametersT *AudioMediaVETable);
int ql_play_amr(char *file_name,AmrPlaybackHandle* handle,AmrPlaybackConfigInfo* amr_configInfo);
int ql_play_amr_stream_buffer(AmrPlaybackHandle handle,unsigned char *amr_buff, int amr_size);
int ql_stop_amr_play(AmrPlaybackHandle handle,int drain);
int ql_amr_get_pcm_handle(AmrPlaybackHandle handle, unsigned int* track_handle);
int ql_record_amr(char* file_name,AmrEncodeHandle* handle,AmrEncConfigInfo* config);
int ql_record_amr_stop(AmrEncodeHandle handle);
void ql_set_dtmf_volume2(int vo);/*音频等级-36 - 16*/
void ql_audio_pipeline_config(unsigned int rate, unsigned int channel);
int ql_audio_track_pipeline_read(unsigned char * buf, unsigned int * size);

/***********************************************************************
*
* Name:   ql_set_audio_ouput_type    
*
* Description: 设置1602平台pwm模拟方案中的输出方式：差分输出/单端输出（需要同硬件设计匹配）（此接口仅对1602平台项目有效）
*
* Parameters:  mode：0----差分输出（默认方式）
*					 1----单端输出
*			  
* 说明：差分输出时，对应的SPI_CLK脚不允许使用，单端时由app控制。		  
***********************************************************************/
void ql_set_audio_ouput_type(char mode);


/***********************************************************************
*
* Name:   ql_get_audio_ouput_type    
*
* Description: 获取1602平台pwm模拟方案中软件设置的输出方式：差分输出/单端输出（此接口仅对1602平台项目有效）
*
* Returns:	  0----差分输出（默认方式）
*			  1----单端输出
*
* 说明：差分输出时，对应的SPI_CLK脚不允许使用，单端时由app控制。	
***********************************************************************/
char ql_get_audio_ouput_type(void);


/****************************old api****************************/
#include "ql_rtos.h"

typedef struct ql_pcm_hadle
{  
    unsigned int handle;
    char record_or_play;
    ql_flag_t pcm_stream_flag;
}ql_pcm_hadle_t;

typedef ql_pcm_hadle_t * PCM_HANDLE_T ;
typedef struct {
	unsigned int channels;
	unsigned int samplerate;
	unsigned int periodcnt; // 20ms buffersize/per period, value:2~8
} QL_PCM_CONFIG_T;
typedef struct aud_input_para{
    int channels;
    int rate;
    int format;
    int hsize;
    int type;
    int file_size;
    int fmt_size;
}aud_input_para_t;

struct riff_chunk{
    unsigned int riff;
    unsigned int file_size;
    unsigned int wave;
};

struct format_chunk{
    unsigned int format;
    unsigned int format_size;
    unsigned short aud_fmt;
    unsigned short channel;
    unsigned int sample;
    unsigned int bytes;
    unsigned short block_align;
    unsigned short bits_sample;
    unsigned short others;
};

typedef enum{
    QUEC_PCM_FORMAT_S8 = 0,
    QUEC_PCM_FORMAT_S16_LE,
    QUEC_PCM_FORMAT_S24_LE,
    QUEC_PCM_FORMAT_S32_LE,
}QUEC_PCM_FMT_T;

typedef enum{
    AUD_STREAM_FORMAT_MP3 = 0,
    AUD_STREAM_FORMAT_AMR = 1,
    AUD_STREAM_FORMAT_PCM = 2,
    AUD_STREAM_FORMAT_AMRNB,
    AUD_STREAM_FORMAT_AMRWB,
    AUD_STREAM_FORMAT_END
}aud_stream_format;

struct data_chunk{
    unsigned int data;
    unsigned int data_size;
};

typedef enum
{
    AUD_PLAYER_ERROR = -1,
    AUD_PLAYER_START = 0,
    AUD_PLAYER_PAUSE,
    AUD_PLAYER_RECOVER,
    AUD_PLAYER_PLAYING,
    AUD_PLAYER_NODATA,
    AUD_PLAYER_LESSDATA,
    AUD_PLAYER_MOREDATA,
    AUD_PLAYER_FINISHED,
}enum_aud_player_state;

typedef enum
{
    MR475 = 0,/* 4.75 kbps */
    MR515,    /* 5.15 kbps */
    MR59,	   /* 5.90 kbps */
    MR67,	   /* 6.70 kbps */
    MR74,	   /* 7.40 kbps */
    MR795,    /* 7.95 kbps */
    MR102,    /* 10.2 kbps */
    MR122,    /* 12.2 kbps */
    MRDTX,    /* DTX		*/
    N_MODES   /* Not Used	*/
}QL_AMR_MODE;

typedef enum
{
    AMR_TX_WMF = 0,
    AMR_TX_IF2,
    AMR_TX_ETS,
}QL_AMR_FORMAT;

typedef void(*cb_on_player)(char *p_data, int len, enum_aud_player_state state);

int ql_audio_play_init(cb_on_player aud_cb);
int ql_play_file_start(char *file_name);
int ql_play_file_stop(void);
int ql_wav_file_play(char *file_name);
void ql_wav_file_stop(void);
int ql_mp3_file_play(char *file_name);
void ql_mp3_file_stop(void);
int ql_amr_file_play(char *file_name);
void ql_amr_file_stop(void);
int ql_amr_stream_write(void *data, unsigned int count);
int ql_amr_stream_close(void);
int ql_amrnbenc_do(unsigned char *input_buff, unsigned char *output_buff);
int ql_amrnbenc_init(QL_AMR_MODE mode, QL_AMR_FORMAT format);
int ql_amrnbenc_deinit(void);
PCM_HANDLE_T ql_pcm_open(QL_PCM_CONFIG_T *config, unsigned int flags);
int ql_pcm_read(PCM_HANDLE_T handle, void *data, unsigned int count);
int ql_pcm_write(PCM_HANDLE_T handle, void *data, unsigned int count);
int ql_pcm_close(PCM_HANDLE_T handle) ;
int ql_pcm_play_init(cb_on_player play_cb);
int ql_pcm_play(int hdl, unsigned char *buffer, int size );
int ql_pcm_play_stop(int hdl);
int ql_pcm_play_deinit(int hdl);

#endif
