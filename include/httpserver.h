/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * http.h
 *                     
 * Author : Adam Dunkels <adam@sics.se>, modified by Tomas P. Correa                               
 *
 * CHANGELOG: this file has been modified by Sergio Perez Alca�iz <serpeal@upvnet.upv.es> 
 *            Departamento de Inform�tica de Sistemas y Computadores          
 *            Universidad Polit�cnica de Valencia                             
 *            Valencia (Spain)    
 *            Date: March 2003                                          
 *
 */

#ifndef __HTTPD_H__
#define __HTTPD_H__

// #ifdef __cplusplus
//  extern "C" {
// #endif

#include "STM32Ethernet.h"
#include "lwip/def.h"
#include "fsdata.h"
#include "iapconfig.h"

typedef struct 
{
  char *file;
  u32_t left;
} http_state;

typedef struct {
  char *data;
  int len;
} fs_file;

typedef enum
{
  LoginPage = 0,
  FileUploadPage,
  UploadDonePage,
  ResetDonePage
} htmlpageState;

typedef enum {
  DATA = 0,
  GET_HTTP = 1,
  GET_RESETMCU,
  GET_FAVICON,
  POST_CHECKLOGIN,
  POST_UPLOAD,
  OCTET_STREAM
} ClientRequestType;

class HttpIapServer
{
private:
  EthernetClient *_client;
  http_state *hs;
  htmlpageState _htmlpage;
  __IO uint32_t _dataFlag;
  __IO uint32_t _size;
  __IO uint32_t _flashWriteAddress;
  uint32_t _totalReceived;
  char _leftBytesTab[4];
  uint8_t _leftBytes;
  __IO uint8_t _resetPage;
  uint32_t _contentLengthOffset;

  int openFs(char const *name, fs_file *file);
  void sendData();
  uint32_t parseContentLength(const char *data, uint32_t len);
  void writeToFlash(const char *data, uint32_t len);
  int32_t readPacket(char data[], const int maxSize);
  ClientRequestType getType(char* data, uint32_t len);
  int32_t getFilename(char* data, uint32_t len);
  int32_t verifyData();
  int32_t updateFirmware();

public:
  HttpIapServer(/* args */);
  ~HttpIapServer();
  err_enum_t server(EthernetClient *client);
};


// #ifdef __cplusplus
// }
// #endif

#endif /* __HTTPD_H__ */
