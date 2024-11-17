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

char command_names[][50] = {
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
int rbCount = 0; /* stores number of bytes to get from EPS */
int tErr; /* stores last return value of endTransmission */

const int leftButtonPin = 4;
const int selectButtonPin = 5;
const int rightButtonPin = 6;

bool leftButtonState = false;
bool rightButtonState = false;
bool selectButtonState = false;
bool rpacket_print_flag = false;

enum state {READ_INPUT, READ_OUTPUT};

enum state prog_state = READ_INPUT;

byte cmd_args[2] = {0x00, 0x00};
byte results[4] = {0x00, 0x00, 0x00, 0x00};

byte send_packet[3] = {0x00, 0x00, 0x00};
byte rec_packet[4] = {0x00, 0x00, 0x00, 0x00};

int getCommand(void){
  unsigned char cmd_id = 0;
  bool complete = false;

  Serial.println("Please input the command you want to use.");
  Serial.println("Use Left and Right buttons to look thru the commands");
  Serial.println("Use Select to select a command");
  Serial.println(cmd_id);
  while (!complete){
    if (digitalRead(leftButtonPin) == HIGH){
      --cmd_id;
      if (cmd_id >= CMDCOUNT) cmd_id = CMDCOUNT - 1;
      Serial.print("Current Command: ");
      Serial.println(command_names[cmd_id]);
    }
    if (digitalRead(rightButtonPin) == HIGH){
      ++cmd_id;
      if (cmd_id >= CMDCOUNT) cmd_id = CMDCOUNT - 1;
      Serial.print("Current Command: ");
      Serial.println(command_names[cmd_id]);
    }
    if (digitalRead(selectButtonPin) == HIGH){
      complete = true;
      Serial.print("Selected Command: ");
      Serial.println(command_names[cmd_id]);
    }
    DELAY;
  }

  send_packet[0] = command_code[cmd_id];

  return cmd_id;
}

void getInputs(byte output[2], int cmd_id)
{
  int idx = argLen[cmd_id];
  bool askForConf = false;
  int retry = 0;

  Serial.println("Please input numbers to use as inputs");
  Serial.println("Left button decreases the byte value, Right button increases the byte value, Select to go to next byte / stop inputting");

  while (idx > 0){
    if (!askForConf){
      if (digitalRead(leftButtonPin) == HIGH){
        send_packet[idx]--;
        Serial.print(idx+1);
        Serial.print("th byte: ");
        Serial.println(output[idx]);
      }
      else if (digitalRead(rightButtonPin) == HIGH){
        send_packet[idx]--;
        Serial.print(idx+1);
        Serial.print("th byte: ");
        Serial.println(output[idx]);
      }
      if (digitalRead(selectButtonPin) == HIGH){
        if (idx - 1 == 0){
          Serial.println("Are you sure your inputs look good?");
          Serial.print("Inputs to command : \"");
          Serial.print(command_names[cmd_id]);
          Serial.print("\" : [ ");
          for (int i = 0; i < argLen[cmd_id]; i++){
            if (i > 0 ) Serial.print(", ");
            Serial.print("0x");
            Serial.print(output[i], HEX);
          }
          Serial.println(" ]");
          Serial.println("Left for correct inputs, Right for redo inputs, Select to commit to one of these options");
        }
        else { 
          idx--;
          Serial.println("Switched to next byte of input");
        }
      }
    }
    else{
      if (digitalRead(leftButtonPin) == HIGH){
        retry = 1;
        Serial.println("User confirmed inputs. Proceeding");
      }
      else if (digitalRead(rightButtonPin) == HIGH){
        retry = 2;
        Serial.println("User wishes to redo inputs. returning to start of logic.");
      }
      
      if (digitalRead(selectButtonPin) == HIGH) {
        if (retry == 1) {
          idx++;
          Serial.println("Selected \"Correct Inputs\". Proceeding.");
        }
        else if (retry == 2) {
          idx = argLen[cmd_id];
          askForConf = false;
          Serial.println("Selected \"Redo Inputs\". Going to first input.");
        }
        else Serial.println("You must select an option: (left) inputs are correct, proceed | (right) inputs are incorrect, try again");
      }
    }
    DELAY;
  }
  return;
}

void read_output(int cmd) {
  byte tmp;
  int rec_idx = Wire.available();

  if (digitalRead(selectButtonPin) == HIGH && selectButtonState == false) {
      Serial.println();
      prog_state = READ_INPUT;
  }
  
  if (rec_idx){
    tmp = Wire.read();

    rec_packet[rec_idx] = tmp;
  }
  if (!rec_idx && rpacket_print_flag){
    Serial.print("Received: [");
    for (int i=0; i<returnLen[cmd]; i++){
      if (i > 0 ) Serial.print(", ");
      Serial.print("0x");
      Serial.print(rec_packet[i], HEX);
    }
    Serial.println("]");
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
  int cmd;
  int arg_c;
  byte inputs[2] = {0x00, 0x00};
  byte outputs[4];

  if (prog_state = READ_INPUT){
    send_packet[1] = 0x00;
    cmd = getCommand();
    getInputs(inputs, cmd);

    Wire.beginTransmission(I2CADDR);
    arg_c = argLen[cmd] + 1;
    if (arg_c == 1) arg_c++;
    Wire.write(send_packet, argLen[cmd] + 1);
    Wire.endTransmission();
    Wire.requestFrom(I2CADDR, 4);

    prog_state = READ_OUTPUT;
    rpacket_print_flag = false;
  }
  else if (prog_state = READ_OUTPUT){
    read_output(cmd);
  }
  /* run func to write command + inputs to I2C line */


  DELAY;
}

