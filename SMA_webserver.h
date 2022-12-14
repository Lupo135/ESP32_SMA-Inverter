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
  charLen=0;
  if (ReadCurrentData()==E_OK) {

    charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen,
      "<html><head><meta http-equiv=\"refresh\" content=\"%d\"><body>\n\
<style type=\"text/css\">\n\
#dT {align:middle; border-collapse:collapse;}\n\
#dT td, #dT th {align:middle; text-align:center; font-size:30%; border:1px solid #98bf21; padding:3px 7px 2px 7px; }\n\
#dT tr:nth-child(odd) {background:#EAF2D3;} .vCenter {display:flex;justify-content:center;margin:5px;}\n\
progress {width:25rem;height:0.5rem;text-align:right;} progress:after{content: attr(value);} </style>\n\
<progress value=\"%d\" max=\"%d\" id=\"pB\"></progress><br>\n\
<table align=\"left\" id=\"dT\" vspace=5 style=\"margin-bottom:5px;\">\n", LOOPTIME_SEC, LOOPTIME_SEC, LOOPTIME_SEC);

  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen,
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
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen,
"<tr><td>E-Today</td><td>%1.3f kWh</td></tr>\n\
 <tr><td>E-Total</td><td>%1.3f kWh</td></tr>\n"
 , tokWh(pInvData->EToday)
 , tokWh(pInvData->ETotal));

if (btConnected)
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen,
"<tr><td>Frequency</td><td>%1.2f Hz</td></tr>\n\
 <tr><td>Efficiency</td><td> %1.2f %%</td></tr>\n"
  , toHz(pInvData->Freq)
  , toPercent(pInvData->Eta));

  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "<tr><td>Last-GMT</td><td>");
  charLen += printUnixTime(charBuf+charLen, pInvData->LastTime);
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "</td></tr></table>\n");

  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen,
"<br><button onclick=\"window.location.href='graphday.htm?day=0'\">DayGraph</button>\n");

  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen,
"<script>var tm=%d;var dTimer=setInterval(function(){\n\
if(tm<=0){clearInterval(dTimer);}\n\
document.getElementById(\"pB\").value=tm;\n\ 
tm -= 1;},1000); </script></body></html>", LOOPTIME_SEC);
  }
  server.send(200, "text/html", charBuf);
}

// *** html day graph ***
void SmaDayGraph () { 
  int offsetDay = 0;
  if (server.hasArg("day")) offsetDay = server.arg("day").toInt();
  time_t day = pInvData->LastTime-24*60*60*offsetDay;
  printUnixTime(timeBuf, day); DEBUG2_PRINTF("\nSmaDayGraph offset=%d %s", offsetDay, timeBuf);

  charLen=0;

  if (btConnected) {
  if ((ArchiveDayData(day)) == E_OK) {

  bool hasData = false;
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen,
"<!DOCTYPE html><html><script src=\"https://cdn.plot.ly/plotly-latest.min.js\"></script><body>\n\
<button onclick=\"window.location.href='table.htm'\">Table</button>\n\
<button onclick='prevDay()'>Day-</button>\n\
<button onclick='nextDay()'>Day+</button>\n\
<div id='myPlot'></div><script>var offs=%d;\n\
function nextDay(){if (offs>0) {offs--; window.location='graphday.htm?day=' + offs;}}\n\
function prevDay(){offs++; window.location='graphday.htm?day=' + offs;}\n\
var d=new Date(\"%u\"*1000);var x=[];\n",
  offsetDay, pInvData->DayStartTime);
  // power values
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "var yp=[");
  int32_t currW;
  uint16_t firstWh=0;
  uint16_t lastWh=0;
  for (uint16_t i=1; i<ARCH_DAY_SIZE; i++) {
    if (pInvData->dayWh[i]>0) {
       lastWh=i;
       if (hasData) {
         currW = (int32_t)((pInvData->dayWh[i] - pInvData->dayWh[i-1])*60/5); 
         if (currW<0) currW=0;
         charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "%d,", currW);
       } else {
          firstWh=i;
       }
       hasData = true;
    }
  }
  DEBUG2_PRINTF("\nfirst=%u=%llu last=%u=%llu d=%llu", firstWh, pInvData->dayWh[firstWh], lastWh, pInvData->dayWh[lastWh], pInvData->dayWh[lastWh]-pInvData->dayWh[firstWh]);
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "];\n");
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "const dayWh=%lld;", pInvData->dayWh[lastWh]-pInvData->dayWh[firstWh]);

  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, 
"const weekday=['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];\n\
var tTx=dayWh/1000+' kWh '+weekday[d.getDay()]+' '+d.getDate().toString()+\n\
'.'+(d.getMonth()+1).toString()+'.'+(d.getYear()-100).toString();\n\ 
function makeRange(y,delta){for(i=0; i<y.length; i++)\n\
{x.push(d.getTime());d.setTime(d.getTime()+delta*60000);} return x;}\n\
var xa=makeRange(yp,5);\n\
var tp={y:yp,x:xa,name:'P',line:{color:'green',width:2}};\n");

  // %a=weekday; %e=day; %b=nonth(Dez); %H=hour
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen,
"var data=[tp];\
var layout={showlegend:false,xaxis:{type:'date',tickformat:'%%H:%%M',hoverformat:'%%H:%%M'},\n\
yaxis:{showgrid:true,ticksuffix:'W'},title:(tTx)}\n\
Plotly.plot('myPlot',data,layout);\
</script></body></html>");

  } else {  // Arch data
    charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "<!DOCTYPE html><html>Day data error !!</html>");
    btConnected = false;
  }
} else { // bt
  charLen += snprintf(charBuf+charLen, CHAR_BUF_MAX-charLen, "<!DOCTYPE html><html>BT not available !!</html>");
}
  server.sendHeader("Cache-Control", "no-cache");
  server.setContentLength(charLen);
  server.send(200, "text/html", charBuf);
}

