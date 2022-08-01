/*
  SDWebServer - Example WebServer with SD Card backend for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Have a FAT Formatted SD Card connected to the SPI port of the ESP8266
  The web root is the SD Card root folder
  File extensions with more than 3 charecters are not supported by the SD Library
  File Names longer than 8 charecters will be truncated by the SD library, so keep filenames shorter
  index.htm is the default index (works on subfolders as well)

  upload the contents of SdRoot to the root of the SDcard and access the editor by going to http://esp8266sd.local/edit
  To retrieve the contents of SDcard, visit http://esp32sd.local/list?dir=/
      dir is the argument that needs to be passed to the function PrintDirectory via HTTP Get request.

*/

/*
Routine modification is based on Arduino-WebServer.
Add JPG image display.
The image size should be below 960 x 540
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <SPIFFS.h>
#include "epd_driver.h"
#include "libjpeg/libjpeg.h"

#define DBG_OUTPUT_PORT Serial

const char *ssid = "your-ssid";
const char *password = "your-password";
const char *host = "esp32sd";

WebServer server(80);

static bool hasSPIFFS = false;
File uploadFile;
EventGroupHandle_t handleServer;
String pic_path;
uint8_t *framebuffer;
#define BIT_CLEAN _BV(0)
#define BIT_SHOW _BV(1)

void returnOK()
{
    server.send(200, "text/plain", "");
}

void returnFail(String msg)
{
    server.send(500, "text/plain", msg + "\r\n");
}

bool loadFromSPIFFSCard(String path)
{
    String dataType = "text/plain";
    if (path.endsWith("/"))
    {
        path += "index.htm";
    }

    if (path.endsWith(".src"))
    {
        path = path.substring(0, path.lastIndexOf("."));
    }
    else if (path.endsWith(".htm"))
    {
        dataType = "text/html";
    }
    else if (path.endsWith(".css"))
    {
        dataType = "text/css";
    }
    else if (path.endsWith(".js"))
    {
        dataType = "application/javascript";
    }
    else if (path.endsWith(".png"))
    {
        dataType = "image/png";
    }
    else if (path.endsWith(".gif"))
    {
        dataType = "image/gif";
    }
    else if (path.endsWith(".jpg"))
    {
        dataType = "image/jpeg";
    }
    else if (path.endsWith(".ico"))
    {
        dataType = "image/x-icon";
    }
    else if (path.endsWith(".xml"))
    {
        dataType = "text/xml";
    }
    else if (path.endsWith(".pdf"))
    {
        dataType = "application/pdf";
    }
    else if (path.endsWith(".zip"))
    {
        dataType = "application/zip";
    }

    File dataFile = SPIFFS.open(path.c_str());
    if (dataFile.isDirectory())
    {
        path += "/index.htm";
        dataType = "text/html";
        dataFile = SPIFFS.open(path.c_str());
    }

    if (!dataFile)
    {
        return false;
    }

    if (server.hasArg("download"))
    {
        dataType = "application/octet-stream";
    }

    if (server.streamFile(dataFile, dataType) != dataFile.size())
    {
        DBG_OUTPUT_PORT.println("Sent less data than expected!");
    }

    dataFile.close();
    return true;
}

void handleFileUpload()
{
    if (server.uri() != "/edit")
    {
        return;
    }
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        if (SPIFFS.exists((char *)upload.filename.c_str()))
        {
            SPIFFS.remove((char *)upload.filename.c_str());
        }
        uploadFile = SPIFFS.open(upload.filename.c_str(), FILE_WRITE);
        DBG_OUTPUT_PORT.print("Upload: START, filename: ");
        DBG_OUTPUT_PORT.println(upload.filename);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (uploadFile)
        {
            uploadFile.write(upload.buf, upload.currentSize);
        }
        DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: ");
        DBG_OUTPUT_PORT.println(upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (uploadFile)
        {
            uploadFile.close();
        }
        DBG_OUTPUT_PORT.print("Upload: END, Size: ");
        DBG_OUTPUT_PORT.println(upload.totalSize);
    }
}

void deleteRecursive(String path)
{
    File file = SPIFFS.open((char *)path.c_str());
    if (!file.isDirectory())
    {
        file.close();
        SPIFFS.remove((char *)path.c_str());
        return;
    }

    file.rewindDirectory();
    while (true)
    {
        File entry = file.openNextFile();
        if (!entry)
        {
            break;
        }
        String entryPath = path + "/" + entry.name();
        if (entry.isDirectory())
        {
            entry.close();
            deleteRecursive(entryPath);
        }
        else
        {
            entry.close();
            SPIFFS.remove((char *)entryPath.c_str());
        }
        yield();
    }

    SPIFFS.rmdir((char *)path.c_str());
    file.close();
}

void handleDelete()
{
    if (server.args() == 0)
    {
        return returnFail("BAD ARGS");
    }
    String path = server.arg(0);
    if (path == "/" || !SPIFFS.exists((char *)path.c_str()))
    {
        returnFail("BAD PATH");
        return;
    }
    deleteRecursive(path);
    returnOK();
}

void handleCreate()
{
    if (server.args() == 0)
    {
        return returnFail("BAD ARGS");
    }
    String path = server.arg(0);
    if (path == "/" || SPIFFS.exists((char *)path.c_str()))
    {
        returnFail("BAD PATH");
        return;
    }

    if (path.indexOf('.') > 0)
    {
        File file = SPIFFS.open((char *)path.c_str(), FILE_WRITE);
        if (file)
        {
            file.write(0);
            file.close();
        }
    }
    else
    {
        SPIFFS.mkdir((char *)path.c_str());
    }
    returnOK();
}

void handleGetPath()
{
    if (server.args() == 0)
    {
        return returnFail("BAD ARGS");
    }
    pic_path = server.arg(0);
    Serial.print("get pic path : ");
    Serial.println(pic_path.c_str());
    xEventGroupSetBits(handleServer, BIT_SHOW);
    returnOK();
}

void printDirectory()
{
    if (!server.hasArg("dir"))
    {
        return returnFail("BAD ARGS");
    }
    String path = server.arg("dir");
    if (path != "/" && !SPIFFS.exists((char *)path.c_str()))
    {
        return returnFail("BAD PATH");
    }
    File dir = SPIFFS.open((char *)path.c_str());
    path = String();
    if (!dir.isDirectory())
    {
        dir.close();
        return returnFail("NOT DIR");
    }
    dir.rewindDirectory();
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/json", "");
    WiFiClient client = server.client();

    server.sendContent("[");
    for (int cnt = 0; true; ++cnt)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            break;
        }

        String output;
        if (cnt > 0)
        {
            output = ',';
        }

        output += "{\"type\":\"";
        output += (entry.isDirectory()) ? "dir" : "file";
        output += "\",\"name\":\"";
        output += entry.path();
        output += "\"";
        output += "}";
        server.sendContent(output);
        entry.close();
    }
    server.sendContent("]");
    dir.close();
}

void handleNotFound()
{
    if (hasSPIFFS && loadFromSPIFFSCard(server.uri()))
    {
        return;
    }
    String message = "SPIFFSCARD Not Detected\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    DBG_OUTPUT_PORT.print(message);
}

void setup(void)
{
    handleServer = xEventGroupCreate();
    DBG_OUTPUT_PORT.begin(115200);
    DBG_OUTPUT_PORT.setDebugOutput(true);
    DBG_OUTPUT_PORT.print("\n");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    DBG_OUTPUT_PORT.print("Connecting to ");
    DBG_OUTPUT_PORT.println(ssid);

    // Wait for connection
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20)
    { // wait 10 seconds
        delay(500);
    }
    if (i == 21)
    {
        DBG_OUTPUT_PORT.print("Could not connect to");
        DBG_OUTPUT_PORT.println(ssid);
        while (1)
        {
            delay(500);
        }
    }
    DBG_OUTPUT_PORT.print("Connected! IP address: ");
    DBG_OUTPUT_PORT.println(WiFi.localIP());

    if (MDNS.begin(host))
    {
        MDNS.addService("http", "tcp", 80);
        DBG_OUTPUT_PORT.println("MDNS responder started");
        DBG_OUTPUT_PORT.print("You can now connect to http://");
        DBG_OUTPUT_PORT.print(host);
        DBG_OUTPUT_PORT.println(".local");
    }

    server.on("/list", HTTP_GET, printDirectory);
    server.on("/edit", HTTP_DELETE, handleDelete);
    server.on("/edit", HTTP_PUT, handleCreate);
    server.on(
        "/edit", HTTP_POST, []()
        { returnOK(); },
        handleFileUpload);
    server.on("/clean", HTTP_POST, []()
              { Serial.println("Get clean msg");
              xEventGroupSetBits(handleServer,BIT_CLEAN);
              returnOK(); });
    server.on("/show", HTTP_POST, handleGetPath);
    server.onNotFound(handleNotFound);

    server.begin();
    DBG_OUTPUT_PORT.println("HTTP server started");
    if (SPIFFS.begin())
    {
        DBG_OUTPUT_PORT.println("SPIFFS Card initialized.");
        hasSPIFFS = true;
    }

    /* Initialize the screen */
    epd_init();
    libjpeg_init();
    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer)
    {
        Serial.println("alloc memory failed !!!");
        while (1)
            ;
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    epd_poweron();
    epd_clear();
}

void loop(void)
{
    server.handleClient();
    delay(2); // allow the cpu to switch to other tasks
    EventBits_t bit = xEventGroupGetBits(handleServer);
    if (bit & BIT_CLEAN)
    {
        xEventGroupClearBits(handleServer, BIT_CLEAN);
        epd_poweron();
        epd_clear();
        epd_poweroff();
    }
    else if (bit & BIT_SHOW)
    {
        xEventGroupClearBits(handleServer, BIT_SHOW);
        epd_poweron();
        File jpg = SPIFFS.open(pic_path);
        String jpg_p;
        while (jpg.available())
            jpg_p += jpg.readString();
        Rect_t rect = {
            .x = 0,
            .y = 0,
            .width = EPD_WIDTH,
            .height = EPD_HEIGHT,
        };
        show_jpg_from_buff((uint8_t *)jpg_p.c_str(), jpg_p.length(), rect);
        Serial.printf("jpg w:%d,h:%d\r\n", rect.width, rect.height);
        epd_poweroff();
    }
}