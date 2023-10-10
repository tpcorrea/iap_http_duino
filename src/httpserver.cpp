/* Copyright (c) 2001, Swedish Institute of Computer Science.
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
 * httpd.c
 *
 * Author : Adam Dunkels <adam@sics.se>, adapted to Arduino framework by Tomas P. Correa
 *
 */

#include <Arduino.h>
#include <string.h>
#include <stdio.h>
// #include <STM32FreeRTOS.h>

#include "httpserver.h"
#include "lwip/tcp.h"
#include "fsdata.c"
#include "flash_if.h"

#include "debug.h"

#ifdef USE_IAP_HTTP

static const char http_crnl_2[4] =
    /* "\r\n--" */
    {0xd, 0xa, 0x2d, 0x2d};
static const char octet_stream[14] =
    /* "octet-stream" */
    {
        0x6f,
        0x63,
        0x74,
        0x65,
        0x74,
        0x2d,
        0x73,
        0x74,
        0x72,
        0x65,
        0x61,
        0x6d,
        0x0d,
};
static const char Content_Length[17] =
    /* Content Length */
    {
        0x43,
        0x6f,
        0x6e,
        0x74,
        0x65,
        0x6e,
        0x74,
        0x2d,
        0x4c,
        0x65,
        0x6e,
        0x67,
        0x74,
        0x68,
        0x3a,
        0x20,
};

const struct fsdata_file file_index_html[] = {{NULL, data_index_html, data_index_html + 12, sizeof(data_index_html) - 12}};
const struct fsdata_file file_reset_html[] = {{file_index_html, data_reset_html, data_reset_html + 12, sizeof(data_reset_html) - 12}};
const struct fsdata_file file_upload_html[] = {{file_reset_html, data_upload_html, data_upload_html + 13, sizeof(data_upload_html) - 13}};
const struct fsdata_file file_uploaddone_html[] = {{file_upload_html, data_uploaddone_html, data_uploaddone_html + 17, sizeof(data_uploaddone_html) - 17}};
#define FS_ROOT file_uploaddone_html
#define FS_NUMFILES 4

HttpIapServer::HttpIapServer() : _dataFlag(0), _size(0), _totalReceived(0), _leftBytes(0), _resetPage(0),
                                 _contentLengthOffset(0), _client(nullptr), _htmlpage(htmlpageState::LoginPage)
{
}

HttpIapServer::~HttpIapServer()
{
}

/**
 * @brief parse data to define what type of request the client has sent
 * @param  data : pointer on receive packet buffer
 * @param  len  : buffer length
 * @retval type of request as a ClientRequestType value
 */
ClientRequestType HttpIapServer::getType(char *data, uint32_t len)
{
  int32_t i = 0;

  if (strncmp(data, "GET / HTTP", 10) == 0)
  {
     DEBUG_MESSAGE("# GET / HTTP\n\r");
    return ClientRequestType::GET_HTTP;
  }
  else if (strncmp(data, "GET /resetmcu.cgi", 17) == 0)
  {
     DEBUG_MESSAGE("# GET /resetmcu.cgi\n\r");
    return ClientRequestType::GET_RESETMCU;
  }
  else if (strncmp(data, "GET /favicon.ico", 16) == 0)
  {
     DEBUG_MESSAGE("# GET /favicon.ico\n\r");
    return ClientRequestType::GET_FAVICON;
  }
  else if (strncmp(data, "POST /checklogin.cgi", 20) == 0)
  {
     DEBUG_MESSAGE("# POST /checklogin.cgi\n\r");
    return ClientRequestType::POST_CHECKLOGIN;
  }
  else if (strncmp(data, "POST /upload.cgi", 16) == 0)
  {
     DEBUG_MESSAGE("# POST /upload.cgi\n\r");
    return ClientRequestType::POST_UPLOAD;
  }
  else
  {
    /* File being uploaded might have an OCTET_STREAM, so we need to parse for it
    only if not receiving firmware data*/
    if (_dataFlag == 0)
    {
      for (i = 0; i < len; i++)
      {
        if (strncmp((char *)(data + i), octet_stream, 13) == 0)
        {
          return ClientRequestType::OCTET_STREAM;
          break;
        }
      }
    }
    return ClientRequestType::DATA;
  }
}

