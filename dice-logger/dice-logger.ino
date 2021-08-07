#include <LittleFS.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h> 
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include "sensor.h"

#define wifi_max_try  10
#define duration_deep_sleep 10

IPAddress ip(192, 168, 125, 40);
IPAddress dns(192, 168, 125, 1);
IPAddress gateway(192, 168, 125, 96);
IPAddress subnet(255, 255, 255, 0);

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

void handleRoot();
void handleSave();
void handleFormat();
void handleRemove();
void handleNotFound();

sensors_event_t event;

std::vector<float> sides_Xs = {};
std::vector<float> sides_Ys = {};
std::vector<float> sides_Zs = {};
std::vector<String> sides_titles = {};

const char* configuration_filename = "/configuration.txt";

void format_memory() {
    if (LittleFS.format()) {
        Serial.println("File System Formated");
    } else {
        Serial.println("File System Formatting Error");
    }
}

void load_configuration() {
    if (LittleFS.exists(configuration_filename)) {
        File f = LittleFS.open(configuration_filename, "r");

        if (!f) {
            Serial.println("file open failed");
        } else {
            Serial.println("Reading Data from File:");
            for(int i = 0; i < f.size(); i++) {
                Serial.print((char)f.read());
            }
            f.close();
            Serial.println("File Closed");
        }
    }
}

void write_configuration() {
    File f = LittleFS.open(configuration_filename, "w");

    if (!f) {
        Serial.println("File open failed");
    } else {
        Serial.println("Writing data to file");
        for (int i = 0; i < sides_Xs.size(); i++) {
            f.print(String(sides_Xs[i], 2));
            f.print(";");
        }
        f.println("");
        for (int i = 0; i < sides_Xs.size(); i++) {
            f.print(String(sides_Ys[i], 2));
            f.print(";");
        }
        f.println("");
        for (int i = 0; i < sides_Xs.size(); i++) {
            f.print(String(sides_Zs[i], 2));
            f.print(";");
        }
        f.println("");
        for (int i = 0; i < sides_Xs.size(); i++) {
            f.print(sides_titles[i]);
            f.print(";");
        }
        f.println("");
        f.close();
    }
}

int read_side() {
    /*
        0: X: -1.41 Y: -0.16 Z: 32.32 m/s^2
        1: X: -1.53 Y: -11.38 Z: 21.50 m/s^2
        2: X: -1.57 Y: 0.71 Z: 12.75 m/s^2
        3: X: -0.82 Y: 12.51 Z: 22.63 m/s^2
        4: X: 10.94 Y: -0.08 Z: 22.32 m/s^2
        5: X: -13.49 Y: 0.55 Z: 22.40 m/s^2
    */

    int closestIndex = -1;
    float closestDistance = -1.0f;

    for (int i = 0; i < sides_Xs.size(); i++) {
        float distance = (sides_Xs[i] - event.acceleration.x) * (sides_Xs[i] - event.acceleration.x) + 
            (sides_Ys[i] - event.acceleration.y) * (sides_Ys[i] - event.acceleration.y) + 
            (sides_Zs[i] - event.acceleration.z) * (sides_Zs[i] - event.acceleration.z);
        if (closestDistance == -1 || distance < closestDistance) {
            closestDistance = distance;
            closestIndex = i;
        }
    }

    return closestIndex;
}

