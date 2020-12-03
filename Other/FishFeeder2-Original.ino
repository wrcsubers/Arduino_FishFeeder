#include <ezTime.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <AccelStepper.h>
#include <FS.h>

//Constants
const char * WIFI_SSID = "Your_WiFi_SSID";
const char * WIFI_Password = "Your_WiFi_Password";
const char * mDNS_Hostname = "FishFeeder";

//Initialize Webserver/SPIFFS
//========================================================================
ESP8266WebServer server(80);
String webPageLog;
// Convert the file extension to the MIME type
String getContentType(String filename);
// Send the right file to the client (if it exists)
bool handleFileRead(String path);

//Initialize Timezone
//========================================================================
Timezone myTZ;

//Initialize Feed Motor 
//========================================================================
AccelStepper feedMotor(AccelStepper::HALF4WIRE, D8, D6, D7, D5);

//Initialize Settings
//========================================================================
int feedingsRemaining = NULL;
int feedingsPerDay = NULL;


void setup() {
	Serial.begin(115200);

  //Connect to Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_Password);
  Serial.print("\nWaiting for WiFi to connect...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);    
  }
  Serial.println("\n\nWiFi connected! IP Address: " + (WiFi.localIP()).toString());

  //Start mDNS
  MDNS.begin(mDNS_Hostname);
  //Serial.println("mDNS started! Using: " + &mDNS_Hostname + ".local");

  //Start SPIFFS
  SPIFFS.begin();
  
  //Start Webserver
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });
  server.begin();

  
	// Wait for ezTime to get its time synchronized
	webPageLog += ("\nWaiting for Time Sync... ");
	waitForSync();
	//Set Timezone
	myTZ.setLocation(F("America/Los_Angeles"));
  myTZ.setDefault();
  //Print Current Time 
	webPageLog += (F("\nTime Set! Los Angeles: "));
	webPageLog += (myTZ.dateTime());
	delay(5000);
  //Set ezTime update interval
	setInterval(1800);
	// Make ezTime show us what it is doing
	setDebug(INFO);

  //Set Events to run
  //Daily Alarms formatted as: setEvent(eventFunctionName, calcDailyAlarmTime(hourToRun, minuteToRun));
  //Regular Interval Alarms formated as: setEvent(eventFunctionName, calcRecurringAlarmTime(hoursToRepeat, minutesToRepeat, secondsToRepeat));
  setEvent(firstAlarm, calcDailyAlarmTime(7, 0));
  setEvent(secondAlarm, calcDailyAlarmTime(19, 0));

  //Setup Feed Motor
  feedMotor.setMaxSpeed(500.0);  
  feedMotor.setAcceleration(100.0);

  webPageLog += ("\n\nSystem is ready to feed");
  webPageLog += ("\n___________________________________________");
  webPageLog += "\n\nFeedings Remaining: ";
  webPageLog += feedingsRemaining;
}

void loop() {
	server.handleClient();
	
	//'events()' must be present for time updates and alarms to trigger
	events();
  
  //Disable Feed Motor when not needing to turn
  if (feedMotor.distanceToGo() == 0){
    delay(250);
    feedMotor.disableOutputs();
  } else {
    feedMotor.enableOutputs();
  }
  //Must be present for motor to run
  feedMotor.run();
}