/**
 * @brief parse data to find filename
 * @param  data : pointer on receive packet buffer
 * @param  len  : buffer length
 * @retval not sure yet
 */
int32_t HttpIapServer::getFilename(char *data, uint32_t len)
{
  uint32_t DataOffset, FilenameOffset;
  char filename[40];
  int32_t i = 0;

  FilenameOffset = 0;
  for (i = 0; i < len; i++)
  {
    if (strncmp((char *)(data + i), "filename=", 9) == 0)
    {
      FilenameOffset = i + 10;
      break;
    }
  }
  i = 0;
  if (FilenameOffset)
  {
    while ((*(data + FilenameOffset + i) != 0x22) && (i < 40))
    {
      filename[i] = *(data + FilenameOffset + i);
      i++;
    }
    filename[i] = 0x0;
  }

  return i;
}

/**
 * @brief sends data found in member "file" of a http_state struct
 * @retval None
 */
void HttpIapServer::sendData()
{
  err_t err;
  u16_t len;

  len = hs->left;
  if (_client->write(hs->file, len) == len)
  {
    hs->file += len;
    hs->left -= len;
  }
}

/**
 * @brief function for handling TCP HTTP traffic
 * @param client: pointer to EthernetClient object
 * @retval err
 */
err_enum_t __RAM_FUNC HttpIapServer::server(EthernetClient *client)
{
  int32_t i;
  int32_t len;
  char *ptr, login[LOGIN_SIZE + 1];
  fs_file file = {0, 0};

  char data[1460];

  hs = new http_state;
  hs->file = NULL;
  hs->left = 0;

  _client = client;

  while (!_client->available())
  {
  }
  
  if (_dataFlag == 0)
  {
    len = readPacket(data, sizeof(data));
  }
  else
  {
    len = _client->read((uint8_t*)data, sizeof(data));
  }
  DEBUG_MESSAGE("# Received %d bytes\n\r", len);

  switch (getType(data, len))
  {
  case ClientRequestType::GET_HTTP:
    /*send the login page (which is the index page) */
    _htmlpage = htmlpageState::LoginPage;
    openFs("/index.html", &file);
    hs->file = file.data;
    hs->left = file.len;

    // /* send index.html page */
    sendData();
    _client->stop();
    break;

  case ClientRequestType::GET_RESETMCU:
    _htmlpage = htmlpageState::ResetDonePage;
    openFs("/reset.html", &file);
    hs->file = file.data;
    hs->left = file.len;
    // /* send reset.html page */
    sendData();
    _client->stop();
    /* Generate a software reset */
    if(1 == _resetPage){NVIC_SystemReset();}

    break;

  case ClientRequestType::GET_FAVICON:
    _client->stop();
    break;

  case ClientRequestType::POST_CHECKLOGIN:
    if (_htmlpage == htmlpageState::LoginPage)
    {
      len = readPacket(data, sizeof(data));
      /* parse packet for the username & password */
      for (i = 0; i < len; i++)
      {
        if (strncmp((char *)(data + i), "username=", 9) == 0)
        {
          sprintf((char *)login, "username=%s&password=%s", USERID, PASSWORD);
          if (strncmp((char *)(data + i), (char *)login, LOGIN_SIZE) == 0)
          {
            _htmlpage = htmlpageState::FileUploadPage;
            openFs("/upload.html", &file);
          }
          else
          {
            _htmlpage = htmlpageState::LoginPage;
            /* reload index.html */
            openFs("/index.html", &file);
          }
          hs->file = file.data;
          hs->left = file.len;

          //  /* send index.html page */
          sendData();
          _client->stop();
          break;
        }
      }
    }
    break;

  case ClientRequestType::POST_UPLOAD:
    // if (((strncmp(data, "POST /upload.cgi", 16) == 0) || (_dataFlag >= 1)) && (_htmlpage == htmlpageState::FileUploadPage))
    if (_htmlpage == htmlpageState::FileUploadPage)
    {
      /* POST Packet received */
      _totalReceived = 0;

      /* parse packet for Content-length field */
      _size = parseContentLength(data, len);

      /* case of MSIE8 : we do not receive data in the POST packet */
      return ERR_OK;
    }
    break;

  case ClientRequestType::OCTET_STREAM:
    /* First time it received the OCTET STREAM flag*/
    if (_dataFlag == 0)
    {
      /* no filename, in this case reload upload page */
      if (0 == getFilename(data, len))
      {
        _htmlpage = htmlpageState::FileUploadPage;

        openFs("/upload.html", &file);
        hs->file = file.data;
        hs->left = file.len;

        /* send index.html page */
        sendData();
        _dataFlag = 0;
        return ERR_OK;
      }
      _dataFlag++;

      _totalReceived += len + 1;

      /* init flash */
      FLASH_If_Init();
      /* erase temporary flash area */
      noInterrupts();
      while(FLASH_If_Erase(TEMP_PROG_BEGIN_SECTOR, TEMP_PROG_END_SECTOR)) {}
      interrupts();

      _flashWriteAddress = TEMP_PROG_BEGIN_ADDRESS;
    }
    else
    {
    }

    break;

  case ClientRequestType::DATA:
    /* code */
    _totalReceived += len;
    DEBUG_MESSAGE("# Total data %d, size %d\n\r", _totalReceived, _size);

    if (_totalReceived == _size)
    {
      /* if last packet need to remove the http boundary tag */
      /* parse packet for "\r\n--" starting from end of data */
      i = 4;
      while (strncmp((char *)(data + len - i), http_crnl_2, 4) && (len - i > 0))
      {
        i++;
      }
      len -= i;

      /* write data in Flash */
      if (len)
        writeToFlash(data, len);

      _dataFlag = 0;
      DEBUG_MESSAGE("# Writing to Flash\n\r");
      updateFirmware();

      _htmlpage = htmlpageState::UploadDonePage;
      /* send uploaddone.html page */
      openFs("/uploaddone.html", &file);
      hs->file = file.data;
      hs->left = file.len;
      sendData();
      _client->stop();
    }
    else
    {
      writeToFlash(data, len);
    }

  default:
    break;
  }

  delete hs;
  return ERR_OK;
}

