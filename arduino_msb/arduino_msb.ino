#define SCOPE_TRIGGER_PIN 2
#define LED_DATA_PIN LED_BUILTIN

unsigned long microS;
unsigned long last_request = 0;       // µS
int poll_delay = 6000;                // µS
int idle_line_timeout = 560;          // µS
unsigned char address = 0x0F;
unsigned char response_byte;
unsigned char response_class;
signed char response_index = -1;
signed short int response_data;
float response_values[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float resolutions[16] = {1, 0.1, 0.1, 0.1, 0.1, 1, 0.1, 0.1, 1, 1, 1, 1, 1, 0.1, 1, 1};
bool led_data_state;

void setup() {
  Serial.begin(115200);
  Serial1.begin(38400);
  pinMode(SCOPE_TRIGGER_PIN, OUTPUT);
  pinMode(LED_DATA_PIN, OUTPUT);
}

void loop() {
  microS = micros();
  // https://www.norwegiancreations.com/2018/10/arduino-tutorial-avoiding-the-overflow-issue-when-using-millis-and-micros/
  if ((unsigned long)(poll_delay < microS - last_request)) {
    last_request = microS;
    digitalWrite(SCOPE_TRIGGER_PIN, LOW);
    address++;
    if (0x0F < address) {
      address = 0;
    }
    Serial1.write(address);
    response_index = 0;
    response_data = 0;
    if (0 == address) {
      for (int i=0; i<16; i++) {
        if (-1 < response_values[i]) {
          led_data_state = !led_data_state;
          digitalWrite(LED_DATA_PIN, led_data_state);
          for (int n=0; n<16; n++) {
            Serial.print(response_values[n]);
            Serial.print("\t");
          }
          Serial.println("");
          break;
        }
      }
    }
  }
  if ((unsigned long)(idle_line_timeout < microS - last_request)) {
    digitalWrite(SCOPE_TRIGGER_PIN, (2 == address));
    while (Serial1.available() && (response_index < 5)) {
      response_byte = Serial1.read();
      if (0 == response_index) {
        response_class = response_byte & 0xF;
      } else if (1 == response_index) {
        response_data = response_byte >> 1;
      } else if (2 == response_index) {
        response_data = ((signed short int)(response_byte << 7)) | response_data;
      }
      if ((response_byte != address) || (0 < response_index)) {
        response_index++;
      }
    }
    if (3 == response_index) {
      response_values[address] = ((float)response_data) * resolutions[response_class];
      digitalWrite(SCOPE_TRIGGER_PIN, LOW);
    } else {
      response_values[address] = -1;
    }
  }
}