//Daily Alarm Calculations
//==================================================================================================================================
time_t calcDailyAlarmTime(int alarmHour, int alarmMinute){
  //Break current time to components
  time_t timeNow = myTZ.now();
  tmElements_t timeNow_tm;
  breakTime(timeNow, timeNow_tm);
  int8_t minuteNow = timeNow_tm.Minute;
  int8_t hourNow = timeNow_tm.Hour;

  //Setup new variables, seconds always are 0 
  int8_t hoursCalculated;
  int8_t minutesCalculated;
  int8_t secondsCalculated = 0;
  
  //Calculate Hours
  if (hourNow > alarmHour){
    hoursCalculated = (24 - (hourNow - alarmHour));
  }
  if ((hourNow == alarmHour) && (minuteNow < alarmMinute)){
    hoursCalculated = 0;  
  }
  if ((hourNow == alarmHour) && (minuteNow >= alarmMinute)){
    hoursCalculated = 24;
  }
  if (hourNow < alarmHour) {
    hoursCalculated = (alarmHour - hourNow);
  }
  
  //Calculate Minutes
  if (minuteNow > alarmMinute){
    minutesCalculated = (60 - (minuteNow - alarmMinute));
    //Subtract an hour due to minute rollover
    hoursCalculated = hoursCalculated - 1;
  }
  if (minuteNow == alarmMinute){
    minutesCalculated = 0;  
  }
  if (minuteNow < alarmMinute){
    minutesCalculated = (alarmMinute - minuteNow);
  }
  
  time_t alarmTime;
  alarmTime = makeTime((hourNow + hoursCalculated), (minuteNow + minutesCalculated), secondsCalculated, timeNow_tm.Day, timeNow_tm.Month, timeNow_tm.Year);
  //webPageLog += ("\n------------------------\n");
  //webPageLog += (ctime(&timeNow));
  //webPageLog += ("\n");
  //webPageLog += (ctime(&alarmTime));
  return alarmTime;
}

//Recurring Alarm Calculations
//==================================================================================================================================
time_t calcRecurringAlarmTime(int alarmHour, int alarmMinute, int alarmSecond){
  //Break current time to components
  time_t timeNow = myTZ.now();
  tmElements_t timeNow_tm;
  breakTime(timeNow, timeNow_tm);
  int8_t minuteNow = timeNow_tm.Minute;
  int8_t hourNow = timeNow_tm.Hour;
  int8_t secondNow = timeNow_tm.Second;
  int8_t zeroTime = 0;
  int16_t zeroYear = 0;
  if ((alarmHour <= 0) && (alarmMinute <= 0) && (alarmSecond <= 0)){
    time_t alarmTime;
    alarmTime = makeTime(zeroTime, zeroTime, zeroTime, zeroTime, zeroTime, zeroYear);
    webPageLog += ("\nNew Recurring Alarm Set for: ");
    webPageLog += (ctime(&alarmTime));
    return alarmTime;
  } else {
    time_t alarmTime;
    alarmTime = makeTime((hourNow + alarmHour), (minuteNow + alarmMinute), (secondNow + alarmSecond), timeNow_tm.Day, timeNow_tm.Month, timeNow_tm.Year);
    webPageLog += ("\nNew Recurring Alarm Set for: ");
    webPageLog += (ctime(&alarmTime));
    return alarmTime;
  }
}

/*
void recurringTimer(){
  webPageLog += ("\nRecurring Timer Triggered");
  advanceFeeder(255);
  setEvent(recurringTimer, calcRecurringAlarmTime(0, 0, 6));
  }
*/

void firstAlarm(){
  webPageLog += ("\nMorning Alarm Triggered");
  advanceFeeder(255);
  //Must reset this event so it will run again
  setEvent(firstAlarm, calcDailyAlarmTime(7, 0));
}


void secondAlarm(){
  webPageLog += ("\nEvening Alarm Triggered");
  advanceFeeder(255);
  //Must reset this event so it will run again
  setEvent(secondAlarm, calcDailyAlarmTime(19, 0));
}


void advanceFeeder(int numberOfSteps){
    if (feedingsRemaining >= 0){
      feedingsRemaining--;
      webPageLog += ("\nFeeding Fish Now! - Feedings Remaining: ");
      webPageLog += (feedingsRemaining);
      feedMotor.move(numberOfSteps);
    } else {
      webPageLog += ("\nNo Feedings Left!");  
    }
    handleRoot();
}

void handleRoot() {
  server.send(200, "text/plain", webPageLog);
}

// Convert the file extension to the MIME type
String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  return "text/plain";
}

// Send the right file to the client (if it exists)
bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "FishFeederAIO.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}