/**
 * @brief  Opens a file defined in fsdata.c ROM filesystem
 * @param  name : pointer to a file name
 * @param  file : pointer to a fs_file structure
 * @retval  1 if success, 0 if fail
 */
int HttpIapServer::openFs(char const *name, fs_file *file)
{
  struct fsdata_file_noconst *f;

  for (f = (struct fsdata_file_noconst *)FS_ROOT; f != NULL; f = (struct fsdata_file_noconst *)f->next)
  {
    if (!strcmp(name, f->name))
    {
      file->data = f->data;
      file->len = f->len;
      return 1;
    }
  }
  return 0;
}

/**
 * @brief  Extract the Content_Length data from HTML data
 * @param  data : pointer on receive packet buffer
 * @param  len  : buffer length
 * @retval size : content_length in numeric format
 */
uint32_t HttpIapServer::parseContentLength(const char *data, uint32_t len)
{
  uint32_t i = 0, size = 0, S = 1;
  int32_t j = 0;
  char sizestring[6], *ptr;

  _contentLengthOffset = 0;

  /* find Content-Length data in packet buffer */
  for (i = 0; i < len; i++)
  {
    if (strncmp((char *)(data + i), Content_Length, 16) == 0)
    {
      _contentLengthOffset = i + 16;
      break;
    }
  }
  /* read Content-Length value */
  if (_contentLengthOffset)
  {
    i = 0;
    ptr = (char *)(data + _contentLengthOffset);
    while (*(ptr + i) != 0x0d)
    {
      sizestring[i] = *(ptr + i);
      i++;
      _contentLengthOffset++;
    }
    if (i > 0)
    {
      /* transform string data into numeric format */
      for (j = i - 1; j >= 0; j--)
      {
        size += (sizestring[j] - '0') * S;
        S = S * 10;
      }
    }
  }
  return size;
}

