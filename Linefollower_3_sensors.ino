#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <Servo.h> 
#include <arduino.h>
#include <string.h>

#define directionPinB 13  
#define pwmPinB 11

#define ADAFRUIT_CC3000_IRQ 3  //nettii
#define ADAFRUIT_CC3000_VBAT 5
#define ADAFRUIT_CC3000_CS 10
#define LISTEN_PORT           80
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2);

#define WLAN_SSID       "aalto open"     //network name   
#define WLAN_PASS       "insert password"    //network password
#define WLAN_SECURITY   WLAN_SEC_UNSEC   //Secure type  WLAN_SEC_UNSEC WLAN_SEC_WPA2

Adafruit_CC3000_Server LineServer(LISTEN_PORT);

Servo myservo; // tekee servon
const int suora = 30;
const int steepestright = suora+11;
const int steepestleft = suora-11;
const int steeperright = suora+9;
const int steeperleft = suora-9;
const int valir = suora+7;
const int valiv = suora-7;
const int gradualright = suora+4;
const int gradualleft = suora-4;
const int max1 = 10;
const int min1 = -10;


const int limit = 200;
const int topspeed = 150;
const int mediumspeed =topspeed+10; //topspeed+45; 
const int lowspeed = topspeed +20;//topspeed+65;
const int lowestspeed = 200;//topspeed+85;
int led = 6;
String result;

class engine{
public:
  //engine();
  void straight();
  void leftgradual();
  void rightgradual();
  void leftsteeper();
  void rightsteeper();
  void leftsteepest();
  void rightsteepest();
  void stay();
private:
};

engine motor;

void setup() {
  Serial.begin(115200);
  pinMode(A3,INPUT);  //Sensorien inputit määritetään
  pinMode(A4,INPUT);
  pinMode(A5,INPUT);
  pinMode(led,OUTPUT);  //LEDi
  
  

  pinMode(directionPinB, OUTPUT);
  pinMode(pwmPinB, OUTPUT);
  
  myservo.attach(9);  //Kertoo mihin pinniin servo on laitettu
  myservo.write(suora); // asettaa renkaat suoraan
  
  //Nettiin liittyminen ja muu
  Serial.println(F("\nInitializing..."));    //voi poistaa myöhemmin
  cc3000.begin();
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  cc3000.checkDHCP(); 
  while (!displayConnectionDetails()) {
    delay(1000);
  }
  LineServer.begin();
  Serial.println("S Listening for connections...");   //voi poistaa myöhemmin
  digitalWrite(led, HIGH);  //näkee kun on liittyny nettiin
}

void loop() {
  Adafruit_CC3000_ClientRef client = LineServer.available();
  unsigned int sensor2 = analogRead(A3);  // Luetaan sensoreiden arvoja
  unsigned int sensor1 = analogRead(A4);
  unsigned int sensor3 = analogRead(A5);
  
  Serial.print(sensor1);
  Serial.print(("\t"));
  Serial.print(sensor2);
  Serial.print(("\t"));
  Serial.print(sensor3);
  Serial.println(("\t"));
  
  // kokeiluun serial monitoril
  
  if (client) { // Check if there is data available to read.
     while (client.connected()) {
       while (client.available()) {
         char c = client.read();
         result = result + c;
         Serial.write(c);
         if(result.endsWith("Content-Type: text/html"))
         {
           result="";
         }
         if (c == '\n') 
         {
           serveri(c,client);
           if (result.indexOf("?button1on") >0){
               digitalWrite(led, LOW);
               client.println("<H2>ON</H2>");
           }
           else if (result.indexOf("?button1off") >0){
               digitalWrite(led, HIGH);
               client.println("<H2>OFF</H2>");   
           }
           delay(20);
           client.stop();

           result =""; 
         }
       }
     }
   }
   if (digitalRead(led)==LOW)
   {
     
     ON(sensor1,sensor2,sensor3);
   }
   if (digitalRead(led)==HIGH)
   {
     Serial.print("OFF");
     OFF();
   }
}

