/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include "ql_rtos.h"
#include "ql_data_call.h"
#include "ql_http_client.h"
#include "ql_application.h"
#include "ql_log.h"

#define PROFILE_IDX 1
#define PRINT_BUF_SIZE 65

static char print_buf[PRINT_BUF_SIZE];
static int test_http_app_upload_cb_count = 0;

typedef struct 
{
	int err_code;
	int need_range; /* 0 表示 需要进行分段下载测试， 1 表示不是分段下载*/
	int asyn_mode;	 /* 0：同步；1：异步*/
	ql_sem_t sem;
	int resp_code;
}http_app_response_context;

const static SSLCertPathPtr rootCA_path = 
"\
-----BEGIN CERTIFICATE-----\n\
MIIJrzCCCJegAwIBAgIMLO4ZPBiCeOo+Q3VzMA0GCSqGSIb3DQEBCwUAMGYxCzAJ\n\
BgNVBAYTAkJFMRkwFwYDVQQKExBHbG9iYWxTaWduIG52LXNhMTwwOgYDVQQDEzNH\n\
bG9iYWxTaWduIE9yZ2FuaXphdGlvbiBWYWxpZGF0aW9uIENBIC0gU0hBMjU2IC0g\n\
RzIwHhcNMTkwNTA5MDEyMjAyWhcNMjAwNjI1MDUzMTAyWjCBpzELMAkGA1UEBhMC\n\
Q04xEDAOBgNVBAgTB2JlaWppbmcxEDAOBgNVBAcTB2JlaWppbmcxJTAjBgNVBAsT\n\
HHNlcnZpY2Ugb3BlcmF0aW9uIGRlcGFydG1lbnQxOTA3BgNVBAoTMEJlaWppbmcg\n\
QmFpZHUgTmV0Y29tIFNjaWVuY2UgVGVjaG5vbG9neSBDby4sIEx0ZDESMBAGA1UE\n\
AxMJYmFpZHUuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtMa/\n\
2lMgD+pA87hSF2Y7NgGNErSZDdObbBhTsRkIsPpzRz4NOnlieGEuVDxJfFbawL5h\n\
VdVCcGoQvvW9jWSWIQCTYwmHtxm6DiA+SchT7QKPRgHroQeTc7vt8bPJ4vvd8Dkq\n\
g630QZi8huq6dKim49DlxY6zC7LSrJF0Dv+AECM2YmUItIf1VwwlxwDY9ahduDNB\n\
pypf2/pwniG7rkIWZgdp/hwmKoEPq3Pj1lIgpG2obNRmSKRv8mgKxWWhTr8EekBD\n\
HNN1+3WsGdZKNQVuz9Vl0UTKawxYBMSFTx++LDLR8cYo+/kmNrVt+suWoqDQvPhR\n\
3wdEvY9vZ8DUr9nNwwIDAQABo4IGGTCCBhUwDgYDVR0PAQH/BAQDAgWgMIGgBggr\n\
BgEFBQcBAQSBkzCBkDBNBggrBgEFBQcwAoZBaHR0cDovL3NlY3VyZS5nbG9iYWxz\n\
aWduLmNvbS9jYWNlcnQvZ3Nvcmdhbml6YXRpb252YWxzaGEyZzJyMS5jcnQwPwYI\n\
KwYBBQUHMAGGM2h0dHA6Ly9vY3NwMi5nbG9iYWxzaWduLmNvbS9nc29yZ2FuaXph\n\
dGlvbnZhbHNoYTJnMjBWBgNVHSAETzBNMEEGCSsGAQQBoDIBFDA0MDIGCCsGAQUF\n\
BwIBFiZodHRwczovL3d3dy5nbG9iYWxzaWduLmNvbS9yZXBvc2l0b3J5LzAIBgZn\n\
gQwBAgIwCQYDVR0TBAIwADBJBgNVHR8EQjBAMD6gPKA6hjhodHRwOi8vY3JsLmds\n\
b2JhbHNpZ24uY29tL2dzL2dzb3JnYW5pemF0aW9udmFsc2hhMmcyLmNybDCCA0kG\n\
A1UdEQSCA0AwggM8ggliYWlkdS5jb22CEmNsaWNrLmhtLmJhaWR1LmNvbYIQY20u\n\
cG9zLmJhaWR1LmNvbYIQbG9nLmhtLmJhaWR1LmNvbYIUdXBkYXRlLnBhbi5iYWlk\n\
dS5jb22CEHduLnBvcy5iYWlkdS5jb22CCCouOTEuY29tggsqLmFpcGFnZS5jboIM\n\
Ki5haXBhZ2UuY29tgg0qLmFwb2xsby5hdXRvggsqLmJhaWR1LmNvbYIOKi5iYWlk\n\
dWJjZS5jb22CEiouYmFpZHVjb250ZW50LmNvbYIOKi5iYWlkdXBjcy5jb22CESou\n\
YmFpZHVzdGF0aWMuY29tggwqLmJhaWZhZS5jb22CDiouYmFpZnViYW8uY29tgg8q\n\
LmJjZS5iYWlkdS5jb22CDSouYmNlaG9zdC5jb22CCyouYmRpbWcuY29tgg4qLmJk\n\
c3RhdGljLmNvbYINKi5iZHRqcmN2LmNvbYIRKi5iai5iYWlkdWJjZS5jb22CDSou\n\
Y2h1YW5rZS5jb22CCyouZGxuZWwuY29tggsqLmRsbmVsLm9yZ4ISKi5kdWVyb3Mu\n\
YmFpZHUuY29tghAqLmV5dW4uYmFpZHUuY29tghEqLmZhbnlpLmJhaWR1LmNvbYIR\n\
Ki5nei5iYWlkdWJjZS5jb22CEiouaGFvMTIzLmJhaWR1LmNvbYIMKi5oYW8xMjMu\n\
Y29tggwqLmhhbzIyMi5jb22CDiouaW0uYmFpZHUuY29tgg8qLm1hcC5iYWlkdS5j\n\
b22CDyoubWJkLmJhaWR1LmNvbYIMKi5taXBjZG4uY29tghAqLm5ld3MuYmFpZHUu\n\
Y29tggsqLm51b21pLmNvbYIQKi5zYWZlLmJhaWR1LmNvbYIOKi5zbWFydGFwcHMu\n\
Y26CESouc3NsMi5kdWFwcHMuY29tgg4qLnN1LmJhaWR1LmNvbYINKi50cnVzdGdv\n\
LmNvbYISKi54dWVzaHUuYmFpZHUuY29tggthcG9sbG8uYXV0b4IKYmFpZmFlLmNv\n\
bYIMYmFpZnViYW8uY29tggZkd3ouY26CD21jdC55Lm51b21pLmNvbYIMd3d3LmJh\n\
aWR1LmNughB3d3cuYmFpZHUuY29tLmNuMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggr\n\
BgEFBQcDAjAdBgNVHQ4EFgQUdrXm1kn4+DbqdaltXk1VWzdc/ccwHwYDVR0jBBgw\n\
FoAUlt5h8b0cFilTHMDMfTuDAEDmGnwwggEEBgorBgEEAdZ5AgQCBIH1BIHyAPAA\n\
dgC72d+8H4pxtZOUI5eqkntHOFeVCqtS6BqQlmQ2jh7RhQAAAWqaLuGaAAAEAwBH\n\
MEUCICx7TcD5hUeKLQrAeTvWtLVm+Kr7glitIzb+Frymg5khAiEAwC/NnJkgy32R\n\
X9KLxhMQc7XBVAMzQZ+masUUk89pK2sAdgBvU3asMfAxGdiZAKRRFf93FRwR2QLB\n\
ACkGjbIImjfZEwAAAWqaLt5PAAAEAwBHMEUCIAMyaJ450OtfGWHbpxJpbyhEgQKl\n\
PMKjE9V+mCZfIBqgAiEAp4tis7C0RDLiEf9FjVURLDarKZNEyDRcznw1VzGuqxIw\n\
DQYJKoZIhvcNAQELBQADggEBAKq5zVKO3DZdR9SL8zIXBkaDYKMnBUkpsRtGbjj+\n\
k/4JQ2zSoVgkEkK3q0H4Rwp9ZLV13FpFFLKkGGuctzuPs37SvcBySzUFrg0tGR9Q\n\
c3Ja35cYO9sq895EzmQtwR6EzHYkPjBnIyboT/cL9uxp139RqaBvuMQU4sBKSsQA\n\
XVdqyUHEJSsyGKpiqB5JgXMcgV9e+uSUMsNQbY6qzGxMUwz6j040eZ+lYMD4UHW4\n\
oZ0B5qslIww7JAJAWCT/NAKLlGEQaC+2gOPQX0oKpwLSwJg+HegCyCdxJrKoh7bb\n\
nRBHS8ITYjTG0Dw5CTklj/6i9PP735snPfzQKOht3N0X0x8=\n\
-----END CERTIFICATE-----\n\
";

