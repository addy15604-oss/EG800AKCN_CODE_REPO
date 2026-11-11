#ifndef __APP_TEST3_H__
#define __APP_TEST3_H__



#ifdef QUEC_AUDIO_DENOISE
bool ql_get_denoise_flag(void);
void ql_set_denoise_flag(bool onoff);
int ql_denoise_licenses_verify(void);
void ql_denoise_get_version(char *version );
void ql_denoise_mute_time(unsigned int delay_ms);

/***************************************************/
/*获取设备信息接口，获取的信息送到服务器获取license 
 dev_info：[in/out] 存放设备信息，malloc 1k 送入
 authcode ：授权码

 return ：0，成功，
          1，该设备已经被授权过,无需再次授权
          其他为错误值

*/
/***************************************************/
int ql_denoise_get_dev_info(unsigned char *dev_info,char *authcode);

/***************************************************/
/*保存license信息接口 ，服务器下发的数据送到这个接口校验
 license_data：需要保存的license信息
 data_len ：license 的长度

 return: 0成功，其他为错误值
*/
/***************************************************/

int ql_denoise_save_info_from_app(unsigned char *license_data,int data_len);
#endif


#endif /* __APP_TEST3_H__ */

