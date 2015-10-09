#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include <math.h>

//TFT Screen:
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

// These are the pins used for the UNO
// for Due/Mega/Leonardo use the hardware SPI pins (which are different)
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 4//10
#define _dc 3//9
#define _rst 2//8
#define SD_CS 5

// Using software SPI is really not suggested, its incredibly slow
//Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _mosi, _sclk, _rst, _miso);
// Use hardware SPI
//Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);




byte x = 0;
#define ADDRESS  (0x2A)

#define AGC (0x01)
#define SYS (0x02)
#define VID (0x03)
#define OEM (0x08)

#define GET (0x00)
#define SET (0x01)
#define RUN (0x02)

#define VOSPI_FRAME_SIZE (164)
byte lepton_frame_packet[VOSPI_FRAME_SIZE];

bool runonce = 0;

const int leptonselectpin = 10;
const int saveimagepin = 7;

int  buttonState = 0;
int lastbuttonState = 0;

word maxval = 0;
word minval = 99999;

int image_index;
#define image_x (80)
#define image_y (60)
word image[image_x][image_y];

int R = 0, G = 0, B = 0;

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  pinMode(leptonselectpin, OUTPUT);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(0);

  SPI.begin();

  pinMode(saveimagepin, INPUT);

  attachInterrupt(digitalPinToInterrupt(7), saveimage, RISING);

  Serial.println("setup complete");

  //tft.begin();

  read_reg(0x2);

  Serial.println("SYS Camera Customer Serial Number");
  lepton_command(SYS, 0x28 >> 2 , GET);
  read_data();

  Serial.println("SYS Flir Serial Number");
  lepton_command(SYS, 0x2 , GET);
  read_data();

  Serial.println("SYS Camera Uptime");
  lepton_command(SYS, 0x0C >> 2 , GET);
  read_data();

  Serial.println("SYS Fpa Temperature Kelvin");
  lepton_command(SYS, 0x14 >> 2 , GET);
  read_data();

  Serial.println("SYS Aux Temperature Kelvin");
  lepton_command(SYS, 0x10 >> 2 , GET);
  read_data();

  Serial.println("OEM Chip Mask Revision");
  lepton_command(OEM, 0x14 >> 2 , GET);
  read_data();

  //Serial.println("OEM Part Number");
  //lepton_command(OEM, 0x1C >> 2 , GET);
  //read_data();

  Serial.println("OEM Camera Software Revision");
  lepton_command(OEM, 0x20 >> 2 , GET);
  read_data();

  Serial.println("AGC Enable");
  //lepton_command(AGC, 0x01  , SET);
  agc_enable();
  read_data();

  Serial.println("AGC READ");
  lepton_command(AGC, 0x00  , GET);
  read_data();

  //Serial.println("SYS Telemetry Enable State");
  //lepton_command(SYS, 0x19>>2 ,GET);
  //read_data();

}

int spi_read_word(int data)
{
  int read_data;
  // take the SS pin low to select the chip:
  digitalWrite(10, LOW);
  //  send in the address and value via SPI:
  read_data = SPI.transfer(data >> 8) << 8;
  read_data |= SPI.transfer(data);
  // take the SS pin high to de-select the chip:
  digitalWrite(10, HIGH);
  return read_data;
}

inline void digitalWriteDirect(int pin, boolean val) {
  if (val) g_APinDescription[pin].pPort -> PIO_SODR = g_APinDescription[pin].ulPin;
  else    g_APinDescription[pin].pPort -> PIO_CODR = g_APinDescription[pin].ulPin;
}

inline int digitalReadDirect(int pin) {
  return !!(g_APinDescription[pin].pPort -> PIO_PDSR & g_APinDescription[pin].ulPin);
}

void read_lepton_frame(void)
{
  int i;
  for (i = 0; i < (VOSPI_FRAME_SIZE / 2); i++)
  {
    digitalWriteDirect(10, LOW);
    //  send in the address and value via SPI:
    lepton_frame_packet[2 * i] = SPI.transfer(0x00);
    lepton_frame_packet[2 * i + 1] = SPI.transfer(0x00);
    // take the SS pin high to de-select the chip:
    digitalWriteDirect(10, HIGH);
  }
}

bool donecapturing = 0;

bool savenewimage = 0;

void testdrawpoints() {

  int border = 12;
  for (int i = border; i < 60 + border; i++) {
    for (int j = border; j < 80 + border; j++) {
      //  tft.drawPixel(i*2, j*2, 255);
      //   tft.drawRect(i*3, j*3,3,3, 255);
    }
  }
}


void buffer_image(void)
{
  int j = lepton_frame_packet[1];
  for (int i = 0; i < image_x; i++)
  {
    image[i][j] = ((lepton_frame_packet[2 * i + 4] << 8) + lepton_frame_packet[2 * i + 5]); //-0x1000;
  }
  if (j == 0x3B ) { //&& savenewimage == 1) {
    print_image();
    scale_image();
    donecapturing = 1;
    savenewimage = 1;
  }
}


