#include <time.h>
#include <fstream>
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h> 
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include "api_code.h" // Containing the variables api_code and workspace id
#include "sensor.h"

#define wifi_max_try  10
#define duration_deep_sleep 1
#define last_sides_length 8
#define project_GET_page_size 12
#define DEBUGMODE false

// This should be built into the CORE I would assume, but it does not seem to be... Or something is not imported correctly
#define D5 14

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

void handle_root();
void handle_save();
void handle_format();
void handle_remove();
void handle_not_found();
void redirect_back_to_root();

sensors_event_t event;

std::vector<float> sides_Xs = {};
std::vector<float> sides_Ys = {};
std::vector<float> sides_Zs = {};
std::vector<String> sides_project_names = {};
std::vector<String> sides_project_ids = {};
std::vector<String> sides_workspace_names = {};
std::vector<String> sides_workspace_ids = {};

std::vector<String> project_names = {};
std::vector<String> project_ids = {};
std::vector<String> project_workspace_names = {};
std::vector<String> project_workspace_ids = {};
String user_id = "";

const char* configuration_filename = "/configuration.txt";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const char* clockify_host = "api.clockify.me";
const int httpsPort = 443;

const char* fingerprint PROGMEM = "9F 22 D4 79 6E B4 65 7A 8E F8 C9 FE D1 8B 38 20 33 5D 23 A0";

int current_side;
int last_side;
int last_sides_index = 0;
int last_sides[last_sides_length];

bool status_led = false;

void connect_to_wifi() {
    if (DEBUGMODE) {
        Serial.print("Connecting");
    }
    int count_try = 0;
    while (wifiMulti.run() != WL_CONNECTED && WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print(".");
        count_try++;
        if (count_try >= wifi_max_try) {
            if (DEBUGMODE) {
                Serial.println("\n");
                Serial.println("Impossible to establish WiFi connexion, sleep a little !");
            }
            turn_off();
            return;
        }
    }

    digitalWrite(LED_BUILTIN, LOW);
    if (DEBUGMODE) {
        Serial.println("");
    }

    timeClient.begin();
    timeClient.setTimeOffset(0); // We want UTC time
}

String get_current_time() {
    time_t epoch = (time_t)timeClient.getEpochTime();
    struct tm* p_tm = gmtime(&epoch);

    int monthDay = p_tm->tm_mday;
    int currentMonth = p_tm->tm_mon+1;
    int currentYear = p_tm->tm_year+1900;
    int hour = p_tm->tm_hour;
    int min = p_tm->tm_min;
    int sec = p_tm->tm_sec;

    String currentDate = String(currentYear) + "-";
    if (currentMonth < 10) {
        currentDate += "0";
    }
    currentDate += String(currentMonth) + "-";
    if (monthDay < 10) {
        currentDate += "0";
    }
    currentDate += String(monthDay) + "T";
    if (hour < 10) {
        currentDate += "0";
    }
    currentDate += String(hour) + ":";
    if (min < 10) {
        currentDate += "0";
    }
    currentDate += String(min) + ":";
    if (sec < 10) {
        currentDate += "0";
    }
    currentDate += String(sec) + ".000Z";

    return currentDate;
}

void GET_user() {
    WiFiClientSecure client;
    client.setFingerprint(fingerprint);
    client.setTimeout(15000);

    if (!client.connect(clockify_host, httpsPort)) {
        if (DEBUGMODE) {
            Serial.print("Connection failure: "); Serial.print(clockify_host); Serial.print(":"); Serial.println(httpsPort);
        }
        return;
    }

    String url = "/api/v1/user";
    if (DEBUGMODE) {
        Serial.print("Requesting URL: "); Serial.print(clockify_host); Serial.println(url);
    }
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + clockify_host  + "\r\n" + "X-Api-Key: " + api_code + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();

    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println("Client timeout!");
            client.stop();
            return;
        }
        delay(100);
    }


    user_id = "";
    bool found_id = false;
    while(client.available() && !found_id) {
        String line = client.readStringUntil('\n'); 
        for (unsigned int i = 5; i < line.length(); i++) {
            if (line[i - 5] == '\"' &&
                line[i - 4] == 'i' &&
                line[i - 3] == 'd' &&
                line[i - 2] == '\"' &&
                line[i - 1] == ':' &&
                line[i    ] == '\"') {
                    i++;
                    while (line[i] != '\"' && i < line.length()) {
                        user_id.concat(line[i]);
                        i++;
                    }
                    found_id = true;
                    break;
            }
        }
    }
}

