#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);
int sensorInterrupt = 0;  // interrupt 0
int sensorPin       = 2;
#define enA 9 //Digital Pin 9
#define button 3
int pumpingMotor = 7;
int aux = 6; 
unsigned int SetPoint = 400; //400 milileter

float calibrationFactor = 75; 
int system_on = 1;

volatile byte pulseCount = 0;

float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;

unsigned long oldTime = 0;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = { //keypad 
  {'#', 'B', '8', '4'},
  {'#', 'A', '7', '3'},
  {'#', '0', '6', '2'},
  {'#', '9', '5', '1'}
};
byte rowPins[ROWS] = { 53, 52, 51, 50 };
byte colPins[COLS] = { 49, 48, 47, 46 };
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
unsigned long int waterlevel=0;

void setup()
{

  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  lcd.setCursor(0,0);   //Move cursor to character 2 on line 1
  lcd.print("Oxigen Generator");
  
  // Initialize a serial connection for reporting values to the host
  Serial.begin(9600);
  pinMode(enA, OUTPUT);
  pinMode(pumpingMotor, OUTPUT);
  pinMode(aux, OUTPUT);
  digitalWrite(pumpingMotor, HIGH);
  digitalWrite(aux, LOW);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  lcd.setCursor(0,1);   
  lcd.print("Lqd");
   waterlevel=GetNumber();
 
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING); //you can use Rising or Falling
}

void loop()
{
 
  {  
    if ((millis() - oldTime) > 1000)   
    {     
      detachInterrupt(sensorInterrupt);
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
      oldTime = millis();
      flowMilliLitres = (flowRate / 60) * 1000;

      
      totalMilliLitres += flowMilliLitres;

      unsigned int frac;
      Serial.print(totalMilliLitres);
      Serial.print("-");
      Serial.print(waterlevel);
      Serial.print("\t");

      if (totalMilliLitres <= waterlevel)
      {
        lcd.setCursor(3,1);
        lcd.print(waterlevel);
        lcd.setCursor(8,1);
        lcd.print("-");
        lcd.setCursor(9,1);
        lcd.print(totalMilliLitres);
        SetpumpingMotor();
      }
      else {
        int pwmOutput = 0;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("GATA");
        analogWrite(enA, pwmOutput);
      }
      pulseCount = 0;
      attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    }
  }
}

//Insterrupt Service Routine

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void SetpumpingMotor()
{
  int pwmOutput = 255; 
  analogWrite(enA, pwmOutput);
  digitalWrite(pumpingMotor, HIGH);
  digitalWrite(aux, LOW);
}

unsigned long int GetNumber()
{
  int num = 0;
  char key = kpd.getKey();
  while (key != '#')
  {
    switch (key)
    {
      case NO_KEY:
        break;

      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        num = num * 10 + (key - '0');
        break;

      case '*':
        num = 0;
        break;
    }

    key = kpd.getKey();
  }

  return num;
}