/**
 * @brief  writes received data in flash
 * @param  data : pointer to data to be written in Flash
 * @param  len  : buffer length
 * @retval None
 */
void __IAP_FUNC HttpIapServer::writeToFlash(const char *data, uint32_t len)
{
  uint32_t count, i = 0, j = 0;

  /* check if any left bytes from previous packet transfer*/
  /* if it is the case do a concat with new data to create a 32-bit word */
  if (_leftBytes)
  {
    while (_leftBytes <= 3)
    {
      if (len > (j + 1))
      {
        _leftBytesTab[_leftBytes++] = *(data + j);
      }
      else
      {
        _leftBytesTab[_leftBytes++] = 0xFF;
      }
      j++;
    }

    if(FLASH_If_Write(&_flashWriteAddress, (uint32_t *)(_leftBytesTab), 1))
    {
      DEBUG_MESSAGE("# Flash ERROR.\n\r");
    }
    _leftBytes = 0;

    /* update data pointer */
    data = (char *)(data + j);
    len = len - j;
  }

  /* write received bytes into flash */
  count = len / 4;

  /* check if remaining bytes < 4 */
  i = len % 4;
  if (i > 0)
  {
    if (_totalReceived != _size)
    {
      /* store bytes in _leftBytesTab */
      _leftBytes = 0;
      for (; i > 0; i--)
        _leftBytesTab[_leftBytes++] = *(char *)(data + len - i);
    }
    else
      count++;
  }
  
  if(FLASH_If_Write(&_flashWriteAddress, (uint32_t *)data, count))
  {
    DEBUG_MESSAGE("# Flash ERROR.\n\r");
  }
}
#endif

/**
 * @brief  gets incoming data till first blank line 
 * @param  data : pointer on receive packet buffer
 * @param  maxLength  : buffer length (maximum length)
 * @retval size of data read
 */

int32_t HttpIapServer::readPacket(char data[], const int maxLength)
{
  char c;
  int32_t len = 0;
  // an http request ends with a blank line
  bool currentLineIsBlank = false;

  c = _client->read();
  while (c != -1)
  {
    if (c == '\n') // you're starting a new line
    {
      currentLineIsBlank = true;
    }
    else if (c != '\r') // you've gotten a character on the current line
    {
      currentLineIsBlank = false;
    }

    data[len] = c;
    len++;

    if (maxLength <= len || (!_client->available()))
      break;

    c = _client->read();

    if ((c == '\n') && currentLineIsBlank)
      break;
  }
  return len;
}

/**
 * @brief  copy data from temporary FLASH to user FLASH 
 * @retval none
 */
int32_t __IAP_FUNC HttpIapServer::updateFirmware()
{
  char* data = (char*)(TEMP_PROG_BEGIN_ADDRESS + IAP_SECTOR_SIZE); // Address of first data

  /* enter critical code area*/
  noInterrupts();
  /* init flash */
  FLASH_If_Init();
  /* erase user flash area */
  while(FLASH_If_Erase(USER_PROG_BEGIN_SECTOR, USER_PROG_END_SECTOR)) {};
 
  _flashWriteAddress = USER_PROG_BEGIN_ADDRESS; // destination address
  _leftBytes = 0; 
  writeToFlash(data, _totalReceived -  IAP_SECTOR_SIZE); 
  _resetPage = 1; 
  /* exit critical code area*/
  interrupts();

  return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
