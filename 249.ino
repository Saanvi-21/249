#include<WiFi.h>
#include<Adafruit_MQTT.h>
#include<Adafruit_MQTT_Client.h>
#include<DHT.h>

//  rgb led details
byte rpin = 25;
byte gpin = 26;
byte bpin = 27;
byte rchannel = 0;
byte gchannel = 1;
byte bchannel = 2;
byte resolution = 8;
int frequency = 5000;

//  dht details
byte dht_pin = 4;
#define dht_type DHT11
DHT dht(dht_pin , dht_type);

//  wifi credentials
const char ssid[] = "Saanvi";
const char password[] = "xperia123";

//  io details
#define IO_USERNAME  "saanvi21"
#define IO_KEY       "aio_WoFh97QhZCJ52rfbN37u2QXozwIX"
#define IO_BROKER    "io.adafruit.com"
#define IO_PORT       1883

//  client details
WiFiClient wificlient;
Adafruit_MQTT_Client mqtt(&wificlient , IO_BROKER , IO_PORT , IO_USERNAME , IO_KEY);

Adafruit_MQTT_Subscribe red = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/roomLight");
Adafruit_MQTT_Subscribe green = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/roomLight");
Adafruit_MQTT_Subscribe blue = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/roomLight");

Adafruit_MQTT_Publish dewp = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/dewP");
Adafruit_MQTT_Publish c = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/tempC");
Adafruit_MQTT_Publish f = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/tempF");
Adafruit_MQTT_Publish k = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/tempK");
Adafruit_MQTT_Publish humid = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/Humidity");

void setup()
{
  Serial.begin(115200);

  //  connecting with wifi
  Serial.print("Connecting with : ");
  Serial.println(ssid);
  WiFi.begin(ssid , password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected !");
  Serial.print("IP assigned by AP : ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //  RGB led setup
  ledcSetup(rchannel , frequency , resolution);
  ledcSetup(gchannel , frequency , resolution);
  ledcSetup(bchannel , frequency , resolution);

  //  attaching pins with channel
  ledcAttachPin(rpin , rchannel);
  ledcAttachPin(gpin , gchannel);
  ledcAttachPin(bpin , bchannel);

  //  dht setup
  dht.begin();

  mqtt.subscribe(&red);
  mqtt.subscribe(&green);
  mqtt.subscribe(&blue);  
}

void loop()
{
  //  connecting with server
  mqconnect();

  //  reading values from dht sensor
  float tempc = dht.readTemperature();
  float tempf = dht.readTemperature(true);
  float tempk = tempc + 273.15;
  float humidity = dht.readHumidity();
  float dew_point = (tempc - (100 - humidity) / 5);  //  dew point in celcius

  if (isnan(tempc)  ||  isnan(tempf)  ||  isnan(humidity))
  {
    Serial.println("Sensor not working!");
    delay(1000);
    return;
  }

  //  printing these values on serial monitor
  String val = String(tempc) + " *C" + "\t" + String(tempf) + " *F" + "\t" + String(tempk) + " *K" + "\t" + 
               String(humidity) + " %RH" + "\t" + String(dew_point) + " *C";
  Serial.println(val);
  
   if (!c.publish(tempc)  ||  !f.publish(tempf)  ||  !k.publish(tempk)  ||  !dewp.publish(dew_point)  ||  !humid.publish(humidity))
  {
    Serial.println("Can't publish!");
  }

  Adafruit_MQTT_Subscribe *subscription;
  while (true)
  {
    subscription = mqtt.readSubscription(5000); 
    if (subscription  ==  0) 
    {
      Serial.println("Can't cach feed");
      break;
    }
    else 
    {
      if (subscription  ==  &red)
      {
        String temp = (char *)red.lastread;

        rval = temp.toInt();
        makecolor(rval , gval , bval);
      }
      
      else if (subscription  ==  &green)
      {
        String temp = (char *)green.lastread;

        gval = temp.toInt();
        makecolor(rval , gval , bval);
      }

      else if (subscription  ==  &blue)
      {
        String temp = (char *)blue.lastread;

        bval = temp.toInt();
        makecolor(rval , gval , bval);
      }
    }
  }

  delay(5000);
}

void mqconnect()
{

  if (mqtt.connected())return;

  {
    while (true)
    {
      int connection = mqtt.connect();  //  mqq client has all details of client, port , username, key
      if (connection  ==  0)
      {
        Serial.println("Connected to IO");
        break;  //  connected
      }
      else
      {
        Serial.println("Can't Connect");
        mqtt.disconnect();
        Serial.println(mqtt.connectErrorString(connection));  //  printing error message
        delay(5000);  //  wait for 5 seconds
      }
    }
  }


  //  wait for some time
  delay(5000);
}

void makecolor(byte r , byte g , byte b)
{
  //  printing values
  Serial.print("RED : ");
  Serial.print(r);
  Serial.print('\t');
  Serial.print("GREEN : ");
  Serial.print(g);
  Serial.print('\t');
  Serial.print("BLUE : ");
  Serial.println(b);

  //  writing values
  ledcWrite(rchannel , r);
  ledcWrite(gchannel , g);
  ledcWrite(bchannel , b);
}