void GET_projects() {
    WiFiClientSecure client;
    client.setFingerprint(fingerprint);
    client.setTimeout(15000);

    project_names = {};
    project_ids = {};
    project_workspace_ids = {};
    String line;

    for (int i = 0; i < workspace_ids.size(); i++) {
        bool found_data = true;
        int page = 1;

        while (found_data) {
            found_data = false;
            if (!client.connect(clockify_host, httpsPort)) {
                if (DEBUGMODE) {
                    Serial.print("Connection failure: "); Serial.print(clockify_host); Serial.print(":"); Serial.println(httpsPort);
                }
                return;
            }

            String url = "/api/v1/workspaces/" + workspace_ids[i] + "/projects?archived=false&page-size=" + String(project_GET_page_size) + "&page=" + String(page);
            if (DEBUGMODE) {
                Serial.print("Requesting URL: "); Serial.print(clockify_host); Serial.println(url);
            }
            client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + clockify_host  + "\r\n" + "X-Api-Key: " + api_code + "\r\n" + "Connection: close\r\n\r\n");
            unsigned long timeout = millis();

            while (client.available() == 0) {
                if (millis() - timeout > 5000) {
                    Serial.println("Client timeout!");
                    client.stop();
                    return;
                }
                delay(100);
            }


            int projects_this_page = 0;
            while(client.available() && projects_this_page < project_GET_page_size) {
                line = client.readStringUntil(']');

                if (line.length() == 0) {
                    if (DEBUGMODE) {
                        Serial.println("Parsed all data from clockify");
                    }
                    break;
                }

                unsigned int i = 5;

                while (i < line.length()) {

                    if (line[i - 5] == '\"' &&
                        line[i - 4] == 'i' &&
                        line[i - 3] == 'd' &&
                        line[i - 2] == '\"' &&
                        line[i - 1] == ':' &&
                        line[i    ] == '\"') {
                            String id = "";
                            String name = "";
                            i++;
                            while (line[i] != '\"' && i < line.length()) {
                                id += line[i];
                                i++;
                            }

                            if (i + 9 >= line.length()) {
                                break;
                            }

                            i += 9;

                            if (line[i - 8] == ',' &&
                                line[i - 7] == '\"' &&
                                line[i - 6] == 'n' &&
                                line[i - 5] == 'a' &&
                                line[i - 4] == 'm' &&
                                line[i - 3] == 'e' &&
                                line[i - 2] == '\"' &&
                                line[i - 1] == ':' &&
                                line[i    ] == '\"') {
                                    i++;
                                    while(line[i] != '\"' && i < line.length()) {
                                        name += line[i];
                                        i++;
                                    }
                            }

                            if (i == line.length()) {
                                break;
                            }

                            if (DEBUGMODE) {
                                Serial.print("Id: "); Serial.print(id); Serial.print(" Name: "); Serial.println(name);
                            }
                            project_names.push_back(name);
                            project_ids.push_back(id);
                            project_workspace_ids.push_back(workspace_ids[i]);
                            project_workspace_names.push_back(workspace_names[i]);
                            projects_this_page++;
                            if (projects_this_page >= project_GET_page_size) {
                                found_data = true;
                            }
                    }

                    i++;
                }
            }

            page++;
        }
    }

    if (DEBUGMODE) {
        Serial.println("Closing connection");
    }
}