void setup(void)
{
    Serial.begin(9600);
    delay(10);
    Serial.println("Accelerometer Test"); Serial.println("");

    /* Initialise the sensor */
    if (!accel.begin())
    {
        /* There was a problem detecting the ADXL345 ... check your connections */
        Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
        while (1);
    }

    /* Set the range to whatever is appropriate for your project */
    accel.setRange(ADXL345_RANGE_16_G);
    // displaySetRange(ADXL345_RANGE_16_G);
    // displaySetRange(ADXL345_RANGE_8_G);
    // displaySetRange(ADXL345_RANGE_4_G);
    // displaySetRange(ADXL345_RANGE_2_G);

    /* Display some basic information on this sensor */
    accel_displaySensorDetails();

    /* Display additional settings (outside the scope of sensor_t) */
    accel_displayDataRate();
    accel_displayRange();
    Serial.println("");

    if (LittleFS.begin()) {
        Serial.println("LittleFS Initialize... ok");
    } else {
        Serial.println("LittleFS Initialize... failed");
    }

    load_configuration();

    Serial.println();
    WiFi.config(ip, dns, gateway, subnet);
    wifiMulti.addAP("Elias 11", "b3yz7naa5mkztar");
    // wifiMulti.addAP("Olof", "mattekollo");
    // WiFi.begin(ssid, password);

    Serial.print("Connecting");
    int count_try = 0;
    while (wifiMulti.run() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
        count_try++;
        if ( count_try >= wifi_max_try ) {
            Serial.println("\n");
            Serial.println("Impossible to establish WiFi connexion, sleep a little !");
            // delay(duration_deep_sleep * 1000);
            ESP.deepSleep(duration_deep_sleep * 1000000);
        }
    }
    Serial.println();

    Serial.print("Connected to "); 
    Serial.print(WiFi.SSID()); 
    Serial.print(", IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp8266")) {
        Serial.println("mDNS responder started.");
    } else {
        Serial.println("Error setting up mDNS responder!");
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/format", HTTP_POST, handleFormat);
    server.on("/remove", HTTP_POST, handleRemove);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

void loop(void)
{
    server.handleClient();
    accel.getEvent(&event);
    /*
    

    Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print(" ");
    Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print(" ");
    Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print(" "); Serial.println("m/s^2 ");
    Serial.print("Side: "); Serial.println(read_side(event));
    delay(500);
    */
}

void handleRoot() {
    String current_sides = "";
    for (int i = 0; i < sides_Xs.size(); i++) {
        current_sides.concat("<p>");
        current_sides.concat(i);
        current_sides.concat(": ");
        current_sides.concat(sides_titles[i]);
        current_sides.concat(" X: ");
        current_sides.concat(sides_Xs[i]);
        current_sides.concat(" Y: ");
        current_sides.concat(sides_Ys[i]);
        current_sides.concat(" Z: ");
        current_sides.concat(sides_Zs[i]);
        current_sides.concat("</p>");
    }

    String current_reading = "X: ";
    current_reading.concat(event.acceleration.x);
    current_reading.concat(" Y: ");
    current_reading.concat(event.acceleration.y);
    current_reading.concat(" Z: ");
    current_reading.concat(event.acceleration.z);
    current_reading.concat(" Side: ");
    current_reading.concat(read_side());

    String html =
        "<html>"
        "<head>"
        "    <title>Time Logging Dice</title>"
        "</head>"
        "<body>"
        "    <h1>Time Logging Dice Interface</h1>";
    html.concat(current_sides);
    html.concat("<h2>Current reading: </h2>");
    html.concat(current_reading);
    html.concat("<h2>Configuration options: </h2>");
    html.concat(
    "    <form action=\"/save\" method=\"POST\">"
    "        <input type=\"text\" name=\"title\" placeholder=\"Title\"><br>"
    "        <input type=\"submit\" value=\"Save\"><br>"
    "    </form>"
    "    <form action=\"/format\" method=\"POST\">"
    "        <input type=\"submit\" value=\"Clear saved sides\"><br>"
    "    </form>"
    "    <form action=\"/remove\" method=\"POST\">"
    "        <input type=\"number\" name=\"index\" placeholder=\"Index\"><br>"
    "        <input type=\"submit\" value=\"Remove\"><br>"
    "    </form>"
    "</body>"
    "</html>");

    Serial.println(html);

    server.send(200, "text/html", html);
}

void handleSave() {
    if (!server.hasArg("title") || server.arg("title") == NULL) {
        server.send(400, "text/plain", "400: Invalid request");
    }

    sides_Xs.push_back(event.acceleration.x);
    sides_Ys.push_back(event.acceleration.y);
    sides_Zs.push_back(event.acceleration.z);
    sides_titles.push_back(server.arg("title"));

    write_configuration();

    handleRoot();
}

void handleFormat() {
    format_memory();
    sides_titles = {};
    sides_Xs = {};
    sides_Ys = {};
    sides_Zs = {};
    handleRoot();
}

void handleRemove() {
    if (!server.hasArg("index") || server.arg("index") == NULL) {
        server.send(400, "text/plain", "400: Invalid request");
        return;
    }

    int ind = server.arg("index").toInt();

    if (ind < 0 || ind >= sides_Xs.size()) {
        server.send(400, "text/plain", "400: Invalid request");
        return;
    }

    sides_titles.erase(sides_titles.begin() + ind);
    sides_Xs.erase(sides_Xs.begin() + ind);
    sides_Ys.erase(sides_Ys.begin() + ind);
    sides_Zs.erase(sides_Zs.begin() + ind);

    write_configuration();

    handleRoot();
}

void handleNotFound() {
    server.send(404, "text/plain", "404: Not found");
}
