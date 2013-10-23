#include "mbed.h"
#include "EthernetInterface.h"
#include "LM75B.h"
#include "C12832_lcd.h"
#include "NTPClient.h"
#include <string>

// Set Location info
#define LOC_LONG -96.751614  //float
#define LOC_LAT 33.007872  //float
#define LOC_ELEV 697.00  //float
#define LOC_NAME "Plano Foundry"

// Set API related info
#define APIPORT 80
#define APIKEY "0d037fb60a9a94c7eb8e77b6f4864a4d"
#define FEEDID "d1d4252ace670282db55f5913e22d8fc"
#define APIHOST "api-m2x.att.citrusbyte.com"

LM75B tmp(p28,p27);         // I2C Temperature Sensor
EthernetInterface eth;      // Ethernet
TCPSocketConnection sock;   // TCP Socket
Serial pc(USBTX, USBRX);    // debugging to PC
C12832_LCD lcd;             // Output to LCD
NTPClient ntp;              // NTP to set real time clock


void setupEthernet()
{

    lcd.printf("Setup Ethernet...\r\n");   // Starting
    eth.init();   // Initialize
    while (eth.connect()) {   // timeout and loop until connected
        lcd.printf("Connect timeout\r\n");
    };
    lcd.printf("IP Address is %s\r\n", eth.getIPAddress());   // successful connect
}

char getStream(char* streamName, char* json)
{
    char http_cmd[512];
    char buffer[1024];
    int ret;

    sock.connect(APIHOST, APIPORT);

    sprintf(http_cmd, "GET /v1/feeds/%s/streams/%s HTTP/1.0\r\nX-M2X-KEY: %s\r\n\r\n", FEEDID, streamName, APIKEY);
    //pc.printf(http_cmd);

    sock.send_all(http_cmd, sizeof(http_cmd)-1);

    ret = sock.receive(buffer, sizeof(buffer)-1);
    if (ret <= 0)
        return(0);
    buffer[ret] = '\0';
    if (strncmp(buffer, "HTTP/", 5) == 0) {
        ret = atoi(&buffer[9]);
    }

    //pc.printf("Received %d chars from server:\r\n%s\r\n", ret, buffer);

    strcpy(json, buffer);
    sock.close();

    return ret;

}

char getLocation(char* json)
{
    char http_cmd[512];
    char buffer[1024];
    int ret;

    sock.connect(APIHOST, APIPORT);

    sprintf(http_cmd, "GET /v1/feeds/%s/location HTTP/1.0\r\nX-M2X-KEY: %s\r\n\r\n", FEEDID, APIKEY);
    //pc.printf(http_cmd);

    sock.send_all(http_cmd, sizeof(http_cmd)-1);

    ret = sock.receive(buffer, sizeof(buffer)-1);
    if (ret <= 0)
        return(0);
    buffer[ret] = '\0';
    if (strncmp(buffer, "HTTP/", 5) == 0) {
        ret = atoi(&buffer[9]);
    }

    //pc.printf("Received %d chars from server:\r\n%s\r\n", ret, buffer);

    strcpy(json, buffer);
    sock.close();

    return ret;

}

char putStream(char* streamName, char* json)
{

    int json_length;
    char http_cmd[512];
    char buffer[1024];
    int ret;

    sock.connect(APIHOST, 80);

    json_length = strlen(json);

    sprintf(http_cmd, "PUT /v1/feeds/%s/streams/%s HTTP/1.0\r\nX-M2X-KEY: %s\r\nContent-length: %d\r\nContent-type: application/json\r\n\r\n%s\r\n", FEEDID, streamName, APIKEY, json_length, json);
    //pc.printf(http_cmd);

    sock.send_all(http_cmd, sizeof(http_cmd)-1);

    ret = sock.receive(buffer, sizeof(buffer)-1);
    if (ret <= 0)
        return(0);
    buffer[ret] = '\0';
    if (strncmp(buffer, "HTTP/", 5) == 0) {
        ret = atoi(&buffer[9]);
    }
    //pc.printf("Received %d chars from server:\r\n%s\r\n", ret, buffer);

    strcpy(json, buffer);
    sock.close();

    return ret;
}

char putLocation(char* json)
{

    int json_length;
    char http_cmd[512];
    char buffer[1024];
    int ret;

    sock.connect(APIHOST, 80);

    json_length = strlen(json);

    sprintf(http_cmd, "PUT /v1/feeds/%s/location HTTP/1.0\r\nX-M2X-KEY: %s\r\nContent-length: %d\r\nContent-type: application/json\r\n\r\n%s\r\n", FEEDID, APIKEY, json_length, json);
    //pc.printf(http_cmd);

    sock.send_all(http_cmd, sizeof(http_cmd)-1);

    ret = sock.receive(buffer, sizeof(buffer)-1);
    if (ret <= 0)
        return(0);
    buffer[ret] = '\0';
    if (strncmp(buffer, "HTTP/", 5) == 0) {
        ret = atoi(&buffer[9]);
    }
    //pc.printf("Received %d chars from server:\r\n%s\r\n", ret, buffer);

    strcpy(json, buffer);
    sock.close();

    return ret;
}

