// Wire Controller Reader
// by Nicholas Zambetti [http://www.zambetti.com](http://www.zambetti.com)

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI peripheral device
// Refer to the "Wire Peripheral Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


//#include <Wire.h>

#define NA 0x00
#define ARG 0xFF

#define TELEMETRY_CMD_ID 5

#define DEVICE 2

#define I2CADDR 0x2B

#define N_FUNCS 26

#define invalidOperation WRITE([0x02, NA] 2);

#define getStatus WRITE([0x01, NA], 2); rbCount = 2;
#define getError WRITE([0x03, NA], 2); rbCount = 2;
#define getChsum WRITE([0x05, NA], 2); rbCount = 2;
#define manualReset WRITE([0x80, NA], 2); rbCount = 0;
#define getTelemetry(arg1, arg2) WRITE([0x10, arg1, arg2], 3); rbCount = 2;
#define setCWatchPeriod(arg) WRITE([0x21, arg], 2); rbCount = 0;
#define getCWatchPeriod WRITE([0x20, NA], 2); rbCount = 2;
#define resetCWatch WRITE([0x22, NA], 2); rbCount = 0;
#define getAutoSoftResets WRITE([0x32, NA], 2); rbCount = 2;
#define getCWatchResets WRITE([0x34, NA], 2); rbCount = 2;
#define allPdmSetOn WRITE([0x40, NA], 2); rbCount = 0;
#define allPdmSetOff WRITE([0x41, NA], 2); rbCount = 0;
#define allPdmGetActual WRITE([0x42, NA], 2); rbCount = 4;
#define allPdmGetExpect WRITE([0x43, NA], 2); rbCount = 4;
#define allPdmGetInit WRITE([0x44, NA], 2); rbCount = 4;
#define allPdmInit WRITE([0x45, NA], 2); rbCount = 0;
#define pdmSetOn(Pdm) WRITE([0x50, Pdm], 2); rbCount = 0;
#define pdmSetOff(Pdm) WRITE([0x51, Pdm], 2); rbCount = 0;
#define pdmIniOn(Pdm) WRITE([0x52, Pdm], 2); rbCount = 0;
#define pdmIniOff(Pdm) WRITE([0x53, Pdm], 2); rbCount = 0;
#define pdmGetActual(Pdm) WRITE([0x54, Pdm], 2); rbCount = 2;
#define pdmSetTimerLim(Pdm) WRITE([0x60, Pdm], 2); rbCount = 0;
#define pdmGetTimerLim(Pdm) WRITE([0x61, Pdm], 2); rbCount = 2;
#define pdmGetTimerVal(Pdm) WRITE([0x62, Pdm], 2); rbCount = 2;
#define pdmPCMReset(Pdm) WRITE([0x70, Pdm], 2); rbCount = 0;

/* sends one command to EPS, used mostly b/c sometimes we need to send multiple byte in a row */
#define WRITE(bytes, len) Wire.beginTransmission(DEVICE); Wire.write(bytes, len); tErr=Wire.endTransmission();

/* delay period */
#define DELAY delay(500);

