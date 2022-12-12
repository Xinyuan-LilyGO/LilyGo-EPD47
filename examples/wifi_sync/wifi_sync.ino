#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Arduino.h>
#include "epd_driver.h"
#include "libjpeg/libjpeg.h"
#include "font/firasans.h"
#include "esp_adc_cal.h"
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <FFat.h>
#include "image/logo.h"
#include "pins.h"

// #define USE_SD
#define USE_FLASH

#if defined(USE_SD)
#define FILE_SYSTEM SD
#elif defined(USE_FLASH)
#define FILE_SYSTEM FFat
#endif

#define DBG_OUTPUT_PORT Serial

const char *ssid = "GL-MT1300-44e";
const char *password = "88888888";
const char *host = "lilygo";

WebServer server(80);
static bool hasFILE_SYSTEM = false;
File uploadFile;
EventGroupHandle_t handleServer;
String pic_path;
uint8_t *framebuffer;
char buf[128];
const Rect_t line1Area = {
    .x = 0,
    .y = 387,
    .width = 960,
    .height = 51,
};
const Rect_t line2Area = {
    .x = 0,
    .y = 438,
    .width = 960,
    .height = 51,
};

const Rect_t line3Area = {
    .x = 0,
    .y = 489,
    .width = 960,
    .height = 51,
};

#define BIT_CLEAN _BV(0)
#define BIT_SHOW _BV(1)

inline void returnOK()
{
    server.send(200, "text/plain", "");
}


inline void returnFail(String msg)
{
    server.send(500, "text/plain", msg + "\r\n");
}


bool loadFromFILE_SYSTEM(String path)
{
    String dataType = "text/plain";
    if (path.endsWith("/")) {
        path += "index.htm";
    }

    if (path.endsWith(".src")) {
        path = path.substring(0, path.lastIndexOf("."));
    } else if (path.endsWith(".htm")) {
        dataType = "text/html";
    } else if (path.endsWith(".css")) {
        dataType = "text/css";
    } else if (path.endsWith(".js")) {
        dataType = "application/javascript";
    } else if (path.endsWith(".png")) {
        dataType = "image/png";
    } else if (path.endsWith(".gif")) {
        dataType = "image/gif";
    } else if (path.endsWith(".jpg")) {
        dataType = "image/jpeg";
    } else if (path.endsWith(".ico")) {
        dataType = "image/x-icon";
    } else if (path.endsWith(".xml")) {
        dataType = "text/xml";
    } else if (path.endsWith(".pdf")) {
        dataType = "application/pdf";
    } else if (path.endsWith(".zip")) {
        dataType = "application/zip";
    }

    File dataFile = FILE_SYSTEM.open(path.c_str(), "rb");
    if (dataFile.isDirectory()) {
        path += "/index.htm";
        dataType = "text/html";
        dataFile = FILE_SYSTEM.open(path.c_str());
        Serial.println("isDirectory");
    }

    if (!dataFile) {
        return false;
    }

    if (server.hasArg("download")) {
        dataType = "application/octet-stream";
    }

    if (server.streamFile(dataFile, dataType) != dataFile.size()) {
        DBG_OUTPUT_PORT.println("Sent less data than expected!");
    }

    dataFile.close();
    return true;
}


void handleFileUpload()
{
    if (server.uri() != "/edit") {
        return;
    }
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        if (FILE_SYSTEM.exists((char *)upload.filename.c_str())) {
            FILE_SYSTEM.remove((char *)upload.filename.c_str());
        }
        uploadFile = FILE_SYSTEM.open(upload.filename.c_str(), FILE_WRITE);
        DBG_OUTPUT_PORT.print("Upload: START, filename: ");
        DBG_OUTPUT_PORT.println(upload.filename);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
            uploadFile.write(upload.buf, upload.currentSize);
        }
        DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: ");
        DBG_OUTPUT_PORT.println(upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
        }
        DBG_OUTPUT_PORT.print("Upload: END, Size: ");
        DBG_OUTPUT_PORT.println(upload.totalSize);
    }
}


void deleteRecursive(String path)
{
    File file = FILE_SYSTEM.open((char *)path.c_str());
    if (!file.isDirectory()) {
        file.close();
        FILE_SYSTEM.remove((char *)path.c_str());
        return;
    }

    file.rewindDirectory();
    while (true) {
        File entry = file.openNextFile();
        if (!entry) {
            break;
        }
        String entryPath = path + "/" + entry.name();
        if (entry.isDirectory()) {
            entry.close();
            deleteRecursive(entryPath);
        } else {
            entry.close();
            FILE_SYSTEM.remove((char *)entryPath.c_str());
        }
        yield();
    }

    FILE_SYSTEM.rmdir((char *)path.c_str());
    file.close();
}


void handleDelete()
{
    if (server.args() == 0) {
        return returnFail("BAD ARGS");
    }
    String path = server.arg(0);
    if (path == "/" || !FILE_SYSTEM.exists((char *)path.c_str())) {
        returnFail("BAD PATH");
        return;
    }
    deleteRecursive(path);
    returnOK();
}


void handleCreate()
{
    if (server.args() == 0) {
        return returnFail("BAD ARGS");
    }
    String path = server.arg(0);
    if (path == "/" || FILE_SYSTEM.exists((char *)path.c_str())) {
        returnFail("BAD PATH");
        return;
    }

    if (path.indexOf('.') > 0) {
        File file = FILE_SYSTEM.open((char *)path.c_str(), FILE_WRITE);
        if (file) {
            file.write(0);
            file.close();
        }
    } else {
        FILE_SYSTEM.mkdir((char *)path.c_str());
    }
    returnOK();
}