static void print_string(const char * string,int len)
{
	int printed = 0;
	while (printed != len) 
	{
		if ((len - printed) > (PRINT_BUF_SIZE - 1)) 
		{
			memcpy(print_buf, string + printed, (PRINT_BUF_SIZE - 1));
			printed += (PRINT_BUF_SIZE - 1);
			print_buf[PRINT_BUF_SIZE - 1] = '\0';
		} 
		else 
		{
			snprintf(print_buf,len - printed+1, "%s", string + printed);
			printed = len;
		}
		printf("%s", print_buf);
	}
	printf("\n");
}

static void ql_datacall_status_callback(int profile_idx, int status)
{
	printf("profile(%d) status: %d\r\n", profile_idx, status);
}

static int datacall_satrt(void)
{
	int i=0;
	printf("wait for network register done\r\n");

	while(i<10)
	{
		if(ql_network_register_wait(120) == 0)
		{
			break;
		}
		else 
		{
			i++;
		}
		
	}

	if(i>=10)
	{
		printf("*** network register fail ***\r\n");
		return 1;
	}
	
	printf("doing network activing ...\r\n");
	
	ql_wan_start(ql_datacall_status_callback);
	ql_set_auto_connect(1, TRUE);
	ql_start_data_call(1, 0, NULL, NULL, NULL, 0);

	return 0;
}

