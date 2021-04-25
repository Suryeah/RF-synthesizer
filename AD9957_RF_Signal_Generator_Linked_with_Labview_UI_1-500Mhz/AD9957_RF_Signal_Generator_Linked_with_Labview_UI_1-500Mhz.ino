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

double    FQ, FQ1, FQ2, SFQ, EFQ, STEP = 0;
int       DC, i, DELAY = 0;
uint16_t  REP = 0;
uint32_t  FTW_D ;                    //FREQUNCY TUNE WORD IN DECIMAL
float     FTW_F;
bool      EXIT, INFI = 0;
uint32_t  T1, T2 = 0;

volatile float ASF = 0.56;           //  AMPLITUDE CONTROL FOR DDS1(0.1-1)
float          SA, EA = 0;
float          ASF_R = ASF * 16384;  // ASF DECIMAL IS WRITE IN THE REGISER
uint16_t       ASF_R_D = ASF_R;
char           m;
String         txtMsg = "";

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
}
void getkey()
{
  if (Serial.available() > 0)
  {
    txtMsg = Serial.readStringUntil('\n');
    if (txtMsg[0] == 's')
    {
      EXIT = 1;
      txtMsg = "";
    }
  }
}
void loop()
{
  while (Serial.available() > 0)
  {
    txtMsg = Serial.readStringUntil('\n');
    Serial.println(txtMsg);

    switch (txtMsg[0])
    {
      case 'a':
        {
          FQ, ASF = 0;
          FQ = txtMsg.substring(2, 11).toInt();
          ASF = txtMsg.substring(12, 16).toFloat();
          txtMsg = "";
          mono_Tone();
        } break;

      case 'b':
        {
          SFQ, ASF, EFQ = 0;
          SFQ = txtMsg.substring(2, 11).toInt();
          EFQ = txtMsg.substring(12, 21).toInt();
          STEP = txtMsg.substring(22, 31).toInt();
          DELAY = txtMsg.substring(32, 37).toInt();
          ASF = txtMsg.substring(38, 42).toFloat();
          REP = txtMsg.substring(43, 47).toInt();
          if (REP == 0)
            INFI = 1;
          else
            INFI = 0;
          txtMsg = "";
          freq_Sweep();
        } break;

      case 'c':
        {
          SA = txtMsg.substring(2, 6).toFloat();
          EA = txtMsg.substring(7, 11).toFloat();
          STEP = txtMsg.substring(12, 16).toFloat();
          DELAY = txtMsg.substring(17, 22).toInt();
          FQ = txtMsg.substring(23, 32).toInt();
          REP = txtMsg.substring(33, 37).toInt();
          if (REP == 0)
            INFI = 1;
          else
            INFI = 0;
          txtMsg = "";
          amp_Sweep();
        } break;

      case 'd':
        {
          FQ1 = txtMsg.substring(2, 11).toInt();
          FQ2 = txtMsg.substring(12, 21).toInt();
          T1 = txtMsg.substring(22, 25).toInt();
          T2 = txtMsg.substring(26, 31).toInt();
          ASF = txtMsg.substring(32, 36).toFloat();
          REP = txtMsg.substring(37, 41).toInt();

          Serial.println("FQ1:  " + String(FQ1));
          Serial.println("FQ2:  " + String(FQ2));
          Serial.println("T1:  " + String(T1));
          Serial.println("T2:  " + String(T2));
          Serial.println("ASF:  " + String(ASF));
          Serial.println("REP:  " + String(REP));

          if (REP == 0)
          {  INFI = 1; Serial.println("INFI SET");}
          else
          {  INFI = 0; Serial.println("INFI SET");}
          txtMsg = "";
          FSK();
        } break;

      case 'e':
        {
          SA = txtMsg.substring(2, 6).toFloat();
          EA = txtMsg.substring(7, 11).toFloat();
          T1 = txtMsg.substring(12, 15).toFloat();
          T2 = txtMsg.substring(16, 21).toInt();
          FQ = txtMsg.substring(22, 31).toInt();
          REP = txtMsg.substring(32, 36).toInt();

          if (REP == 0)
          {  INFI = 1; Serial.println("INFI SET");}
          else
          {  INFI = 0; Serial.println("INFI SET");}
          txtMsg = "";
          ASK();
        } break;

      case 's':
        {
          Stop();
        } break;

      default:
        txtMsg = "";
    }
  }
}
void mono_Tone()
{
  ASF_R = ASF * 16384;
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
}
void freq_Sweep()
{
  ASF_R = ASF * 16384;
  ASF_R_D = ASF_R;

  for (int i = 0; i < REP || true; i++)
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
      delay(DELAY);
      getkey();
      if (EXIT || ((i == REP) && (!INFI)))
      {
        Serial.println("In break 1 Loop ");
        EXIT = 1;
        break;     // EXIT Statement to stop signal generation
      }
    }
    if (EXIT)
    {
      Serial.println("In break 2 Loop ");
      EXIT = 0;
      INFI = 0;
      Stop();
      break;
    }
  }
}
void amp_Sweep()
{
  FTW_F = (FQ / Fsck) * 4294967296;
  FTW_D = FTW_F;

  for (i = 0; i < REP || true ; i++)
  {
    for (ASF = SA ; ASF <= EA ; ASF = (ASF + STEP))
    {
      ASF_R = ASF * 16384;
      ASF_R_D = ASF_R;

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
      delay(DELAY);
      getkey();
      if (EXIT || ((i == REP) && (!INFI)))
      {
        EXIT = 1;
        break;  // EXIT Statement to stop signal generation
      }
    }
    if (EXIT)
    {
      EXIT = 0;
      INFI = 0;
      Stop();
      break;
    }
  }
}
void FSK()
{
  Serial.println("REP: " + String(REP));
  for (i = 0; i < REP || true; i++)
  {
    Serial.println(FQ1);

    FTW_F = (FQ1 / Fsck) * 4294967296;
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

    delay(T1);

    Serial.println(FQ2);
    FTW_F = (FQ2 / Fsck) * 4294967296;
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

    delay(T2);

    getkey();
    if (EXIT || ((i == REP) && (!INFI)))
    {
      Stop();
      EXIT = 0;
      INFI = 0;
      break;                // EXIT Statement to stop signal generation
    }
  }
}

void ASK()
{
  FTW_F = (FQ / Fsck) * 4294967296;
  FTW_D = FTW_F;

  for (i = 0; i < REP || true ; i++)
  {
    ASF_R = SA * 16384;
    ASF_R_D = ASF_R;

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

    delay(T1);

    ASF_R = EA * 16384;
    ASF_R_D = ASF_R;

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

    delay(T2);

    getkey();
    if (EXIT || ((i == REP) && (!INFI)))
    {
      Stop();
      EXIT = 0;
      INFI = 0;
      break;                // EXIT Statement to stop signal generation
    }
  }
}
void Stop()
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
}
