//
//  test_http.h
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 19/12/21.
//

#ifndef test_http_h
#define test_http_h

#include <stdio.h>

//
//Client consuption
//

#define NB_PROXY_NET_READ_BUFF_SZ			4096	//Cannot be zero
#define NB_PROXY_HTTP_BODY_READ_BUFF_SZ		4096	//0 = ignore body data (only parse).
#define NB_PROXY_HTTP_BODY_STORG_BUFF_SZ	0		//0 = do not save http-body-chunks (just parse).


#endif /* test_http_h */
