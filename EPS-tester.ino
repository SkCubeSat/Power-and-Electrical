// Wire Controller Reader
// by Nicholas Zambetti [http://www.zambetti.com](http://www.zambetti.com)

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI peripheral device
// Refer to the "Wire Peripheral Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

#define CMDCOUNT 26

#define I2CADDR 0x2B

/* delay period */
#define DELAY delay(500);

int returnLen[] = {0, 2, 2, 2, 0, 2, 0, 2, 0, 2, 2, 0, 0, 4, 4, 4, 0, 0, 0, 0, 0, 2, 0, 2, 2, 0};
int argLen[] = {0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1};

byte command_codes[] = {
  0x02, 0x01, 0x03, 0x05, 0x80, 0x10, 0x21, 0x20, 0x22, 0x32, 0x34, 0x40,
  0x41, 0x42, 0x43, 0x44, 0x45, 0x50, 0x51, 0x52, 0x53, 0x54, 0x60, 0x61,
  0x62, 0x70
};

char *command_names[] = {
  "invalidOperation",
  "getStatus",
  "getError",
  "getChsum",
  "manualReset",
  "getTelemetry",
  "setCommWatchdogPeriod",
  "getCommWatchdogPeriod",
  "resetCommsWatchdog",
  "getAutomaticSoftwareResets",
  "GetCommsWatchdogResets",
  "TurnOnAllPdms",
  "TurnOffAllPdms",
  "GetActualStateOfAllPdms",
  "GetExpectedStateOfAllPdms",
  "SwitchToInitialStateForAllPdms",
  "TurnOnNthPdm",
  "TurnOffNthPdm",
  "TurnOnNthPdmForInitialState",
  "TurnOffNthPdmForInitialState",
  "GetActualStateOfNthPdm",
  "SetNthPdmTimerLimit",
  "GetNthPdmTimerLimit",
  "GetNthPdmTimerCount",
  "PcmReset"
};


int test_counter = 0; /* counter for the current test */
int cmd_id = 0; /* id number for current command */
int rbCount = 0; /* stores number of bytes to get from EPS */
unsigned char result[4]; /* stores actual result */
unsigned char expected[4]; /* stores expected result */
int tErr; /* stores last return value of endTransmission */
int outputSrc = 0; /* stores the current input source, if it's 0 it's I2C, else its channel */
bool readMode = false; /* skips either testSend or testRecieve in the loop */

const int leftButtonPin = 2;
const int selectButtonPin = 3;
const int rightButtonPin = 4;

bool leftButtonState = 0;
bool rightButtonState = 0;

unsigned char get_input(int is_command)
{
  unsigned char input = 0;
  int done = 0;
  while ( ! done )
  {
    if (digitalRead(rightButtonPin) == HIGH)
    {
      input += 1;
    }
    if (digitalRead(leftButtonPin) == HIGH)
    {
      input += 1;
    }
    if (digitalRead(selectButtonPin) == HIGH)
    {
      Serial.println("Are you sure you would like to accept the current input? (left == no, right == yes)");
      Serial.print("Current input is ");
      Serial.println(input, HEX);
      /* wait for it to be unpressed */
      while (1)
      {
        if (digitalRead(leftButtonPin) == HIGH)
        {
          break;
        }
        if (digitalRead(rightButtonPin) == HIGH)
        {
          done = 1;
          break;
        }
      }
    }

    if (is_command)
    {
      if (input > CMDCOUNT)
      {
        input = 0;
      }
      Serial.print("Current Command: ");
      Serial.print(command_codes[input], HEX);
      Serial.print("(");
      Serial.print(command_names[input]);
      Serial.println(")");
    }
    else {
      Serial.println("Current byte: ");
      Serial.println(input, HEX);
    }

    DELAY
  }
  return input;
}

int getCommand(void){
  Serial.println("======================");
  Serial.println("   Getting command");
  Serial.println("======================");
  cmd_id = get_input(1);
  Serial.println("======================");
  Serial.println("     Got Command");
  Serial.println(command_names[cmd_id]);
  Serial.println("======================");

  return cmd_id;
}

void testSend(int command_num) {
  unsigned char payload[2];

  /* get payload bytes from user */
  Serial.println("======================");
  Serial.println(" first parameter byte");
  Serial.println("======================");
  payload[0] = get_input(0);
  Serial.println("======================");
  Serial.println("second parameter byte");
  Serial.println("======================");
  payload[1] = get_input(0);

  Serial.println("======================");
  Serial.println("     Got inputs:");
  Serial.print("         0x");
  Serial.println(payload[0], HEX);
  Serial.print("         0x");
  Serial.println(payload[1], HEX);
  Serial.println("     To Command");
  Serial.println(command_names[cmd_id]);
  Serial.println("======================");

  Serial.println("======================");
  Serial.println("sending message to EPS");
  Serial.println("======================");
  Wire.beginTransmission(I2CADDR);
  Wire.write(payload, 2);
  tErr = Wire.endTransmission();
  Serial.println("======================");
  Serial.println(" message sent to EPS");
  Serial.println("======================");

}

void testRecieve(int test_num){
  int bLeft = Wire.available();
  char c;
  if (bLeft > 0){
    c = Wire.read();
    result[rbCount-bLeft] = c;
    Serial.print(c);
  }
  if (bLeft == 1){
    readMode = false;
  }
}


void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output

  // initialize control pins
  pinMode(rightButtonPin, INPUT);
  pinMode(selectButtonPin, INPUT);
  pinMode(leftButtonPin, INPUT);
}


void loop() {
  getCommand();
  if (!readMode) {
    testSend(cmd_id);
  }
  else {
    testRecieve(cmd_id);
  }
  DELAY
}