void PATCH_project_stop() {
    WiFiClientSecure client;

    client.setFingerprint(fingerprint);
    client.setTimeout(15000);

    if (!client.connect(clockify_host, httpsPort)) {
        if (DEBUGMODE) {
            Serial.print("Connection failure: "); Serial.print(clockify_host); Serial.print(":"); Serial.println(httpsPort);
        }
        return;
    }

    for (unsigned int i = 0; i < workspace_ids.size(); i++) {
        String url = "/api/v1/workspaces/" + workspace_ids[i] + "/user/" + user_id + "/time-entries";
        if (DEBUGMODE) {
            Serial.print("Requesting URL: "); Serial.print(clockify_host); Serial.println(url);
        }
        String postData = "{\"end\":\"" + get_current_time() + "\"}";
        HTTPClient http;
        http.begin(client, String("https://") + clockify_host + url);
        http.addHeader("Content-type", "application/json");
        http.addHeader("X-Api-Key", api_code);
        int httpCode = http.PATCH(postData);
        if (DEBUGMODE) {
            String payload = http.getString();
            Serial.println(httpCode);
            Serial.println(payload);
        }

        http.end();
    }
}

void POST_project_start() {
    if (current_side < 0) {
        return;
    }

    WiFiClientSecure client;

    client.setFingerprint(fingerprint);
    client.setTimeout(15000);

    if (!client.connect(clockify_host, httpsPort)) {
        if (DEBUGMODE) {
            Serial.print("Connection failure: "); Serial.print(clockify_host); Serial.print(":"); Serial.println(httpsPort);
        }
        return;
    }

    String url = "/api/v1/workspaces/" + sides_workspace_ids[current_side] + "/time-entries";
    if (DEBUGMODE) {
        Serial.print("Requesting URL: "); Serial.print(clockify_host); Serial.println(url);
    }
    String postData = "{\"start\":\"" + get_current_time() + "\",\"projectId\":\"" + sides_project_ids[current_side] + "\"}";
    HTTPClient http;
    http.begin(client, String("https://") + clockify_host + url);
    http.addHeader("Content-type", "application/json");
    http.addHeader("X-Api-Key", api_code);
    int httpCode = http.POST(postData);
    if (DEBUGMODE) {
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
    }

    http.end();
}

void POST_project() {
    if (current_side == -1 || sides_project_ids[current_side].length() <= 10) { // NOTE: We should be able to compare it to "noproj". But for some reason the length is 7, probably some \r or other character at the end or begining :(
        PATCH_project_stop();
    } else {
        POST_project_start();
    }
}

void turn_off() {
    for (unsigned int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(50);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(50);
    }
    digitalWrite(LED_BUILTIN, LOW);
    if (WiFi.status() == WL_CONNECTED) {
        PATCH_project_stop();
    }

    if (DEBUGMODE) {
        Serial.println("Turning off");
    }
    for (unsigned int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(50);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(50);
    }
    ESP.deepSleep(duration_deep_sleep * 1000000);
}

void format_memory() {
    if (LittleFS.format()) {
        if (DEBUGMODE) {
            Serial.println("File System Formated");
        }
    } else {
        if (DEBUGMODE) {
            Serial.println("File System Formatting Error");
        }
    }
}

