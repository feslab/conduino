#include <Wire.h>

// The user must calibrate the AD5933 system for a known impedance range to determine the gain factor

#define AD5933_ADDR 0x0D
#define AD5245_ADDR 0x2D //B0101101; // << assumes AD0 tied to VCC

const float CLOCK_SPEED = 16.776 * pow(10, 6); // AD5933 has internal clock of 16.776 MHz

const int CONTROL_REGISTER[2] =                    { 0x80, 0x81       }; // see mapping below
const int START_FREQUENCY_REGISTER[3] =            { 0x82, 0x83, 0x84 }; // 24 bits for start frequency
const int FREQ_INCREMENT_REGISTER[3] =             { 0x85, 0x86, 0x87 }; // 24 bits for frequency increment
const int NUM_INCREMENTS_REGISTER[2] =             { 0x88, 0x89       }; //  9 bits for # of increments
const int NUM_SETTLING_CYCLES_REGISTER[2] =        { 0x8A, 0x8B       }; //  9 bits + 1 modifier for # of settling times
const int STATUS_REGISTER[1] =                     { 0x8F             }; // see mapping below
const int TEMPERATURE_DATA_REGISTER[2] =     { 0x92, 0x93       }; // 16-bit, twos complement format
const int REAL_DATA_REGISTER[2] =                  { 0x94, 0x95       }; // 16-bit, twos complement format
const int IMAG_DATA_REGISTER[2] =                  { 0x96, 0x97       }; // 16-bit, twos complement format

// control register map (D15 to D12)
const int DEFAULT_VALUE =                 B0000; // initial setting
const int INITIALIZE =                    B0001; // excite the unknown impedance initially
const int START_SWEEP =             B0010; // begin the frequency sweep
const int INCREMENT =                     B0011; // step to the next frequency point
const int REPEAT =                        B0100; // repeat the current frequency point measurement
const int MEASURE_TEMP =                  B1001; // initiates a temperature reading
const int POWER_DOWN =                    B1010; // VOUT and VIN are connected internally to ground
const int STANDBY =                       B1011; // VOUT and VIN are connected internally to ground

const int BLOCK_WRITE = B1010000;
const int BLOCK_READ =  B1010001;
const int ADDRESS_PTR = 0xB0;// B1011000;

const int validTemperatureMeasurement =  1;
const int validImpedanceData =           2;
const int frequencySweepComplete =       4;

//functions declarations
int checkStatus();
boolean measureTemperature();
byte frequencyCode(float, int);
boolean setStartFrequency(float);
boolean setFrequencyIncrement(float);
boolean setNumberOfIncrements(int);
boolean setNumberOfSettlingTimes(int);
void measureImpedance();
void configureAD5933(int, float, float, int);
int getByte(int);
boolean setByte(int, int);
boolean setControlRegister(int);
void setChannel(int);

boolean setControlRegister2();


const int ledPin = 13;

int reading = 0;
byte rxByte;
int number;
char FirstChar[1];
char SecondChar[1];

int e = 0;
int y;
int ch;
int u;

// Arduino setup:

// D8 = MUX1
// D9 = MUX0


const int mux1 = 8;
const int mux0 = 9;

int channelNumber = 0;

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  pinMode(mux1, OUTPUT);
  pinMode(mux0, OUTPUT);


  configureAD5933(2, // number of settling times
                  1 * pow(10, 5), // start frequency (Hz)
                  1 * pow(10, 4), // frequency increment (Hz)
                  0); // number of increments

  setControlRegister2();

}

void loop()
{
      y = 2;
      Serial.print("Specify number of samples:    ");
      Serial.setTimeout(10);
      
          while (y == 2) {

              if (Serial.available()) {
                y = 1;
               number = Serial.parseInt();
               Serial.println(number);
              }
           }
          
      Serial.println("Start reading..............");
      Serial.println(" ");
      Serial.println("  Ch1               Ch2               Ch3               Ch4  ");
      digitalWrite(ledPin, HIGH);
      delay(10);

      measureImpedance();

      digitalWrite(ledPin, LOW);
      delay(1000);
}







// message sequence:
// to set the memory address:
// 1. slave address
// 2. pointer command
// 3. register address to point to

// to write data (address already set):
// 1. slave address
// 2. block write
// 3. number bytes write
// 4. byte 0, byte 1, byte 2, etc.

// to read data (address is already set):
// 1. slave address with read bit high
// 2. register data is returned

// to read a block of data (address is already set):
// 1. slave address
// 2. block read
// 3. number of bytes
// 4. slave address (read bit high)
// 5. byte 0, byte 1, byte 2, etc.


