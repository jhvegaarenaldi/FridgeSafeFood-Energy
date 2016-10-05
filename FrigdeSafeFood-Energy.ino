//Release Passed OK v.1.2
// Author : Jaime Vega Arenaldi
// Country : Chile
// email : jh.vega.arenaldi@gmail.com


#include <WiFi101.h>
#include <ArduinoJson.h>

// REST API params Artik Cloud Connection
char ServerArtik[] = "api.artik.cloud";  
int portTCP = 443; //(port 443 is default for HTTPS)
WiFiSSLClient clientArtik;
String AuthorizationData = "Authorization: Bearer c539ecdcc6514c6ba351a27d16d70fa6";
char buffer[200];

// Parameters WIFI conection
char ssid[] = "FAMILIA";      //  your wifi network SSID (name)
char pass[] = ".Bbeach.123...";  // your password wifi
int status = WL_IDLE_STATUS;

// Variable and Constant used in Program 
int SENSORDOOR = 3; // define the obstacle avoidance sensor interface - MKR1000 pin ~3
int BUZZERPIN = 6; // define the BUZZER  sensor interface - MKR1000 pin 6
int Sensorval ;// define numeric variable asociate a input value obstacle avoidance sensor 
unsigned long currentMillis = 0, previousMillis=0;  // variable used to control elapsed time between open and close door
unsigned signalDuration=300; // lapsed time long BEEP sound
unsigned int CountSendAlert=1; // variable used to control message to Artik
const long intervalBEEP = 20*1000; // => 20 secons Door Open -> Sound Alarm
const int IntervalSendAlert = 1 ; // => variable used to control the Alert interval -> in MINUTES ( It must be set equal to RULE ARTIK CLOUD).
const long intervalMAIL = IntervalSendAlert * 60 * 1000; // => the value (IntervalSendAlert) means minutes Door Open too long -> Message to Artik

void setup ()
{
  Serial.begin(9600);
  pinMode (BUZZERPIN, OUTPUT) ;// define BUZZERPIN as output interface BUZZER
  pinMode (SENSORDOOR, INPUT) ;// define the obstacle avoidance sensor input interface  
  ConnectToWIFI(ssid,pass); // Connect to WIFI function
}

void loop ()
{
  Sensorval = digitalRead (SENSORDOOR) ;// digital interface input data obstacle avoidance sensor
  if (Sensorval == HIGH) // When the obstacle avoidance sensor detects a open/close door
  {
    currentMillis = millis();
    if ( currentMillis - previousMillis >= intervalBEEP )  // Verify elapsed time from open door ( elapsed time 20 seconds Alert Beep)
    {
      Beep(signalDuration);
    } 
//    Verify elapsed time from open door ( elapsed time ( intervalMAIL * CountSendAlert ) minutes send message to Artik Cloud )
    if ( currentMillis - previousMillis >= ( intervalMAIL * CountSendAlert ) ) 
    {
         signalDuration=90;  // Duration sound beep , time in ms 
         Beep(signalDuration);  // Signal audible (Beep) --> Open Door Long Time
         if (status != WL_CONNECTED ) // Retry connect to WIFI if not previus connected WIFI
         {  ConnectToWIFI(ssid,pass); }
         clientArtik.connect(ServerArtik, portTCP); // Connect to Artik Cloud 
         if (!clientArtik.connected()) 
            { 
              Serial.println("Error... No connection to ARTIK Cloud!");
            }
         else
            {
              SendAlertToArtikCloud(CountSendAlert*IntervalSendAlert); // Send Message to Artik Cloud ( elapsed time Open Door )
            }
         CountSendAlert++;
     } 
 } else
  {
    //Door Close Fridge ( Safe state , food OK and efficient energy  OK ).
    previousMillis = millis() ;
    signalDuration=300;
  }
}

void Beep(int duration){ // This function allow play signal audible
      digitalWrite (BUZZERPIN, HIGH) ;//  signal audible ON 
      delay (300) ;// Delay 
      digitalWrite (BUZZERPIN, LOW) ;// signal audible OFF
      delay (duration);
}

void SendAlertToArtikCloud(int tdoor) 
// Function allow Send Data to Artik Cloud
{
      Serial.println("Sending data - "+String(tdoor));
      clientArtik.println("POST /v1.1/messages HTTP/1.1");
      clientArtik.println("Host: api.artik.cloud");
      clientArtik.println("Accept: */*");
      clientArtik.println("Content-Type: application/json");
      clientArtik.println("Connection: close");
      clientArtik.println(AuthorizationData);

       // Automated POST data section
       clientArtik.print("Content-Length: ");
       clientArtik.println(loadBuffer(tdoor)); // loads buffer, returns length
       clientArtik.println();
       clientArtik.println(buffer);
       Serial.println("Data posted");
       clientArtik.stop();      
}


int loadBuffer(int Value ) {  
   StaticJsonBuffer<200> jsonBuffer; // reserve spot in memory

   JsonObject& root = jsonBuffer.createObject(); // create root objects
     root["sdid"] = "7912145b617243a7a21412f9644678c4"; // FIX 
     root["type"] = "message";

   JsonObject& dataPair = root.createNestedObject("data"); // create nested objects
     dataPair["OpenDoor"] = Value;  

   root.printTo(buffer, sizeof(buffer)); // JSON-print to buffer

   return (root.measureLength()); // also return length
 }

int ConnectToWIFI(char *id,char *passw) { 
  /* Function ConnectToWIFI - allow connect to WIFI 
   => Connection Failure to WIFI = 3 BEEP when POWER ON MKR1000
      -> 3 retry failed it means no connection WIFI 
   => Connection OK to WIFI = NOT BEEP when POWER ON MKR1000
*/ 
    int retry=1;
    Serial.println("Connect to Wifi Device.. please wait");
    Serial.print(id);
    while (status != WL_CONNECTED && retry !=3){
       Serial.println("Attempting to connect to WiFi Network");
       status = WiFi.begin(id,passw);
       retry++;
    }
    if (status == WL_CONNECTED ){
       Serial.println("connected to WiFi ");}
    else{
      Serial.println("Error to connect Wifi name = "+String(id));
      Serial.println("ONLY ACTIVE LOCAL ALARM AUDIBLE !! ");
      Beep(10);Beep(10);Beep(10);
        }
   return(status);
 }