void serveri(char c,Adafruit_CC3000_ClientRef client)
{
  Serial.print(c);
  client.println("HTTP/1.1 200 OK"); //send new page
  client.println("Content-Type: text/html");
  client.println();     
  client.println("<HTML>");
  client.println("<HEAD>");
  client.println("<body style='background-color:pink'>");
  client.println("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />");
  client.println("<TITLE>RC Car Line Follower</TITLE>");
  client.println("</HEAD>");
  client.println("<BODY>");
  client.println("<H1>RC Car Line Follower</H1>");
  client.println("<a href=\"/?button1on\"\">Turn On</a>");
  client.println("<a href=\"/?button1off\"\">Turn Off</a><br />");   
  client.println("<br />"); 
  client.println("</BODY>");
  client.println("</HTML>"); 
}

void OFF()
{
  motor.stay();
}

void ON(int sensor1,int sensor2,int sensor3)
{

  // Sensorit esim vasemmalta oikealle numerojärjestyksessä 1 vasen 4 oikein
  
  if(sensor2 > limit && limit > sensor1 && limit > sensor3) // suoraan
  {
    motor.straight();
    myservo.write(suora); //keski/suoraan
   }
   else if(sensor3 > limit && limit > sensor2 && limit > sensor1) //maksimi käännös oikealle max right
  {
    motor.rightsteepest();
    myservo.write(steepestright); //maksimi oikea
  }
  else if(sensor1 > limit && limit > sensor2 && limit > sensor3) //maksimi käännös vasemmalle max left
  {
    motor.leftsteepest();
    myservo.write(steepestleft); //maksimi vasen
    
  }
  else if(sensor1 > limit && sensor2 > limit && limit > sensor3) //väli vasen
  {
    if (sensor1-sensor2 < max1 && sensor1-sensor2 > min1)
    {
      motor.rightsteepest();
      myservo.write(valiv); //keski/suoraan
    }
    else if (sensor1-sensor2 > max1)
    {
      motor.rightsteepest();
      myservo.write(steeperleft); //keski/vasen
    }
    else if (sensor1-sensor2 < min1)
    {
      motor.rightsteepest();
      myservo.write(gradualleft); //keski/oikee
    }
    
  }
  else if(sensor2 > limit && sensor3 > limit && limit > sensor1) //väli oikee
  {
    if (sensor3-sensor2 < max1 && sensor3-sensor2 > min1)
    {
      motor.rightsteepest();
      myservo.write(valir); //keski/suoraan
    }
    else if (sensor3-sensor2 > max1)
    {
      motor.rightsteepest();
      myservo.write(steeperright); //keski/vasen
    }
    else if (sensor3-sensor2 < min1)
    {
      motor.rightsteepest();
      myservo.write(gradualright); //keski/oikee
    }
    
  }

   

}
 
 


void engine::leftgradual(){
  analogWrite(pwmPinB, topspeed);
  digitalWrite(directionPinB, HIGH);
}

void engine::rightgradual(){
  analogWrite(pwmPinB, mediumspeed);
  digitalWrite(directionPinB, HIGH);
}

void engine::leftsteeper(){
  analogWrite(pwmPinB, lowspeed);
  digitalWrite(directionPinB, HIGH);
}

void engine::rightsteeper(){
  analogWrite(pwmPinB, lowspeed);
  digitalWrite(directionPinB, HIGH);
}

void engine::leftsteepest(){
  analogWrite(pwmPinB, lowestspeed);
  digitalWrite(directionPinB, HIGH);
}

void engine::rightsteepest(){
  analogWrite(pwmPinB, lowestspeed);
  digitalWrite(directionPinB, HIGH);
}

void engine::straight(){
  analogWrite(pwmPinB, topspeed);
  digitalWrite(directionPinB, HIGH);
}

void engine::stay()
{
  analogWrite(pwmPinB, 0);
  digitalWrite(directionPinB, HIGH);
}
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
