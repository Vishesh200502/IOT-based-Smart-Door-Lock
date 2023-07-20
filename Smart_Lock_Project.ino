#include <ESP8266WiFi.h>
#include<ESP8266HTTPClient.h>

WiFiClient client;
HTTPClient http;
WiFiServer server(80);

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* WIFI settings */
const char* ssid = "";   //WIFI SSID
const char* password = "";    //WIFI PASSWORD

/* data received from application */
String  data = "";

/* define L298N or L293D motor control pins */
int Relay1 = 12;    //D6

#define trigPin D4
#define echoPin D5

int buzzer = 15; //D8

char message[] = "Welcome to SMART LOCK...";
int x,minX;

long duration;
int distance;

void setup()
{
  /* initialize motor control pins. */
  pinMode(Relay1, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(buzzer, OUTPUT);

  digitalWrite(Relay1, HIGH);

  /* start server communication */
  Serial.begin(115200);  //Data Transfer Rate in Bits per Second
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.setTextSize(3);
  display.setCursor(x,20);
  display.print("");
  display.display();
  x = display.width();
  minX = -18 *strlen(message);
    
  server.begin();
}

void loop()
{   
  if(data == "Start")
  {
    initial();
  }

  sensor();
  
  /* If the server available, run the "checkClient" function */
  client = server.available();
  if (!client) return;
  data = checkClient ();
  Serial.print(data);
  /************************ Run function according to incoming data from application *************************/

  if (data == "Unlock")
  {
    digitalWrite(Relay1, LOW);
    door_open();
  }

  else if (data == "Lock")
  {
    digitalWrite(Relay1, HIGH);
    door_close();
  } 

  if(data == "Visitor_Granted")
  {
    visit_yes();
  }

  if(data == "Visitor_Denied")
  {
    visit_no();
  }

  if(data == "Owner_Granted")
  {
    owner_yes();
  }

  if(data == "Owner_Denied")
  {
    owner_no();
  }
  if(data == "Stop")
  {
    final();
  }
}

void initial(void)
{
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(x,20);
  display.print(message);
  display.display();
  x = x - 2;
  if(x < minX)
    x = display.width();
}

void final(void)
{
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(x,20);
  display.print("");
  display.display();
}

void visit_yes(void)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(32,10);
  display.print("ACCESS");
  display.setCursor(32,40);
  display.print("GRANTED!");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(0,10);
  display.print("Hello Visitor!");
  display.setCursor(0,30);
  display.print("The IP ADDRESS is:");
  display.setCursor(0,50);
  display.print(WiFi.localIP());
  display.display();
}

void visit_no(void)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(32,10);
  display.print("ACCESS");
  display.setCursor(32,40);
  display.print("DENIED!");
  display.display();
}

void owner_yes(void)
{
  digitalWrite(buzzer, LOW);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(32,10);
  display.print("ACCESS");
  display.setCursor(32,40);
  display.print("GRANTED!");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(0,10);
  display.print("Welcome Back!");
  display.setCursor(0,30);
  display.print("The IP ADDRESS is:");
  display.setCursor(0,50);
  display.print(WiFi.localIP());
  display.display();
}

void owner_no(void)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(32,10);
  display.print("ACCESS");
  display.setCursor(32,40);
  display.print("DENIED!");
  display.display();
}

void door_open(void)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25,10);
  display.print("DOOR");
  display.setCursor(25,40);
  display.print("UNLOCKED");
  display.display();
}

void sensor(void)
{
  digitalWrite(trigPin, LOW);

  digitalWrite(trigPin, HIGH);

  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distance = duration*0.034/2;

  Serial.print("Distance: ");
  Serial.println(distance);
  if(distance < 5)
  {
    digitalWrite(buzzer, HIGH);
    http.begin(client,"http://api.thingspeak.com/apps/thinghttp/send_request?api_key=4CXKIOU9H91CQB0K");
    http.GET();
    http.end();
    alert();
  }
}
