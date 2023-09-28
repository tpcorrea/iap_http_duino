# iap_http_duino

An In-Application Programming for updating the STM32 firmware using Ethernet connection and a Web browser.

### What is this repository for? ###

This is a library for In-Application Programming (IAP, update of uC firmware) using the Ethernet interface and HTTP. It adapts ST and Adams Dukels example to the Arduino framework. It was developed ans is meant to use with Platformio, but you should have no problem in adapting it STMCude or other SDK.

### How to use? ###

To use this library, it is necessary to create an object of the HttpIapServer class in your code. The function server() must be called when a new client is connected to the EthernetServer server, passing the client object to the function. Below an example on how the main.cpp file may look like:

main.cpp
~~~
#include <Arduino.h>
#include <STM32Ethernet.h>

#include "main.h"
#include "httpserver.h"

// Tasks
static void taskEthernetClient(void);

// Enter an IP address for your controller below.
// The IP address will be dependent on your local network:
IPAddress ip(192, 168, 7, 200);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
HttpIapServer iapServer;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

//-----------------------------------------------------------------------------
void loop() 
{
  taskEthernetClient();
}
__RAM_FUNC
//------------------- TASKS ---------------------------------------------------
static void __RAM_FUNC taskEthernetClient(void)
{

  char data[1500];
  int32_t len;

  // listen for incoming clients
  EthernetClient client = server.available();


  if (client) {
    Serial.println("new client");

    /* Allocate memory for the structure that holds the state of the connection */
    // an http request ends with a blank line
    while (client.connected()) {
      if (client.available()) {
        iapServer.server(&client);
      }
    }

    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
~~~

In this way, the STM32 will behave as HTTP server, and you can use your web browser to make login (default user/password: user/stm32) and upload the new firmware. One of the tricky things about the In-Application Programming is that it is necessary to put all code necessary for its execution outside the main .text memory. This was achieved as follows:

+ Flash divided in at least two sections:
    - FLASH_SECTOR0: first section, where the ISR vector table and IAP code will be allocated
    - FLASH: where the rest of the code will stay
+ After the .isr_sector, we will allocate the .iap_flash
    - All necessary functions that cannot be erased while programming must be located at the .iap_sector
    - This can be accomplished function wise with the MACRO __IAP_FUNC (put just before the function name)
+ Then, we need to put here also the HAL library files that are necessary to perform the FLASH programming:
    - Note that it would be unwise to include the __IAP_FUNC in all HAL functions, hence we must put all functions present in the files in the .iap_sector:
        - stm32yyxx_hal_flash_ex.c
        - stm32yyxx_hal_flash.c
        - stm32yyxx_hal.c
+ Next, the .text can be allocated to the FLASH sector
    - Obs.: we need to exclude the HAL files from the .text and .text* region

### Restrictions/ known issues: ###

The code uses the rest of the unused flashto temporarily store the incoming data before erasing the FLASH sector and copying the new version. I know this is sub-optimal, but it is a way of doing it in the meantime. In this sense, you must restrict the FLASH sector to the minimum, otherwise the .bin file will become too large (it has also data for not used memory).

Another issue is that, this far, you cannot update the .iap_sector and we might get link between .text and .iap_sector broken when updating the firmware.  

### Dependencies ###
    * Arduino framework (STM32duino)
    * STM32Ethernet
    * STM32duino LwIP

### Authorship ###

As stated, the initial code was developed by Adam Dunkels and the ST team, so most of the hard lifing is on them. Nevertheless, I wanted the IAP server to work with Arduino framework and, in this process, I found that a few tricks were left behind by (or were unimportant to) the original code. I hope a few find it useful. 