static int http_app_response_cb(
	QL_HTTP_CLIENT_T *client,
	QL_HTTP_CLIENT_EVENT_E event,
	int status_code, 
	char *data, 
	int data_len, 
	void *private_data)
{
	int ret=0;
	char para[64]={0};
	http_app_response_context* resp_param = (http_app_response_context*)private_data;

	switch(event)
	{
		case QL_HTTP_CLIENT_EVENT_SEND_FAIL:
			printf("http send failed!, err= %d\r\n",status_code);
			if(resp_param->asyn_mode == 1 && resp_param->sem != NULL)
			{
				ql_rtos_semaphore_release(resp_param->sem);
			}
			break;
		case QL_HTTP_CLIENT_EVENT_SEND_SUCCESSED:
			printf("http send successed!\r\n");
			break;
		case QL_HTTP_CLIENT_EVENT_RECV_HEADER_FAIL:
			printf("http parse response header failed!\r\n");
			if(resp_param->asyn_mode == 1 && resp_param->sem != NULL)
			{
				ql_rtos_semaphore_release(resp_param->sem);
			}
			break;
		case QL_HTTP_CLIENT_EVENT_RECV_HEADER_FINISHED:
			printf("http recv header status_code:%d;header_len:%d!\r\n",status_code,data_len);
			ret=ql_http_client_get_header(data,"Content-Type",0,para,sizeof(para));
			if(ret>0)
				printf("Content-Type[len:%d]:%s!\n",ret,para);
			resp_param->resp_code = status_code;
			if(status_code == 200)
				printf("http recv response ok!\r\n");
			print_string(data,data_len);
			ret=1; //返回1，表示继续接受body数据，如果返回0，表示不继续接受body数据
			break;
		case QL_HTTP_CLIENT_EVENT_RECV_BODY:
			printf("http recv body status_code:%d;recv_body_len:%d!\r\n",status_code,data_len);
			print_string(data,data_len);
			ret=1; //返回1，表示继续接受body数据，如果返回0，表示不继续接受body数据
			break;
		case QL_HTTP_CLIENT_EVENT_RECV_BODY_FINISHED:
			printf("http recv body finished!\r\n");
			if(resp_param->asyn_mode == 1 && resp_param->sem != NULL)
			{
				ql_rtos_semaphore_release(resp_param->sem);
			}
			break;
		case QL_HTTP_CLIENT_EVENT_DISCONNECTED:
			printf("http be closed by server!\r\n");
			if(resp_param->asyn_mode == 1 && resp_param->sem != NULL)
			{
				ql_rtos_semaphore_release(resp_param->sem);
			}
			break;
		case QL_HTTP_CLIENT_EVENT_SOCK_RECV_FAIL:
			printf("http socket recv failed!\r\n");
			if(resp_param->asyn_mode == 1 && resp_param->sem != NULL)
			{
				ql_rtos_semaphore_release(resp_param->sem);
			}
			break;
		default:
			printf("other event= %d, exit http client\r\n",event);
			if(resp_param->asyn_mode == 1 && resp_param->sem != NULL)
			{
				ql_rtos_semaphore_release(resp_param->sem);
			}
			break;
	}
	return ret;
}

