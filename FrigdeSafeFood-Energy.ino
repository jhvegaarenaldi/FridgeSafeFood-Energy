//Release Passed OK v.1.2
// Author : Jaime Vega Arenaldi
// Country : Chile
// email : jh.vega.arenaldi@gmail.com


#include <WiFi101.h>
#include <ArduinoJson.h>

// REST API params Artik Cloud Connection
char ServerArtik[] = "api.artik.cloud";   // Artik Cloud Name Server
int portTCP = 443;                        //(port 443 is default for HTTPS)
WiFiSSLClient clientArtik;                // Definition Class WiFiSSLClient
String AuthorizationData = "Authorization: Bearer <INSERT YOU ID>";  // Autorization Id --> please Insert you ID
char buffer[200];                         // buffer is used by send message to Cloud

// Parameters WIFI conection
char ssid[] = "YOU SSID WIFI";      //  your wifi network SSID (name)
char pass[] = "THE PASSWORD WIFI";  // your password wifi
int status = WL_IDLE_STATUS;        // status is used know result WiFi connection (WiFI.begin)

// Variable and Constant used in Program 
int SENSORDOOR = 3; // define the obstacle avoidance sensor interface - MKR1000 pin ~3
int BUZZERPIN = 6; // define the BUZZER  sensor interface - MKR1000 pin 6
int Sensorval ;// define numeric variable asociate a input value obstacle avoidance sensor 
unsigned long currentMillis = 0, previousMillis=0;  // variable used to control elapsed time between open and close door
unsigned signalDuration=300; // lapsed time long BEEP sound
unsigned int CountSendAlert=1; // variable used to control message to Artik
const long intervalBEEP = 20*1000; // => 20 secons Door Open -> Sound Alarm
const int IntervalSendAlert = 5 ; // => variable used to control the elapsed time  -> in MINUTES ( It must be set equal to RULE ARTIK CLOUD definitions).
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
  Sensorval = digitalRead (SENSORDOOR) ;//  input data obstacle avoidance sensor
  if (Sensorval == HIGH) // When the obstacle avoidance sensor detects a open/close door
  {
    currentMillis = millis();  
    if ( currentMillis - previousMillis >= intervalBEEP )  // Verify elapsed time from open door ( elapsed time 20 seconds Alert Beep)
    {
      Beep(signalDuration);  // call function Beep with the duration signal sound 
    } 
// Verify elapsed time from open door ( elapsed time ( intervalMAIL * CountSendAlert ) minutes send message to Artik Cloud )
    if ( currentMillis - previousMillis >= ( intervalMAIL * CountSendAlert ) ) 
    {
         signalDuration=90;             // Duration sound beep , time in ms 
         Beep(signalDuration);          // Signal audible (Beep) --> Open Door Long Time
         if (status != WL_CONNECTED )   // Retry connect to WIFI if not previus connected WIFI
         {  ConnectToWIFI(ssid,pass); }
         clientArtik.connect(ServerArtik, portTCP); // Connect to Artik Cloud 
         if (!clientArtik.connected()) 
            { 
              Serial.println("Error... No connection to ARTIK Cloud!");
            }
         else
            {
              SendAlertToArtikCloud(CountSendAlert*IntervalSendAlert);    // Send Message to Artik Cloud ( elapsed time Open Door )
            }
         CountSendAlert++;
     } 
 } else
  {
    //Door Close Fridge ( Safe state , food OK and efficient energy  OK )
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

void SendAlertToArtikCloud(int tdoor) // Function allow Send Data to Artik Cloud
{
      Serial.println("Sending data - "+String(tdoor));
      // Set Header Message HTTP
      clientArtik.println("POST /v1.1/messages HTTP/1.1");
      clientArtik.println("Host: api.artik.cloud");
      clientArtik.println("Accept: */*");
      clientArtik.println("Content-Type: application/json");
      clientArtik.println("Connection: close");
      clientArtik.println(AuthorizationData);

       // Set POST data section
       clientArtik.print("Content-Length: ");
       clientArtik.println(loadBuffer(tdoor)); // loads buffer, returns length
       clientArtik.println();
       clientArtik.println(buffer);
       Serial.println("Data posted");
       clientArtik.stop();      
}


int loadBuffer(int Value ) {  // Create object JSON 
   StaticJsonBuffer<200> jsonBuffer; // reserve spot in memory

   JsonObject& root = jsonBuffer.createObject(); // create root objects
     root["sdid"] = "<YOU SDID>"; // You Device ID , the value in Device Info --> DEVICE ID 
     root["type"] = "message";

   JsonObject& dataPair = root.createNestedObject("data"); // create nested objects
     dataPair["OpenDoor"] = Value;    // OpenDoor defined in Manifest -> Fields

   root.printTo(buffer, sizeof(buffer)); // JSON-print to buffer

   return (root.measureLength()); // also return length
 }

int ConnectToWIFI(char *id,char *passw) { 
  /* Function ConnectToWIFI - allow connect to WIFI 
   => When connection Failure to WIFI = 3 BEEP when POWER ON MKR1000
      -> IF 3 retry failed , then means no connection WIFI , retry later....
   => When connection to WIFI is OK  = NOT BEEP when POWER ON MKR1000
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
       Serial.println("Connected to WiFi is OK ");}
    else{
      Serial.println("Error to connect Wifi name = "+String(id));
      Serial.println("ONLY ACTIVE LOCAL ALARM AUDIBLE !! ");
      Beep(10);Beep(10);Beep(10);
        }
   return(status);
 }
