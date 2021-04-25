#include<SPI.h>

#define   pinUpdate(pin) { digitalWrite(IO_UPDATEPIN,LOW); delayMicroseconds(1); digitalWrite(IO_UPDATEPIN,HIGH); delayMicroseconds(1); digitalWrite(IO_UPDATEPIN,LOW); }
#define   pinReset(pin)  { digitalWrite(MASTER_R,HIGH); delayMicroseconds(1); digitalWrite(MASTER_R,LOW); }

#define   IO_UPDATEPIN   PA2
#define   MASTER_R       PA3
#define   pinSPI_SS      PA4
#define   pinSPI_CLK     PA5
#define   pinSPI_MISO    PA6
#define   pinSPI_MOSI    PA7

#define   Fsck           1440000000

double    FQ, SFQ, EFQ = 0;
int       STEP, REP, DELAY;
uint32_t  FTW_D ;//FREQUNCY TUNE WORD IN DECIMAL
float     FTW_F;

volatile float     ASF = 0.99994;       //  AMPLITUDE CONTROL FOR DDS1(0.1-1)
float     ASF_R = ASF * 16384; // ASF DECIMAL IS WRITE IN THE REGISER
uint16_t  ASF_R_D = ASF_R;
char m;

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(60000000L, MSBFIRST, SPI_MODE0));
  pinMode(pinSPI_SS, OUTPUT);
  pinMode(IO_UPDATEPIN, OUTPUT);
  pinMode(MASTER_R, OUTPUT);
  pinReset(MASTER_R);

  //REGISTER 1  CFR1
  digitalWrite(pinSPI_SS, LOW); //
  SPI.transfer(0x00);
  SPI.transfer(0x01);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.endTransaction();
  pinUpdate(IO_UPDATEPIN);
  digitalWrite(pinSPI_SS, HIGH);

  //REGISTER 2 CFR2
  digitalWrite(pinSPI_SS, LOW);
  SPI.transfer(0x01);
  SPI.transfer(0x01);
  SPI.transfer(0x40);
  SPI.transfer(0x00);
  SPI.transfer(0x20);
  SPI.endTransaction();
  pinUpdate(IO_UPDATEPIN);
  digitalWrite(pinSPI_SS, HIGH);

  // //REGISTER 3 CFR3
  digitalWrite(pinSPI_SS, LOW);
  SPI.transfer(0x02);
  SPI.transfer(0x1D);
  SPI.transfer(0x3F);
  SPI.transfer(0x41);
  SPI.transfer(0x48);
  SPI.endTransaction();
  pinUpdate(IO_UPDATEPIN);
  digitalWrite(pinSPI_SS, HIGH);

  //REGISTER 4  DAC
  digitalWrite(pinSPI_SS, LOW);
  SPI.transfer(0x03);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x7F);
  SPI.transfer(0xFF);//127 FF=255;
  SPI.endTransaction();
  pinUpdate(IO_UPDATEPIN);
  digitalWrite(pinSPI_SS, HIGH);

  Serial.println("Select Modes : 1 / 2");
}
void loop()
{
  Serial.println("Select Modes : 1 / 2 / 3");
  while (!Serial.available()) {}
  m = Serial.read();

  switch (m)
  {
    case '1':
      {
        Serial.print("Enter Frequency[MHz]: ");
        while (!Serial.available()) {}
        FQ = Serial.parseInt();
        Serial.println(FQ);

        Serial.print("Enter Amplitude[mA] : ");
        while (!Serial.available()) {}
        ASF = Serial.parseFloat();
        Serial.println(ASF, 5);

        Serial.println("Generating Signal ...");
        delay(10);

        ASF_R = ASF * 16384;                 // 2^128 Scaling Factor
        ASF_R_D = ASF_R;

        FTW_F = (FQ / Fsck) * 4294967296;
        FTW_D = FTW_F;
        digitalWrite(pinSPI_SS, LOW);
        SPI.transfer(0x0E);
        SPI.transfer((ASF_R_D >> 8) & 0xFF);
        SPI.transfer((ASF_R_D >> 0) & 0xFF);
        SPI.transfer(0x00);
        SPI.transfer(0x00);
        SPI.transfer((FTW_D >> 24) & 0xFF);
        SPI.transfer((FTW_D >> 16) & 0xFF);
        SPI.transfer((FTW_D >> 8) & 0xFF);
        SPI.transfer((FTW_D >> 0) & 0xFF);
        SPI.endTransaction();
        pinUpdate(IO_UPDATEPIN);
        digitalWrite(pinSPI_SS, HIGH);
        delayMicroseconds(100);
      } break;


    case '2':
      {
        Serial.print("Enter Start Frequency[MHz]: ");
        while (!Serial.available()) {}
        SFQ = Serial.parseInt();
        Serial.println(SFQ);

        Serial.print("Enter End Frequency[MHz]: ");
        while (!Serial.available()) {}
        EFQ = Serial.parseInt();
        Serial.println(EFQ);

        Serial.print("Set Step Count: ");
        while (!Serial.available()) {}
        STEP = Serial.parseInt();
        Serial.println(STEP);

        Serial.print("Iterations of SWEEP : ");
        while (!Serial.available()) {}
        REP = Serial.parseInt();
        Serial.println(REP);

        Serial.print("Delay in SWEEP[mS] : ");
        while (!Serial.available()) {}
        DELAY = Serial.parseInt();
        Serial.println(DELAY);

        Serial.print("Enter Amplitude[mA] : ");
        while (!Serial.available()) {}
        ASF = Serial.parseFloat();
        Serial.println(ASF, 5);

        Serial.println("Generating Signal ...");

        ASF_R = ASF * 16384;              // 2^128 Scaling Factor
        ASF_R_D = ASF_R;

        for (int i = 1; i <= REP; i++)
        {
          for (FQ = SFQ ; FQ <= EFQ ; FQ = (FQ + STEP))
          {
            FTW_F = (FQ / Fsck) * 4294967296;
            FTW_D = FTW_F;
            digitalWrite(pinSPI_SS, LOW);
            SPI.transfer(0x0E);
            SPI.transfer((ASF_R_D >> 8) & 0xFF);
            SPI.transfer((ASF_R_D >> 0) & 0xFF);
            SPI.transfer(0x00);
            SPI.transfer(0x00);
            SPI.transfer((FTW_D >> 24) & 0xFF);
            SPI.transfer((FTW_D >> 16) & 0xFF);
            SPI.transfer((FTW_D >> 8) & 0xFF);
            SPI.transfer((FTW_D >> 0) & 0xFF);
            SPI.endTransaction();
            pinUpdate(IO_UPDATEPIN);
            digitalWrite(pinSPI_SS, HIGH);
            delayMicroseconds(DELAY);
          }
          Serial.println(i);
        }
        FQ = 0;
        FTW_F = (FQ / Fsck) * 4294967296;
        FTW_D = FTW_F;
        digitalWrite(pinSPI_SS, LOW);
        SPI.transfer(0x0E);
        SPI.transfer((ASF_R_D >> 8) & 0xFF);
        SPI.transfer((ASF_R_D >> 0) & 0xFF);
        SPI.transfer(0x00);
        SPI.transfer(0x00);
        SPI.transfer((FTW_D >> 24) & 0xFF);
        SPI.transfer((FTW_D >> 16) & 0xFF);
        SPI.transfer((FTW_D >> 8) & 0xFF);
        SPI.transfer((FTW_D >> 0) & 0xFF);
        SPI.endTransaction();
        pinUpdate(IO_UPDATEPIN);
        digitalWrite(pinSPI_SS, HIGH);
        delayMicroseconds(10);
      } break;

    case '3':
      {
        FQ = 0;
        FTW_F = (FQ / Fsck) * 4294967296;
        FTW_D = FTW_F;
        digitalWrite(pinSPI_SS, LOW);
        SPI.transfer(0x0E);
        SPI.transfer((ASF_R_D >> 8) & 0xFF);
        SPI.transfer((ASF_R_D >> 0) & 0xFF);
        SPI.transfer(0x00);
        SPI.transfer(0x00);
        SPI.transfer((FTW_D >> 24) & 0xFF);
        SPI.transfer((FTW_D >> 16) & 0xFF);
        SPI.transfer((FTW_D >> 8) & 0xFF);
        SPI.transfer((FTW_D >> 0) & 0xFF);
        SPI.endTransaction();
        pinUpdate(IO_UPDATEPIN);
        digitalWrite(pinSPI_SS, HIGH);
        delayMicroseconds(10);
        Serial.println("Signal Stopped .");
      } break;

    default:
      {
        Serial.print("Reacived : ");
        Serial.println(m);
        Serial.println("Enter an Valid Option");
      } break;
  }
}