// save and load project workspace ids
void load_configuration() {
    if (LittleFS.exists(configuration_filename)) {
        File f = LittleFS.open(configuration_filename, "r");

        if (!f) {
            if (DEBUGMODE) {
                Serial.println("file open failed");
            }
        } else {
            if (DEBUGMODE) {
                Serial.println("Reading Data from File:");
            }
            String current = "";
            char c;
            while (f.available() && (c = (char)f.read())) {
                if (DEBUGMODE) {
                    Serial.print(c);
                }
                if (c == ';') {
                    sides_Xs.push_back(current.toFloat());
                    current = "";
                } else if (c == '\n') {
                    break;
                } else {
                    current.concat(c);
                }
            }
            while (f.available() && (c = (char)f.read())) {
                if (DEBUGMODE) {
                    Serial.print(c);
                }
                if (c == ';') {
                    sides_Ys.push_back(current.toFloat());
                    current = "";
                } else if (c == '\n') {
                    break;
                } else {
                    current.concat(c);
                }
            }
            while (f.available() && (c = (char)f.read())) {
                if (DEBUGMODE) {
                    Serial.print(c);
                }
                if (c == ';') {
                    sides_Zs.push_back(current.toFloat());
                    current = "";
                } else if (c == '\n') {
                    break;
                } else {
                    current.concat(c);
                }
            }
            while (f.available() && (c = (char)f.read())) {
                if (DEBUGMODE) {
                    Serial.print(c);
                }
                if (c == ';') {
                    sides_project_names.push_back(current);
                    current = "";
                } else if (c == '\n') {
                    break;
                } else {
                    current.concat(c);
                }
            }
            while (f.available() && (c = (char)f.read())) {
                if (DEBUGMODE) {
                    Serial.print(c);
                }
                if (c == ';') {
                    sides_project_ids.push_back(current);
                    current = "";
                } else if (c == '\n') {
                    break;
                } else {
                    current.concat(c);
                }
            }
            while (f.available() && (c = (char)f.read())) {
                if (DEBUGMODE) {
                    Serial.print(c);
                }
                if (c == ';') {
                    sides_workspace_names.push_back(current);
                    current = "";
                } else if (c == '\n') {
                    break;
                } else {
                    current.concat(c);
                }
            }
            while (f.available() && (c = (char)f.read())) {
                if (DEBUGMODE) {
                    Serial.print(c);
                }
                if (c == ';') {
                    sides_workspace_ids.push_back(current);
                    current = "";
                } else if (c == '\n') {
                    break;
                } else {
                    current.concat(c);
                }
            }
            f.close();
            if (DEBUGMODE) {
                Serial.println("File Closed");
            }

            if (sides_Xs.size() != sides_Ys.size() ||
                sides_Ys.size() != sides_Zs.size() ||
                sides_Zs.size() != sides_project_ids.size() ||
                sides_project_ids.size() != sides_project_names.size() ||
                sides_project_names.size() != sides_workspace_names.size() ||
                sides_workspace_names.size() != sides_workspace_ids.size()) {
                if (DEBUGMODE) {
                    Serial.println("The loaded values did not have the same size. Formatting memory...");
                }
                format_memory();
            }
        }
    }
}

void write_configuration() {
    File f = LittleFS.open(configuration_filename, "w");

    if (!f) {
        if (DEBUGMODE) {
            Serial.println("File open failed");
        }
    } else {
        if (DEBUGMODE) {
            Serial.println("Writing data to file");
        }
        for (unsigned int i = 0; i < sides_Xs.size(); i++) {
            f.print(String(sides_Xs[i], 2));
            f.print(";");
        }
        f.print("\n");
        for (unsigned int i = 0; i < sides_Ys.size(); i++) {
            f.print(String(sides_Ys[i], 2));
            f.print(";");
        }
        f.print("\n");
        for (unsigned int i = 0; i < sides_Zs.size(); i++) {
            f.print(String(sides_Zs[i], 2));
            f.print(";");
        }
        f.print("\n");
        for (unsigned int i = 0; i < sides_project_names.size(); i++) {
            f.print(sides_project_names[i]);
            f.print(";");
        }
        f.print("\n");
        for (unsigned int i = 0; i < sides_project_ids.size(); i++) {
            f.print(sides_project_ids[i]);
            f.print(";");
        }
        f.print("\n");
        for (unsigned int i = 0; i < sides_workspace_names.size(); i++) {
            f.print(sides_workspace_names[i]);
            f.print(";");
        }
        f.print("\n");
        for (unsigned int i = 0; i < sides_workspace_ids.size(); i++) {
            f.print(sides_workspace_ids[i]);
            f.print(";");
        }
        f.print("\n");
        f.close();
    }
}

