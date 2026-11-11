#ifndef _HTTP_APP_H_
#define _HTTP_APP_H_

/* Creates the "QhttpApp" task: periodically POSTs device info to the FOTA server
 * and acts on NEWFILEPATH / FOTA keys in the reply. Call once from a launcher. */
void ql_http_init(void);

#endif /* _HTTP_APP_H_ */