int scale(int num) {

  int val = maxval - minval;
  float val2 = val / 255.0;

  // Serial.print(val,DEC);
  //Serial.print(" ");
  // Serial.print((float)val2);
  //  Serial.print(" ");

  int ret = (((num - minval) * (255.0 - 0.0)) / (maxval - minval)) + 0.0;

  //float retval = (num / val2);
  // Serial.print(ret,DEC);
  return ret;
}

void hsvl2rgb(float H, float L)
{
  int var_i;
  float S = 1, V, lightness, var_1, var_2, var_3, var_h, var_r, var_g, var_b;

  V = L * 2;                         // For the "darkness" of the LED
  if ( V > 1 ) V = 1;

  if ( S == 0 )                      //HSV values = 0 ÷ 1
  {
    R = round(V * 255);
    G = round(V * 255);
    B = round(V * 255);
  }
  else
  {
    var_h = H * 6;
    if ( var_h == 6 ) var_h = 0;  //H must be < 1
    var_i = int( var_h ) ;            //Or ... var_i = floor( var_h )
    var_1 = V * ( 1 - S );
    var_2 = V * ( 1 - S * ( var_h - var_i ) );
    var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );

    if ( var_i == 0 ) {
      var_r = V     ;
      var_g = var_3 ;
      var_b = var_1 ;
    }
    else if ( var_i == 1 ) {
      var_r = var_2 ;
      var_g = V     ;
      var_b = var_1 ;
    }
    else if ( var_i == 2 ) {
      var_r = var_1 ;
      var_g = V     ;
      var_b = var_3 ;
    }
    else if ( var_i == 3 ) {
      var_r = var_1 ;
      var_g = var_2 ;
      var_b = V     ;
    }
    else if ( var_i == 4 ) {
      var_r = var_3 ;
      var_g = var_1 ;
      var_b = V     ;
    }
    else {
      var_r = V     ;
      var_g = var_1 ;
      var_b = var_2 ;
    }

    if ( L > 0.5 )         // Adjusting the Lightness (whiteness)
    {
      lightness = ( L - 0.5 ) / 0.5;
      var_r += ( lightness * ( 1 - var_r ) );
      var_g += ( lightness * ( 1 - var_g ) );
      var_b += ( lightness * ( 1 - var_b ) );
    }

    R = round((1 - var_r) * 255);   // RGB results = 0 ÷ 255. Reversed for common anode RGB LED's
    G = round((1 - var_g) * 255);
    B = round((1 - var_b) * 255);
  }
}

void scale_image(void)
{
  delay(250);
  Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);
  delay(250);
  tft.begin();
  delay(250);

  File    saveFile;
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
    return;
  }
  else {
    Serial.println("OK!");

    //find most recent file number, and increment:
    bool savefinished = 0;
    int fileid = -1;

    String str = "";

    Serial.println("FILE: ");
    while (savefinished == 0) {
      fileid++;
      str = "output" + String(fileid) + ".lrf";

      Serial.println(str);

      if ((saveFile = SD.open(str)) == NULL) {
        Serial.println("File not found"); //create
        savefinished = 1;
      }
      else //If file is found, close, don't overwrite:
        saveFile.close();


    }
    saveFile = SD.open(str, FILE_WRITE);
    Serial.println(str);



  }

  for (int j = 0; j < image_y; j++)
  {
    for (int i = 0; i < image_x; i++)
    {
      int num = scale(image[i][j]);

      //    Serial.print(num, DEC);
      //     Serial.print(" ");
      //   tft.drawRect(j*4+15,i*4+15,4,4,6);

      //convert num to the correct scale:
      float col = (((num - 0) * (1.0 - 0.0)) / (255 - 0)) + 0.0;

      //   Serial.print(col, DEC);
      //   Serial.print(" ");

      //      Serial.println((((t - 0) * (1.0 - 0.0)) / (255 - 0)) + 0.0);
      //      Serial.println((((h - 0) * (1.0 - 0.0)) / (255 - 0)) + 0.0);

      hsvl2rgb(col, 0.5);

      int colval = tft.Color565(R, G, B);

      // tft.drawRect(j * 4 + 20, i * 4 + 20, 4, 4, colval);
      tft.drawRect(j * 4, i * 4 , 4, 4, colval);

      if (saveFile) {
        saveFile.print(num, DEC);
        saveFile.print(" ");
      }
      else
        Serial.print("Error!");

    }
  }
  Serial.println();
  saveFile.close();
}