#define BUTTONPROMPT(B, incdec, var, max, prnt, pval, skipCheck) \
if (digitalRead(B ## ButtonPin) == HIGH) { \
  incdec; \
  if (!skipCheck){ \
    if (var < 0) {var = 0;}\
    if (var >= max) {;}\
  }\
}


#define LRBUTTONCHECK(B, incdecvar, var, max, prnt, pval, checkval) \
if (digitalRead(B ## ButtonPin) == HIGH){\
  incdecvar;\
  if (checkval){\
    if (var < 0) { var = 0; }\
    if (var >= max) { var = max-1; }\
  }\
  Serial.print(prnt);\
  Serial.print(pval);\
}

#define LRBUTTONPROMPT(B, incdec, var, max, prnt, pval) \
if (digitalRead( B ## ButtonPin) == HIGH) {\
  if (! B ## ButtonState ){\
    B ## ButtonState = true;\
    var ## incdec;\
    if (var < 0) {var = 0;}\
    if (var >= max) {var = max - 1;}\
    Serial.print(prnt);\
    Serial.println(pval);\
  }\
}\
else { B ## ButtonState = false; }

int returnLen[] = {0, 2, 2, 2, 0, 2, 0, 2, 0, 2, 2, 0, 0, 4, 4, 4, 0, 0, 0, 0, 0, 2, 0, 2, 2, 0};
int argLen[] = {0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1};

byte command_codes[] = {
  0x02, 0x01, 0x03, 0x05, 0x80, 0x10, 0x21, 0x20, 0x22, 0x32, 0x34, 0x40,
  0x41, 0x42, 0x43, 0x44, 0x45, 0x50, 0x51, 0x52, 0x53, 0x54, 0x60, 0x61,
  0x62, 0x70
};

char command_names[][35] = {
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
int cmd;

const int leftButtonPin = 2;
const int selectButtonPin = 3;
const int rightButtonPin = 4;


int ro_idx;


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

void setup() {
  //Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output

  // initialize control pins
  pinMode(rightButtonPin, INPUT);
  pinMode(selectButtonPin, INPUT);
  pinMode(leftButtonPin, INPUT);
  prog_state = READ_INPUT;
}

void loop() {
  int arg_c;
  byte inputs[2] = {0x00, 0x00};
  byte outputs[4];

  Serial.print(READ_INPUT);
  Serial.println(prog_state);


  if (prog_state == READ_INPUT){
    send_packet[1] = 0x00;
    cmd = getCommand();
    getInputs(inputs, cmd);

    //Wire.beginTransmission(I2CADDR);
    arg_c = argLen[cmd] + 1;
    if (arg_c == 1) arg_c++;
   /* Wire.write(send_packet, argLen[cmd] + 1);
    Wire.endTransmission();
    Wire.requestFrom(I2CADDR, 4);*/

    prog_state = READ_OUTPUT;
    rpacket_print_flag = false;
    ro_idx = returnLen[cmd];
    Serial.println("END OF READ INPUT BLOCK");
  }
  else if (prog_state == READ_OUTPUT){
    read_output(cmd);
  }
  /* run func to write command + inputs to I2C line */


  DELAY;
}

int getCommand(void){
  int cmd_id = 0;
  bool complete = false;

  Serial.println("Please input the command you want to use.");
  Serial.println("Use Left and Right buttons to look thru the commands");
  Serial.println("Use Select to select a command");
  Serial.println(cmd_id);

  while (complete == false){
    LRBUTTONCHECK(left, --cmd_id, cmd_id, N_FUNCS, "Current Command: ", command_names[cmd_id], true);
    LRBUTTONCHECK(right, ++cmd_id, cmd_id, N_FUNCS, "Current Command: ", command_names[cmd_id], true);
    LRBUTTONCHECK(select, complete = true, complete, 2, "Selected Command: ", command_names[cmd_id], false);
    DELAY;
  }

  send_packet[0] = cmd_id;

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
        Serial.println(send_packet[idx]);
      }
      else if (digitalRead(rightButtonPin) == HIGH){
        send_packet[idx]++;
        Serial.print(idx+1);
        Serial.print("th byte: ");
        Serial.println(send_packet[idx]);
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
            Serial.print(send_packet[i], HEX);
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
      LRBUTTONPROMPT(left, retry = 1, retry, N_FUNCS, "Chose \"Correct Inputs\"", false);
      LRBUTTONPROMPT(right, retry = 2, retry, N_FUNCS, "Chose \"Redo Inputs\"", false);
      if (digitalRead(selectButtonPin) == HIGH) {
        if (retry == 1) {
          idx++;
          Serial.println("Selected \"Correct Inputs\". Proceeding.");
        }
        else if (retry == 2) {
          idx = argLen[cmd_id] + 1;
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
  int rec_idx = ro_idx; // = Wire.available();

  if (digitalRead(selectButtonPin) == HIGH && selectButtonState == false) {
    Serial.print("Received: [");
    for (int i=0; i<returnLen[cmd]; i++){
      if (i > 0 ) Serial.print(", ");
      Serial.print("0x");
      Serial.print(rec_packet[i], HEX);
    }
    Serial.println("]");
    prog_state = READ_INPUT;
  }
  
  if (rec_idx){
    tmp = rand()%255; /* = Wire.read();*/

    rec_packet[rec_idx--] = tmp;
  }
}