static int http_app_upload_cb(QL_HTTP_CLIENT_T *client, void *arg, char *data, int size)
{
	char* temp = NULL;
	if(arg == NULL || client == NULL || data == NULL)
	{
		printf("http upload cb param invalid!\r\n");
		return -1;
	}
	
	printf("size= %d, data= %p, %s\r\n",size,data,(char*)arg);
	if(size > 0)
	{	
		temp = malloc(size); /*操作需要上传的书数据，可以是内存中的值，也可以是使用文件系统读取文件*/
		if(temp == NULL)
		{
			return -1;
		}
		else 
		{
			memset(temp, test_http_app_upload_cb_count++,size);
			memcpy(data,temp,size);/*data指针再http client 中已经声请过内存，且申请的内存大小为size 大小*/
			free(temp);
		}
	}
	return size;
}

/*
run_num == 0 表示测试: basic auth 		|GET 	|no range 	|同步	| not total timeout
run_num == 1 表示测试: no auth 			|GET 	|no range	|同步	| not total timeout
run_num == 2 表示测试: no auth 			|GET 	|need range	|异步	| not total timeout
run_num == 3 表示测试: no auth 			|POST	|同步			|not total timeout
run_num == 4 表示测试: no auth 			|POST	|异步			|not total timeout
run_num == 5 表示测试: no auth 			|POST	|同步			|total timeout
run_num == 6 表示测试: no auth 			|POST	|异步			|total timeout

*/