void print_image(void)
{
  for (int j = 0; j < image_y; j++)
  {
    for (int i = 0; i < image_x; i++)
    {
      if (image[i][j] < minval)
        minval = image[i][j];

      if (image[i][j] > maxval)
        maxval = image[i][j];

      //     Serial.print(image[i][j],DEC);
      //     Serial.print(" ");
    }
  }
  runonce = 1;

  //Serial.println("PRINTLN");
  // Serial.println("Min/Max: ");
  // Serial.print(minval);
  //  Serial.print(" ");
  //  Serial.print(maxval);
  //  Serial.println();
}

void lepton_sync(void)
{
  int i;
  int data = 0x0f;

  digitalWriteDirect(10, HIGH);
  delay(185);
  while (data & 0x0f == 0x0f)
  {
    digitalWriteDirect(10, LOW);
    data = SPI.transfer(0x00) << 8;
    data |= SPI.transfer(0x00);
    digitalWriteDirect(10, HIGH);

    for (i = 0; i < ((VOSPI_FRAME_SIZE - 2) / 2); i++)
    {
      digitalWriteDirect(10, LOW);
      SPI.transfer(0x00);
      SPI.transfer(0x00);
      digitalWriteDirect(10, HIGH);
    }
  }
}

void print_lepton_frame(void)
{
  int i;
  for (i = 0; i < (VOSPI_FRAME_SIZE); i++)
  {
    Serial.print(lepton_frame_packet[i], HEX);
    Serial.print(",");
  }
  Serial.println(" ");
}

void lepton_command(unsigned int moduleID, unsigned int commandID, unsigned int command)
{
  byte error;
  Wire.beginTransmission(ADDRESS);

  // Command Register is a 16-bit register located at Register Address 0x0004
  Wire.write(0x00);
  Wire.write(0x04);

  if (moduleID == 0x08) //OEM module ID
  {
    Wire.write(0x48);
  }
  else
  {
    Wire.write(moduleID & 0x0f);
  }
  Wire.write( ((commandID << 2 ) & 0xfc) | (command & 0x3));

  error = Wire.endTransmission();    // stop transmitting
  if (error != 0)
  {
    Serial.print("error=");
    Serial.println(error);
  }
}

void agc_enable()
{
  byte error;
  Wire.beginTransmission(ADDRESS); // transmit to device #4
  Wire.write(0x01);
  Wire.write(0x05);
  Wire.write(0x00);
  Wire.write(0x01);

  error = Wire.endTransmission();    // stop transmitting
  if (error != 0)
  {
    Serial.print("error=");
    Serial.println(error);
  }
}

void set_reg(unsigned int reg)
{
  byte error;
  Wire.beginTransmission(ADDRESS); // transmit to device #4
  Wire.write(reg >> 8 & 0xff);
  Wire.write(reg & 0xff);            // sends one byte

  error = Wire.endTransmission();    // stop transmitting
  if (error != 0)
  {
    Serial.print("error=");
    Serial.println(error);
  }
}

//Status reg 15:8 Error Code  7:3 Reserved 2:Boot Status 1:Boot Mode 0:busy

int read_reg(unsigned int reg)
{
  int reading = 0;
  set_reg(reg);

  Wire.requestFrom(ADDRESS, 2);

  reading = Wire.read();  // receive high byte (overwrites previous reading)
  //Serial.println(reading);
  reading = reading << 8;    // shift high byte to be high 8 bits

  reading |= Wire.read(); // receive low byte as lower 8 bits
  Serial.print("reg:");
  Serial.print(reg);
  Serial.print("==0x");
  Serial.print(reading, HEX);
  Serial.print(" binary:");
  Serial.println(reading, BIN);
  return reading;
}

int read_data()
{
  int i;
  int data;
  int payload_length;

  while (read_reg(0x2) & 0x01)
  {
    Serial.println("busy");
  }

  payload_length = read_reg(0x6);
  Serial.print("payload_length=");
  Serial.println(payload_length);

  Wire.requestFrom(ADDRESS, payload_length);
  //set_reg(0x08);
  for (i = 0; i < (payload_length / 2); i++)
  {
    data = Wire.read() << 8;
    data |= Wire.read();
    Serial.println(data, HEX);
  }
}

void captureimage() {
  while (donecapturing == 0)
  {
    // lepton_sync();
    read_lepton_frame();
    // if(lepton_frame_packet[0]&0x0f == 0x0f )
    {
      // print_lepton_frame();
      // Serial.print(lepton_frame_packet[1],HEX);
      // Serial.println(" ");
      buffer_image();
    }
  }
}

volatile int state = LOW;

void saveimage() {

  Serial.println("SAVING IMAGE");

  state = HIGH;
  //  while (1) {
  //  read_lepton_frame();
  //buffer_image();
  // }

}


void loop()
{
  //lepton_sync();

  delay(250);
  
  state = LOW;
  donecapturing = 0;
  while (donecapturing == 0) {

    // scale
    read_lepton_frame();
    // if(lepton_frame_packet[0]&0x0f == 0x0f )
    {
      buffer_image();
    }

  }
  while (1) {};
}