// control register map (D10 to D9)
// ranges = {2.0 V p-p, 200 mV p-p, 400 mV p-p, 1.0 V p-p}
const int OUTPUT_VOLTAGE[4] = {
  B00, B01, B10, B11
};

// control register map (D11, D8 to D0)
// D11 = no operation
// D8 = PGA gain (0 = x5, 1 = x1) // amplifies the response signal into the ADC
// D7 = reserved, set to 0
// D6 = reserved, set to 0
// D5 = reserved, set to 0
// D4 = reset // interrupts a frequency sweep
// D3 = external system clock, set to 1; internal system clock, set to 0
// D2 = reserved, set to 0
// D1 = reserved, set to 0
// D0 = reserved, set to 0

// number of increments register
// 0x88: D15 to D9 -- don't care
// 0x89: D8 -- number of increments bit 1
// 0x89: D7 to D0 -- number of increments bits 2 through 9; 9-bit integer number stored in binary format

// number of settling times
// 0x8A: D15 to D11 -- don't care
// 0x8A: D10 to D9 -- 2-bit decode
//        0 0 = default
//        0 1 = # of cycles x 2
//        1 0 = reserved
//        1 1 = # of cycles x 4
// 0x8A: D8 -- MSB number of settling times
// 0x8B: D7 to D0 -- number of settling times; 9-bit integer number stored in binary format

// status register


int checkStatus() {
  return (getByte(STATUS_REGISTER[0]) & 7);
}

boolean measureTemperature() {

  setControlRegister(MEASURE_TEMP);

  delay(10); // wait for 10 ms

  if (checkStatus() & 1 == validTemperatureMeasurement) {

    // temperature is available
    int temperatureData = getByte(TEMPERATURE_DATA_REGISTER[0]) << 8;
    temperatureData |= getByte(TEMPERATURE_DATA_REGISTER[1]);
    temperatureData &= 0x3FFF; // remove first two bits

    if (temperatureData & 0x2000 == 1) { // negative temperature

      temperatureData -= 0x4000;
    }

    temperatureData /= 32;

    Serial.print("Current temperature is ");
    Serial.print(temperatureData);
    Serial.println(" degrees Celsius.");

    setControlRegister(POWER_DOWN);

    return true;

  } else {
    return false;
  }
}

// start frequency and frequency increment formula:
byte frequencyCode(float freqInHz, int byteNum)
{
  long value = long((freqInHz / (CLOCK_SPEED / 4)) * pow(2, 27));

  byte code[3];

  code[0] = (value & 0xFF0000) >> 0x10;
  code[1] = (value & 0x00FF00) >> 0x08;
  code[2] = (value & 0x0000FF);

  return code[byteNum];
}

boolean setStartFrequency(float freqInHz)
{



  boolean statusValue;

  for (int n = 0; n < 3; n++) {
    statusValue = setByte(START_FREQUENCY_REGISTER[n], frequencyCode(freqInHz, n));
  }

  return statusValue;

}


boolean setFrequencyIncrement(float freqInHz)
{


  boolean statusValue;

  for (int n = 0; n < 3; n++) {
    statusValue = setByte(FREQ_INCREMENT_REGISTER[n], frequencyCode(freqInHz, n));
  }

  return statusValue;

}


boolean setNumberOfIncrements(int n)
{


  boolean i2cStatus;

  int numIncrements = min(n, 511);

  i2cStatus = setByte(NUM_INCREMENTS_REGISTER[0], numIncrements >> 8);
  i2cStatus = setByte(NUM_INCREMENTS_REGISTER[1], numIncrements & 255);

  return i2cStatus;
}


boolean setNumberOfSettlingTimes(int n)
{



  int decode;
  int numSettlingTimes = min(n, 2047);

  if (n > 1023) {
    decode = 3;
    numSettlingTimes /= 4;
  }
  else if (n > 511) {
    decode = 1;
    numSettlingTimes /= 2;
  }
  else {
    decode = 0;
    numSettlingTimes = n;
  }

  boolean i2cStatus;

  i2cStatus = setByte(NUM_SETTLING_CYCLES_REGISTER[0], (numSettlingTimes >> 8) + (decode << 1));
  i2cStatus = setByte(NUM_SETTLING_CYCLES_REGISTER[0], numSettlingTimes & 255);

  return i2cStatus;

}