static void http_test_new_app()
{
    int run_num=0,ret=0;
    struct http_client * client = NULL;
    struct http_client_list * header = NULL;
	char url[64] = "https://www.baidu.com";
	char username[64] = "test";
	char password[64] = "test";
    char upload_param[]="this is upload param!";
	http_app_response_context resp_param_ctx = {0};
	int need_upload_len = 300000;

	SSLConfig sslConfig = 
	{
		.en = 1,
		.profileIdx = PROFILE_IDX,
		.serverName = "www.baidu.com",
		.serverPort = 443,
		.protocol = 0,
		.dbgLevel = 4,
		.sessionReuseEn = 0,
		.vsn = SSL_VSN_ALL,
		.verify = SSL_VERIFY_MODE_OPTIONAL,
		.cert.from = SSL_CERT_FROM_BUF,
		.cert.path.rootCA = rootCA_path,
		.cert.path.clientKey = NULL,
		.cert.path.clientCert = NULL,
		.cert.clientKeyPwd.data = NULL,
		.cert.clientKeyPwd.len = 0,
		.cipherList = "ALL",
		.CTRDRBGSeed.data = NULL,
		.CTRDRBGSeed.len = 0
	};

	if(ql_rtos_semaphore_create(&resp_param_ctx.sem, 0) != kNoErr)
	{
		printf("create sem fail, exit test!\r\n");
		return ;
	}

    client = ql_http_client_init();
	while (client && (run_num <= 6)) 
	{
		printf("\r\n==============http_client_test[%d]================\r\n",run_num);
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_URL, url);										/*set url*/
		if(ret==QL_HTTP_CLIENT_ERR_LAST_REQUEST_NOT_FINISH)													
		{
			printf("last request not finish, not to set!\r\n");												/*如果函数返回QL_HTTP_CLIENT_ERR_LAST_REQUEST_NOT_FINISH错误码时，表示上一次http client请求还在进行，当前opt无法设置成功*/
			goto retry_wita;
		}

		ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PDP_CID, PROFILE_IDX);         				/*set PDP cid,if not set,using default PDP*/
		ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PROTOCOL_VER, 1);              				/*"0" is HTTP 1.1, "1" is HTTP 1.0*/
		ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_ENABLE_COOKIE, 1);             				/*"0" is DISENABLE COOKIE, "1" is ENABLE COOKIE*/
		ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_SSL_CTX, &sslConfig);							/*是否使能ssl是由url中的protocol字段决定，该条件的优先级最高*/
		if(run_num != 0)
		{
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_AUTH_TYPE, QL_HTTP_CLIENT_AUTH_TYPE_NONE);		/*No authentication*/
		}
		else
		{
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_AUTH_TYPE, QL_HTTP_CLIENT_AUTH_TYPE_BASE);		/*use basic authentication mode*/
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_USERNAME, username);
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_PASSWORD, password);
		}
		ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_RESPONSE_FUNC, (void*)http_app_response_cb);
		if(ret == QL_HTTP_CLIENT_ERR_SUCCESS)
		{
			resp_param_ctx.err_code = 0;
			resp_param_ctx.resp_code = 0;
			if(run_num != 2)
			{
				resp_param_ctx.need_range = 0;
			}
			else 
			{
				resp_param_ctx.need_range = 1;
			}
			
			if(run_num == 0 || run_num == 1 || run_num == 3|| run_num == 5)
			{
				resp_param_ctx.asyn_mode = 0;
			}
			else 
			{
				resp_param_ctx.asyn_mode = 1;
			}
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_ASYN, resp_param_ctx.asyn_mode);			/*配置ql_http_client_perform()函数执行的模式 0：同步；1：异步*/
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_RESPONSE_PARAM, (void*)&resp_param_ctx);
		}
		
		if(run_num == 0 || run_num == 1 || run_num == 2)
		{
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_REQUEST_METHOD, QL_HTTP_CLIENT_REQUEST_GET);
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_UPLOAD_FUNC, NULL);						/*GET的情况，是可以不需要配置upload相关的参数，但是为了 防止上一次http client 请求的配置影响这一次的请求*/
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_UPLOAD_PARAM, NULL);
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_UPLOAD_LEN, 0);
			if(resp_param_ctx.need_range == 1)
			{
				header=ql_http_client_list_append(header, "Range: 100-199\r\n");							/*配置  分段下载，但是该功能需要服务器支持*/
			}
		}

		if(run_num == 3 || run_num == 4 || run_num == 5 || run_num == 6)
		{
			test_http_app_upload_cb_count = 0;
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_REQUEST_METHOD, QL_HTTP_CLIENT_REQUEST_POST);
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_UPLOAD_FUNC, (void*)http_app_upload_cb);
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_UPLOAD_PARAM, (void*)upload_param);
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_UPLOAD_LEN, need_upload_len);
			if(run_num == 5 || run_num == 6)
				ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_TOTAL_TIMEOUT_MS, 2000);				/*配置 http client 超时功能， 完成整个http client 消耗的时间，开启客户端超时功能，超时时间:2s*/
		}

        header=ql_http_client_list_append(header, "Connection: keep-alive\r\n");							/*声请内存，用于保存自定义http请求头的信息，格式："key: value\r\n"， 配置  该次http连接为长连接*/
		if(header != NULL)
		{
			ret=ql_http_client_setopt(client, QL_HTTP_CLIENT_OPT_HTTPHEADER, header);						/*将拼接成功的自定会header 保存到http client 句柄中*/ 
		}

		ret=ql_http_client_perform(client);
		if(ret == QL_HTTP_CLIENT_ERR_SUCCESS)
		{
			if(resp_param_ctx.asyn_mode == 1  && resp_param_ctx.sem != NULL)
			{
				ql_rtos_semaphore_wait(resp_param_ctx.sem, QL_WAIT_FOREVER);
			}
		}

		printf("ql_http_client_perform ret= %d!\r\n",ret);
		
		printf("\r\n==============http_client_test[%d] end================\r\n",run_num++);

		ql_http_client_list_destroy(header);																/*释放用于保存自定义http请求头的信息内存*/
		header = NULL;
		
retry_wita:	
	
		ql_rtos_task_sleep_s(5);
		
	}
	
	ql_rtos_semaphore_delete(resp_param_ctx.sem);
	ql_http_client_release(client); /*release http resources*/
	return ;
}

static void ql_http_client_test_1(void *arg)
{	
	char ip4_addr_str[16] = {0};
	struct ql_data_call_info info = {0};

	if(datacall_satrt()!=0)
		return;

	ql_get_data_call_info(1, 0, &info);

	printf("info.profile_idx: %d\r\n", info.profile_idx);
	printf("info.ip_version: %d\r\n", info.ip_version);
	printf("info.v4.state: %d\r\n", info.v4.state);
	printf("info.v4.reconnect: %d\r\n", info.v4.reconnect);

	inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
	printf("info.v4.addr.ip: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
	printf("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
	printf("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);
	ql_log_mask_set(QL_LOG_APP_MASK,QL_LOG_PORT_UART);
	if(info.v4.state)
	{
		http_test_new_app();
	}
	return ;
}

//application_init(ql_http_client_test_1, "ql_http_client_test_1", 32, 2);