int postStream (char* streamName, char* json)
{

    int json_length;
    char http_cmd[512];
    char buffer[1024];
    int ret;

    sock.connect(APIHOST, 80);

    json_length = strlen(json);

    sprintf(http_cmd, "POST /v1/feeds/%s/streams/%s/values HTTP/1.0\r\nX-M2X-KEY: %s\r\nContent-length: %d\r\nContent-type: application/json\r\n\r\n%s\r\n", FEEDID, streamName, APIKEY, json_length, json);
    //pc.printf(http_cmd);

    sock.send_all(http_cmd, sizeof(http_cmd)-1);

    ret = sock.receive(buffer, sizeof(buffer)-1);
    if (ret <= 0)
        return(0);
    buffer[ret] = '\0';
    if (strncmp(buffer, "HTTP/", 5) == 0) {
        ret = atoi(&buffer[9]);
    }
    //pc.printf("Received %d chars from server:\r\n%s\r\n", ret, buffer);

    strcpy(json, buffer);
    sock.close();

    return ret;
}

std::string tail(std::string const& source, size_t const length)
{
    if (length >= source.size()) {
        return source;
    }
    return source.substr(source.size() - length);
}

int readContentLength(char* json)
{

    std::string str ("Content-Length: ");
    std::string str2 ("\r\n");
    std::size_t found2;
    std::size_t found3;
    int value=0;

    std::string response = json;

    //pc.printf("json length %d\r\nresponse %s\r\n", strlen(json), json);
    std::size_t found = response.find(str);
    strcpy(json, "");
    if (found!=std::string::npos) {

        found2=response.find(str2,found+1);
        found3=found2-(found+16);

        std::string str3 = response.substr (found+16, found3);
        value = atoi(str3.c_str());
        if (value<1) {
            strcpy(json, "");
        } else {
            std::string t = tail(response, value);
            strcpy(json, t.c_str());
        }
    }

    return value;

}


int main()
{
    setbuf(stdout, NULL); // no buffering for this filehandle
    char json[1024];

    int content_length;
    int response_code;
    char buf[1024];

    float amb_temp;

    setupEthernet();

    // set time
    lcd.printf("Reading Time... ");
    ntp.setTime("time-c.nist.gov");
    lcd.printf("Time set");


    while(1) {  // Loop
        char streamName[] = "amb-temp";  // Set value for stream you are using
     
        // GET Stream value
        response_code = getStream(streamName, json);
        content_length = readContentLength(json);
        pc.printf("GET Stream\r\nResponse: %d\r\nContent Length: %d\r\njson: %s\r\n\r\n", response_code, content_length, json);
        Thread::wait(5000);

/////
        // PUT value to Strem
        amb_temp = tmp.read();

        sprintf(json, "{\"value\":\"%0.2f\"}", amb_temp);

        response_code = putStream(streamName, json);
        content_length = readContentLength(json);
        pc.printf("PUT Stream\r\nResponse: %d\r\nContent Length: %d\r\njson: %s\r\n\r\n", response_code, content_length, json);
        Thread::wait(5000);


//////
        // POST value(s) to Stream
        time_t seconds;

        seconds = time(NULL);   // get current time from mbed RTC
        strftime(buf,40, "%Y-%m-%dT%H:%M:%S%z", localtime(&seconds));

        amb_temp = tmp.read();

        sprintf(json, "{ \"values\": [ {\"at\": \"%s\", \"value\":\"%0.2f\"} ] }", buf, amb_temp);

        response_code = postStream(streamName, json);
        content_length = readContentLength(json);

        pc.printf("POST Stream\r\nResponse: %d\r\nContent Length: %d\r\njson: %s\r\n\r\n", response_code, content_length, json);
        Thread::wait(5000);

///////
        // PUT Location to Feed
        sprintf(json, "{ \"name\": \"%s\", \"latitude\": \"%0.8f\", \"longitude\": \"%0.8f\", \"elevation\": \"%0.2f\" }", LOC_NAME, LOC_LAT, LOC_LONG, LOC_ELEV);

        response_code = putLocation(json);
        content_length = readContentLength(json);

        pc.printf("PUT Location\r\nResponse: %d\r\nContent Length: %d\r\njson: %s\r\n\r\n", response_code, content_length, json);
        Thread::wait(5000);

///////
        // GET Location of Feed
        response_code = getLocation(json);
        content_length = readContentLength(json);

        pc.printf("GET Location\r\nResponse: %d\r\nContent Length: %d\r\njson: %s\r\n\r\n", response_code, content_length, json);
        Thread::wait(5000);

    }

    eth.disconnect();

    while(1) {}
}