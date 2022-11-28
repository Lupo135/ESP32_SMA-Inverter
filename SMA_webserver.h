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
char temp[1024];
snprintf(temp, 1024,"<html><head><meta http-equiv=\"refresh\" content=\"%d\"><body>\n\
<style type=\"text/css\">\n\
#customers {align:middle; border-collapse:collapse;}\n\
#customers td, #customers th {align:middle; text-align:center; font-size:30%; border:1px solid #98bf21; padding:3px 7px 2px 7px; }\n\
#customers tr:nth-child(odd) {background:#EAF2D3;} .vCenter {display:flex;justify-content:center;margin:5px;}\n\
</style> <table align=\"left\" id=\"customers\" vspace=5 style=\"margin-bottom:5px;\">\n\
<tr><td>Pac</td><td>%1.3f kW</td></tr>\n\
<tr><td>Udc</td><td>%1.1f V</td></tr>\n\
<tr><td>Idc</td><td>%1.3f A</td></tr>\n\
<tr><td>Uac</td><td>%1.1f V</td></tr>\n\
<tr><td>Iac</td><td>%1.3f A</td></tr>\n\
<tr><td>E-Today</td><td>%1.3f kWh</td></tr>\n\
<tr><td>E-Total</td><td>%1.3f kWh</td></tr>\n\
<tr><td>Frequency</td><td>%1.2f Hz</td></tr>\n\
<tr><td>Efficiency</td><td> %1.2f %%</td></tr>\n\
</table></body></html>"
, LOOPTIME_SEC
, tokW(pInvData->Pac)
, toVolt(pInvData->Udc)
, toAmp(pInvData->Idc)
, toVolt(pInvData->Uac)
, toAmp(pInvData->Iac)
, tokWh(pInvData->EToday)
, tokWh(pInvData->ETotal)
, toHz(pInvData->Freq)
, toPercent(pInvData->Eta)
);
  //server.send(200, "text/plain", temp);
  server.send(200, "text/html", temp);
}