void handleGetPath()
{
    if (server.args() == 0) {
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
    if (!server.hasArg("dir")) {
        return returnFail("BAD ARGS");
    }
    String path = server.arg("dir");
    if (path != "/" && !FILE_SYSTEM.exists((char *)path.c_str())) {
        return returnFail("BAD PATH");
    }
    File dir = FILE_SYSTEM.open((char *)path.c_str());
    path = String();
    if (!dir.isDirectory()) {
        dir.close();
        return returnFail("NOT DIR");
    }
    dir.rewindDirectory();
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/json", "");
    WiFiClient client = server.client();

    server.sendContent("[");
    for (int cnt = 0; true; ++cnt) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }

        String output;
        if (cnt > 0) {
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
    if (loadFromFILE_SYSTEM(server.uri())) {
        return;
    }
    String message = "FILE SYSTEM Not Detected\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    DBG_OUTPUT_PORT.print(message);
}


void setup()
{
    handleServer = xEventGroupCreate();
    DBG_OUTPUT_PORT.begin(115200);
    DBG_OUTPUT_PORT.setDebugOutput(true);
    DBG_OUTPUT_PORT.print("\n");

    /** Initialize the screen */
    epd_init();
    libjpeg_init();
    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1) ;
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    epd_poweron();
    epd_clear();

    Rect_t area = {
        .x = 256,
        .y = 180,
        .width = logo_width,
        .height = logo_height,
    };

    // epd_draw_grayscale_image(area, (uint8_t *)logo_data);
    epd_draw_image(area, (uint8_t *)logo_data, BLACK_ON_WHITE);

    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(ssid, password);
    sprintf(buf, "Connecting to %s", ssid);
    DBG_OUTPUT_PORT.println(buf);
    int cursor_x = line1Area.x;
    int cursor_y = line1Area.y + FiraSans.advance_y + FiraSans.descender;
    epd_clear_area(line1Area);
    writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);

    if (MDNS.begin(host)) {
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
        "/edit",
        HTTP_POST,
        []() {
            returnOK();
        },
        handleFileUpload
    );
    server.on(
        "/clean",
        HTTP_POST,
        []() {
            Serial.println("Get clean msg");
            xEventGroupSetBits(handleServer,BIT_CLEAN);
            returnOK();
        }
    );
    server.on("/show", HTTP_POST, handleGetPath);
    server.onNotFound(handleNotFound);
    server.begin();
    DBG_OUTPUT_PORT.println("HTTP server started");

#if defined(USE_SD)
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    bool rlst = FILE_SYSTEM.begin(SD_CS, SPI);
#else
    bool rlst = FILE_SYSTEM.begin(true);
#endif
    if (rlst) {
        DBG_OUTPUT_PORT.println("FS initialized.");
        hasFILE_SYSTEM = true;
    } else {
        DBG_OUTPUT_PORT.println("FS initialization failed.");
        epd_clear_area(line3Area);
        cursor_x = line3Area.x;
        cursor_y = line3Area.y + FiraSans.advance_y + FiraSans.descender;
        writeln((GFXfont *)&FiraSans, "FatFS initialization failed", &cursor_x, &cursor_y, NULL);
    }
}


void loop()
{
    server.handleClient();
    delay(2); // allow the cpu to switch to other tasks
    EventBits_t bit = xEventGroupGetBits(handleServer);
    if (bit & BIT_CLEAN) {
        xEventGroupClearBits(handleServer, BIT_CLEAN);
        epd_poweron();
        epd_clear();
        epd_poweroff();
    } else if (bit & BIT_SHOW) {
        xEventGroupClearBits(handleServer, BIT_SHOW);
        epd_poweron();
        File jpg = FILE_SYSTEM.open(pic_path);
        String jpg_p;
        while (jpg.available()) {
            jpg_p += jpg.readString();
        }
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


void WiFiEvent(WiFiEvent_t event) {
    int32_t cursor_x = 0;
    int32_t cursor_y = 0;

    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            Serial.println("WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Serial.println("Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Serial.println("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            WiFi.begin(ssid, password);
            Serial.println("Disconnected from WiFi access point");
            epd_clear_area(line1Area);
            cursor_x = line1Area.x;
            cursor_y = line1Area.y + FiraSans.advance_y + FiraSans.descender;
            writeln((GFXfont *)&FiraSans, "WiFi Disconnected", &cursor_x, &cursor_y, NULL);
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Serial.println("Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("Obtained IP address: ");
            Serial.println(WiFi.localIP());

            memset(buf, 0, sizeof(buf));
            sprintf(buf, "Connected to %s", ssid);
            epd_clear_area(line1Area);
            cursor_x = line1Area.x;
            cursor_y = line1Area.y + FiraSans.advance_y + FiraSans.descender;
            writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);

            epd_clear_area(line2Area);
            cursor_x = line2Area.x;
            cursor_y = line2Area.y + FiraSans.advance_y + FiraSans.descender;
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "Please visit http://%s.local/edit", host);
            writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.println("Lost IP address and IP address is reset to 0");
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.println("WiFi access point stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Serial.println("Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Serial.println("Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Serial.println("Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Serial.println("Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Serial.println("AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.println("STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            Serial.println("Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            Serial.println("Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("Ethernet disconnected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.println("Obtained IP address");
            break;
        default: break;
    }
}