int read_side() {
    int closestIndex = -1;
    float closestDistance = -1.0f;

    for (unsigned int i = 0; i < sides_Xs.size(); i++) {
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

void setup(void) {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(D5, INPUT);
    digitalWrite(LED_BUILTIN, LOW); // For some reason HIGH and LOW seem to be switched when it comes to the builtin led

    for (unsigned int i = 0; i < last_sides_length; i++) {
        last_sides[i] = -1;
    }
    last_sides_index = 0;
    last_side = -1;
    current_side = -1;

    if (DEBUGMODE) {
        Serial.begin(9600);
    }

    delay(100);
    if (DEBUGMODE) {
        Serial.println("Accelerometer Test"); Serial.println("");
    }

    if (!accel.begin())
    {
        if (DEBUGMODE) {
            Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
        }
        turn_off();
        return;
    }

    accel.setRange(ADXL345_RANGE_16_G);
    // displaySetRange(ADXL345_RANGE_16_G);
    // displaySetRange(ADXL345_RANGE_8_G);
    // displaySetRange(ADXL345_RANGE_4_G);
    // displaySetRange(ADXL345_RANGE_2_G);

    accel_displaySensorDetails();
    accel_displayDataRate();
    accel_displayRange();
    if (DEBUGMODE) {
        Serial.println("");
    }

    if (LittleFS.begin()) {
        if (DEBUGMODE) {
            Serial.println("LittleFS Initialize... ok");
        }
    } else {
        if (DEBUGMODE) {
            Serial.println("LittleFS Initialize... failed");
        }
    }

    load_configuration();

    if (DEBUGMODE) {
        Serial.println();
    }

    wifiMulti.addAP("Elias 11", "b3yz7naa5mkztar");
    // wifiMulti.addAP("Olof", "mattekollo");
    // WiFi.begin(ssid, password);

    connect_to_wifi();

    if (DEBUGMODE) {
        Serial.println();
        Serial.print("Connected to "); 
        Serial.print(WiFi.SSID()); 
        Serial.print(", IP address: ");
        Serial.println(WiFi.localIP());
    }

    if (MDNS.begin("esp8266")) {
        if (DEBUGMODE) {
            Serial.println("mDNS responder started.");
        }
    } else {
        if (DEBUGMODE) {
            Serial.println("Error setting up mDNS responder!");
        }
    }

    server.on("/", HTTP_GET, handle_root);
    server.on("/save", HTTP_POST, handle_save);
    server.on("/format", HTTP_POST, handle_format);
    server.on("/remove", HTTP_POST, handle_remove);
    server.onNotFound(handle_not_found);

    server.begin();
    if (DEBUGMODE) {
        Serial.println("HTTP server started");
    }
    GET_projects();
    GET_user();
    if (DEBUGMODE) {
        Serial.print("UserID: "); Serial.println(user_id);
    }
}

void loop(void)
{
    status_led = !status_led;
    digitalWrite(LED_BUILTIN, status_led);

    if (WiFi.status() != WL_CONNECTED) {
        if (DEBUGMODE) {
            Serial.println("Lost wifi connection");
        }
        connect_to_wifi();
        return;
    }

    if (digitalRead(D5) == HIGH) {
        if (DEBUGMODE) {
            Serial.println("Turned off by user");
        }
        turn_off();
        return;
    }

    timeClient.update();

    server.handleClient();
    accel.getEvent(&event);

    int side = read_side();
    last_sides[last_sides_index] = side;
    last_sides_index = (last_sides_index + 1) % last_sides_length;

    bool all_same = true;
    int value = -2;
    for (unsigned int i = 0; i < last_sides_length; i++) {
        if (value == -2) {
            value = last_sides[i];
        }

        if (last_sides[i] != value) {
            all_same = false;
            break;
        }
    }

    if (all_same && value >= 0) {
        current_side = side;
    }

    if (current_side != last_side) {
        if (DEBUGMODE) {
            Serial.print("Changing side to: "); Serial.println(current_side);
        }
        last_side = current_side;
        POST_project();
    }

    // if (DEBUGMODE) {
        // Serial.print("Current date: "); Serial.println(get_current_time());
        // Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print(" ");
        // Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print(" ");
        // Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print(" "); Serial.println("m/s^2 ");
        // Serial.print("Side: "); Serial.println(read_side()); 
    // }

    delay(128);
}

void handle_root() {
    String current_sides = "";
    for (unsigned int i = 0; i < sides_Xs.size(); i++) {
        current_sides.concat("<p>");
        current_sides.concat("[");
        current_sides.concat(i);
        current_sides.concat("] ");
        current_sides.concat(sides_workspace_names[i]);
        current_sides.concat(" (");
        current_sides.concat(sides_workspace_ids[i]);
        current_sides.concat("): ");
        current_sides.concat(sides_project_names[i]);
        current_sides.concat(" (");
        current_sides.concat(sides_project_ids[i]);
        current_sides.concat(") X: ");
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

    String project_dropdown = "";
    for (unsigned int i = 0; i < project_names.size(); i++) {
        project_dropdown += "<option value=\"" + project_workspace_ids[i] + ":" + project_ids[i] + "\">" + project_workspace_names[i] + ": " + project_names[i] + "</option>";
    }

    String html =
        "<html>"
        "<head>"
        "    <meta charset=\"UTF-8\">"
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
    "        <label for=\"project\">Choose what project to log time on</label>"
    "        <select name=\"project\" id=\"project\">"
    "            <option value=\"noproj:noproj\">No Project/Stop Logging</option>");
    html.concat(project_dropdown);
    html.concat(
    "        </select><br>"
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

    if (DEBUGMODE) {
        Serial.println(html);
    }

    server.send(200, "text/html", html);
}

void handle_save() {
    if (!server.hasArg("project") || server.arg("project") == NULL) {
        server.send(400, "text/plain", "400: Invalid request");
    }

    sides_Xs.push_back(event.acceleration.x);
    sides_Ys.push_back(event.acceleration.y);
    sides_Zs.push_back(event.acceleration.z);
    String project_id, ws_id;
    bool is_ws = true;
    for (unsigned int i = 0; i < server.arg("project").length(); i++) {
        if (server.arg("project")[i] == ':') {
            is_ws = false;
            continue;
        }

        if (is_ws) {
            ws_id.concat(server.arg("project")[i]);
        } else {
            project_id.concat(server.arg("project")[i]);
        }
    }
    sides_project_ids.push_back(project_id);
    sides_workspace_ids.push_back(ws_id);

    if (project_id == "noproj") {
        sides_project_names.push_back("No Project/Stop Logging");
    } else {
        for (unsigned int i = 0; i < project_ids.size(); i++) {
            if (DEBUGMODE) {
                Serial.println(project_ids[i]);
                Serial.println(server.arg("project"));
            }
            if (project_ids[i] == project_id) {
                sides_project_names.push_back(project_names[i]);
                break;
            }
        }
    }

    if (ws_id == "noproj") {
        sides_workspace_names.push_back("");
    } else {
        for (unsigned int i = 0; i < project_ids.size(); i++) {
            if (DEBUGMODE) {
                Serial.println(workspace_ids[i]);
                Serial.println(ws_id);
            }
            if (workspace_ids[i] == ws_id) {
                sides_workspace_names.push_back(workspace_names[i]);
                break;
            }
        }
    }

    write_configuration();

    redirect_back_to_root();
}

void handle_format() {
    format_memory();
    sides_Xs = {};
    sides_Ys = {};
    sides_Zs = {};
    sides_project_ids = {};
    sides_project_names = {};
    sides_workspace_names = {};
    sides_workspace_ids = {};

    redirect_back_to_root();
}

void handle_remove() {
    if (!server.hasArg("index") || server.arg("index") == NULL) {
        server.send(400, "text/plain", "400: Invalid request");
        return;
    }

    int ind = server.arg("index").toInt();

    if (ind < 0 || ind >= sides_Xs.size()) {
        server.send(400, "text/plain", "400: Invalid request");
        return;
    }

    sides_Xs.erase(sides_Xs.begin() + ind);
    sides_Ys.erase(sides_Ys.begin() + ind);
    sides_Zs.erase(sides_Zs.begin() + ind);
    sides_project_ids.erase(sides_project_ids.begin() + ind);
    sides_project_names.erase(sides_project_names.begin() + ind);
    sides_workspace_ids.erase(sides_workspace_ids.begin() + ind);
    sides_workspace_names.erase(sides_workspace_names.begin() + ind);

    write_configuration();

    redirect_back_to_root();
}

void handle_not_found() {
    server.send(404, "text/plain", "404: Not found");
}

void redirect_back_to_root() {
    server.send(200, "text/html", "<html><script>location.pathname = \"/\"</script></html>");
}