void measureImpedance() {


  int real = 0;
  int imag = 0;
  e = 0;
  //0.Inizialize bit D11,D10,D9,D8
  setControlRegister2();

  // 1. place AD5933 in standby mode
  setControlRegister(STANDBY);

  // 2. initialize with start frequency
  setControlRegister(INITIALIZE);


  // 3. start frequency sweep
  setControlRegister(START_SWEEP);

  int rxcontrol = getByte(CONTROL_REGISTER[0]);

      real = getByte(REAL_DATA_REGISTER[0]) << 8;
      real |= getByte(REAL_DATA_REGISTER[1]);
    
        imag = getByte(IMAG_DATA_REGISTER[0]) << 8;
        imag |= getByte(IMAG_DATA_REGISTER[1]);
    

  // start array:



  // 4. poll status register until complete
  //while (checkStatus() < validImpedanceData) {
  //}
  for (u = 0; u < number; u++) { 
    
    for ( ch=1; ch < 5 ; ch++) {

      setChannel(ch);
    setControlRegister(REPEAT);
      delay(6);

      // 5. read values
      real = getByte(REAL_DATA_REGISTER[0]) << 8;
      real |= getByte(REAL_DATA_REGISTER[1]);

        if (real > 0x7FFF) { // negative value
          real &= 0x7FFF;
          real -= 0x10000;
        }
    
        imag = getByte(IMAG_DATA_REGISTER[0]) << 8;
        imag |= getByte(IMAG_DATA_REGISTER[1]);
    
        if (imag > 0x7FFF) { // negative value
          imag &= 0x7FFF;
          imag -= 0x10000;
        }
    
        double magnitude = sqrt(pow(double(real), 2) + pow(double(imag), 2));
        double gain = 2.3 * pow(10, -10); //calibrated with 220kOhm resistor on 1/3/12
    
        double impedance = 1 / (gain * magnitude * 45400 / 1.63);
    
        double phase = atan(double(imag) / double(real)) * 360 / (2 * 3.14) + 47;
    
        double conductivity = 129 / impedance;
    
    
        //Serial.print("Conductivity: ");
        Serial.print(conductivity, 4);
        Serial.print("            ");
    
    
        //  double limit = analogRead(A0);
        //   limit *=5;
        //   limit /=1023;
        //   Serial.print(limit);
        //   Serial.print(" V ");
        //   Serial.println("; ... ");
    
    
    
    
        //Serial.println(" ");


  }
  Serial.println(" ");
  }




  // 8. power-down mode
  setControlRegister(POWER_DOWN);


}


void configureAD5933(int settlingTimes, float startFreq, float freqIncr, int numIncr)
{
  setNumberOfSettlingTimes(settlingTimes);
  setStartFrequency(startFreq);
  setFrequencyIncrement(freqIncr);
  setNumberOfIncrements(numIncr);

}






// How to write or read a byte:
// 1. master device asserts a start condition on SDA
// 2. master sends the 7-bit slave address followed by the write bit (low)
// 3. addressed slave device asserts an acknowledge on SDA
// 4. The master send a register address or pointer command code (1011 0000)
// 5. the slave asserts an acknowledge on SDA
// 6. the master sends a data byte or register address to where the pointer is to point
// 7. the slave asserts an acknowledge on SDA
// 8. the master asserts a stop condition on SDA to end the transaction

int getByte(int address) {

  int rxByte;

  Wire.beginTransmission(AD5933_ADDR);
  Wire.write(ADDRESS_PTR);
  Wire.write(address);
  int i2cStatus = Wire.endTransmission();

  Wire.requestFrom(AD5933_ADDR, 1);

  if (1 <= Wire.available()) {
    rxByte = Wire.read();
  }
  else {
    rxByte = -1;
  }

  return rxByte;

}

boolean setByte(int address, int value) {

  Wire.beginTransmission(AD5933_ADDR);
  Wire.write(address);
  Wire.write(value);
  int i2cStatus = Wire.endTransmission();

  if (i2cStatus)
    return false;
  else
    return true;

}

boolean setControlRegister(int code) {

  int rxByte = getByte(CONTROL_REGISTER[0]);

  rxByte &= 0x0F; // clear upper four bits
  rxByte |= code << 4; // set to 1011

  boolean s = setByte(CONTROL_REGISTER[0], rxByte);

  delay(1);


  return s;

}

boolean setControlRegister2() {

  int rxByte = getByte(CONTROL_REGISTER[0]);

  rxByte &= 0xF0; // clear lower four bits
  rxByte |= B00000011; // set to 0011

  boolean s = setByte(CONTROL_REGISTER[0], rxByte);

  delay(10);

  return s;

}


void setChannel(int num)
{
switch (num) {

  case 1:
  digitalWrite(mux0, HIGH);
  digitalWrite(mux1, HIGH);
  break;

  case 2:
  digitalWrite(mux0, LOW);
  digitalWrite(mux1, HIGH);
  break;

  case 3:
  digitalWrite(mux0, LOW);
  digitalWrite(mux1, LOW);
  break;

  case 4:
  digitalWrite(mux0, HIGH);
  digitalWrite(mux1, LOW);
  break;

}
}
