#include <Wire.h>
#include <SPI.h>

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

int image_index;
#define image_x (80)
#define image_y (60)
word image[image_x][image_y];

void setup()
{
  //pinMode(pin, INPUT);           // set pin to input
  //digitalWrite(pin, HIGH);

  Wire.begin();
  Serial.begin(115200);

  pinMode(10, OUTPUT);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(0);

  SPI.begin();

  Serial.println("setup complete");
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

inline void digitalWriteDirect(int pin, boolean val){
  if(val) g_APinDescription[pin].pPort -> PIO_SODR = g_APinDescription[pin].ulPin;
  else    g_APinDescription[pin].pPort -> PIO_CODR = g_APinDescription[pin].ulPin;
}

inline int digitalReadDirect(int pin){
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

void buffer_image(void)
{
  int j = lepton_frame_packet[1];
  for (int i = 0; i < image_x; i++)
  {
    image[i][j] = ((lepton_frame_packet[2*i+4] << 8) + lepton_frame_packet[2*i+5]);//-0x1000;
  }
  // Serial.print("d");
  if (j == 0x3B) print_image();
}

void print_image(void)
{
  int count = 0;
 
  for (int j = 0; j < image_y; j++)
  {
    for (int i = 0; i < image_x; i++)
    {
      count++;
    //  Serial.print(image[i][j],HEX);
  //    Serial.print(",");
    }
  }
  
 // Serial.print(count);
  Serial.println(" ");
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
    Serial.print(lepton_frame_packet[i], DEC);
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


void loop()
{
  int i;
  int reading = 0;
  String debugString;
  Serial.println("beginTransmission");

  set_reg(0);

  read_reg(0x0);

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

  Serial.println("SYS Telemetry Enable State");
  lepton_command(SYS, 0x19>>2 ,GET);
  read_data();

  while (1)
  {
    //lepton_sync();
    read_lepton_frame();
   // if(lepton_frame_packet[0]&0x0f == 0x0f )
    {
    //  print_lepton_frame();
   //   Serial.print(lepton_frame_packet[1],DEC);  
    //  Serial.println(" ");
    
      buffer_image();
    }

  }
  x++;
  delay(10000);
}

