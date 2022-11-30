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

void SmaDayGraph();
void SmaTable();

WebServer server(80);
#define HTTP_BUF_MAX 2048
char httpBuf[HTTP_BUF_MAX];
int length = 0;

// ****** Setup ***
bool setupWebserver() {
  Serial.printf("Connecting to %s ", ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int i;
  for (i=0; i<51; i++) {
    if (WiFi.status() == WL_CONNECTED) 
      break;
    if (i==20) WiFi.begin(ssid, password);
    delay(500);
    Serial.print(".");
  }
  
  if (i>=50) { // without connection go to sleep 
    Serial.println("No Connection!!");
    WiFi.disconnect(true);
    delay(1000);
    return false;
  }
  Serial.print(" Connected to IP: ");
  Serial.println(WiFi.localIP());
  server.on("/", SmaTable);
  server.on("/table.htm", SmaTable);
  server.on("/graphday.htm", SmaDayGraph);
  server.begin();
  DEBUG1_PRINT("WebServer started");
  return true;
}

// *** Html Table ***
void SmaTable() {
uint32_t reloadSec;
length=0;
if (btConnected) reloadSec = (((nextTime-millis())/1000)+1); // wait until new values
else             reloadSec = 60; 
if (reloadSec>120) reloadSec = 10;

length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
      "<html><head><meta http-equiv=\"refresh\" content=\"%d\"><body>\n\
<style type=\"text/css\">\n\
#dT {align:middle; border-collapse:collapse;}\n\
#dT td, #dT th {align:middle; text-align:center; font-size:30%; border:1px solid #98bf21; padding:3px 7px 2px 7px; }\n\
#dT tr:nth-child(odd) {background:#EAF2D3;} .vCenter {display:flex;justify-content:center;margin:5px;}\n\
progress {width:25rem;height:0.5rem;text-align:right;} progress:after{content: attr(value);} </style>\n\
<progress value=\"%d\" max=\"%d\" id=\"pB\"></progress><br>\n\
<table align=\"left\" id=\"dT\" vspace=5 style=\"margin-bottom:5px;\">\n", reloadSec, reloadSec-1, reloadSec-1);

if (btConnected) {
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
"<tr><td>Pac</td><td>%1.3f kW</td></tr>\n\
 <tr><td>Udc</td><td>%1.1f V</td></tr>\n\
 <tr><td>Idc</td><td>%1.3f A</td></tr>\n\
 <tr><td>Uac</td><td>%1.1f V</td></tr>\n\
 <tr><td>Iac</td><td>%1.3f A</td></tr>\n"
 , tokW(pInvData->Pac)
 , toVolt(pInvData->Udc)
 , toAmp(pInvData->Idc)
 , toVolt(pInvData->Uac)
 , toAmp(pInvData->Iac));
} else {
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
        "<tr><td>Bluetooth</td><td>offline</td></tr>\n");
}
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
"<tr><td>E-Today</td><td>%1.3f kWh</td></tr>\n\
 <tr><td>E-Total</td><td>%1.3f kWh</td></tr>\n"
 , tokWh(pInvData->EToday)
 , tokWh(pInvData->ETotal));

if (btConnected)
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
"<tr><td>Frequency</td><td>%1.2f Hz</td></tr>\n\
 <tr><td>Efficiency</td><td> %1.2f %%</td></tr>\n"
  , toHz(pInvData->Freq)
  , toPercent(pInvData->Eta));

  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, "<tr><td>Last-GMT</td><td>");
  length += printUnixTime(httpBuf+length, pInvData->LastTime);
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, "</td></tr></table>\n");

  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
"<br><button onclick=\"window.location.href='graphday.htm?day=0'\">DayGraph</button>\n");

  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
"<script>var tm=%d;var dTimer=setInterval(function(){\n\
if(tm<=0){clearInterval(dTimer);}\n\
document.getElementById(\"pB\").value=tm;\n\ 
tm -= 1;},1000); </script></body></html>", reloadSec);

  server.send(200, "text/html", httpBuf);
}

// *** html day graph ***
void SmaDayGraph () { 
  int offsetDay = 0;
  if (server.hasArg("day")) offsetDay = server.arg("day").toInt();
  time_t day = pInvData->LastTime-24*60*60*offsetDay;
  printUnixTime(charBuf, day); DEBUG2_PRINTF("\nSmaDayGraph offset=%d %s", offsetDay, charBuf);

  length=0;

  if (btConnected) {
  if ((ArchiveDayData(day)) == E_OK) {

  bool hasData = false;
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
"<!DOCTYPE html><html><script src=\"https://cdn.plot.ly/plotly-latest.min.js\"></script><body>\n\
<button onclick=\"window.location.href='table.htm'\">Table</button>\n\
<button onclick='prevDay()'>Day-</button>\n\
<button onclick='nextDay()'>Day+</button>\n\
<div id='myPlot'></div><script>var offs=%d;\n\
function nextDay(){if (offs>0) {offs--; window.location='graphday.htm?day=' + offs;}}\n\
function prevDay(){offs++; window.location='graphday.htm?day=' + offs;}\n\
var xa=[];var d=new Date(\"%u\"*1000);\n",
  offsetDay, pInvData->DayStartTime);
  // power values
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, "var yp=[");
  int32_t currW;
  uint16_t firstWh=0;
  uint16_t lastWh=0;
  for (uint16_t i=1; i<ARCH_DAY_SIZE; i++) {
    if (pInvData->dayWh[i]>0) {
       lastWh=i;
       if (hasData) {
         currW = (int32_t)((pInvData->dayWh[i] - pInvData->dayWh[i-1])*60/5); 
         if (currW<0) currW=0;
         length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, "%d,", currW);
       } else {
          firstWh=i;
       }
       hasData = true;
    }
  }
  DEBUG2_PRINTF("\nfirst=%u=%llu last=%u=%llu d=%llu", firstWh, pInvData->dayWh[firstWh], lastWh, pInvData->dayWh[lastWh], pInvData->dayWh[lastWh]-pInvData->dayWh[firstWh]);
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, "];\n");
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, "const dayWh=%lld;", pInvData->dayWh[lastWh]-pInvData->dayWh[firstWh]);

  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, 
"const weekday=['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];\n\
var tTx=dayWh/1000+' kWh '+weekday[d.getDay()]+' '+d.getDate().toString()+'.'+(d.getMonth()+1).toString()+'.'+(d.getYear()-100).toString();\n\ 
function makeRange(y,delta){var x=[];\n\
for(i=0; i<y.length; i++){x.push(d.getTime());d.setTime(d.getTime()+delta*60000);}\n\
return x;}\n\
var xa=makeRange(yp,5);\n\
var tp={y:yp,x:xa,name:'P',line:{color:'green',width:2}};\n");

  // %a=weekday; %e=day; %b=nonth(Dez); %H=hour
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length,
"var data=[tp];\
var layout={showlegend:false,xaxis:{type:'date',tickformat:'%%H:%%M',hoverformat:'%%H:%%M'},\n\
yaxis:{showgrid:true,ticksuffix:'W'},title:(tTx)}\n\
Plotly.plot('myPlot',data,layout);\
</script></body></html>");

  } else {  // Arch data
    length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, "<!DOCTYPE html><html>Day data error !!</html>");
  }
} else { // bt
  length += snprintf(httpBuf+length, HTTP_BUF_MAX-length, "<!DOCTYPE html><html>BT not available !!</html>");
}
  server.sendHeader("Cache-Control", "no-cache");
  server.setContentLength(length);
  server.send(200, "text/html", httpBuf);
}

