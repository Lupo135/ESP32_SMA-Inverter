/* MIT License

Copyright (c) 2022 Lupo135

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <esp_wifi.h> 
#include <WebServer.h>

WebServer server(80);

bool setupWebserver() {
  Serial.printf("Connecting to %s ", ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int i=0;
  for (i=0; i<101; i++) {
    if (WiFi.status() == WL_CONNECTED) 
      break;
    delay(100);
    Serial.print(".");
  }
  
  if (i>=100) { // without connection go to sleep 
    Serial.println("No Connection!!");
    WiFi.disconnect(true);
    delay(1000);
    return false;
  }
  Serial.print(" Connected to IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

void SMA_ServeAll() {
  char temp[256];
  snprintf(temp, 256,"\
Pac     %15.3f kW\n\
Uac     %15.3f V\n\
Iac     %15.3f A\n\
Udc     %15.3f V\n\
Idc     %15.3f A\n\
E-Today %15.3f kWh\n\
E-Total %15.3f kWh\n\
Efficiency %11.2f %%\n"
, tokW(pInvData->Pac)
, toVolt(pInvData->Uac)
, toAmp(pInvData->Iac)
, toVolt(pInvData->Udc)
, toAmp(pInvData->Idc)
, tokWh(pInvData->EToday)
, tokWh(pInvData->ETotal)
, toPercent(pInvData->Eta)
);
  server.send(200, "text/plain", temp);
}

