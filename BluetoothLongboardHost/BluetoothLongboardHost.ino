#include <Connection.h>
#include <HardwareBluetoothRN42.h>

// Must be a digital pin
const int left_pwm_pin = 10;   // D10
const int right_pwm_pin = 11;  // D11

const int status_pin = 2;      // D2
const int status_led_pin = 3;  // D3

long cur_millis = 0;
long prev_millis = 0;
long interval_millis = 100;
int status_led_state = LOW;
int recent_disconnect = 0;

const int max_acceleration = 10;
const int min_acceleration = 1;
const int max_power = 100;
const int min_power = 0;
int cur_power = 0;

HardwareBluetoothRN42 bluetooth(Serial1, status_pin, 3, "BlueHost");
Connection connection(bluetooth);

int read_percent();
void request_percent(int percent);

void connection_up();
void connection_lost();
void connection_down();

// Utility
int parseInt(const char *str);
char * strchr(char *str, int c);

int str_to_percent(char *str);

void setup()
{
  delay(1000);
  analogReference(DEFAULT);
  bluetooth.setup();
  bluetooth.setTimeout(10);
  pinMode(status_led_pin, OUTPUT);
}

void loop ()
{
  long cur_millis = millis();
  if (cur_millis - prev_millis > interval_millis) {
    prev_millis = cur_millis;

    if (bluetooth.connected()) {
      connection_up();
    } else if (recent_disconnect < 50) {
      connection_lost();
    } else {
      connection_down();
    }
  }
}

inline void connection_up()
{
  int percent;
  // Connection is up.

  recent_disconnect = 0;
  interval_millis = 100;

  percent = connection.read();
  
  if (status_led_state != HIGH) {
    // Turn on LED
    digitalWrite(status_led_pin, HIGH);
    status_led_state = HIGH;
  }
}

inline void connection_lost()
{
  // Connection recently lost.
  // Attempt to resend quickly :)

  interval_millis = 500;
  recent_disconnect++;

  // Blink LED
  if (status_led_state == LOW) {
    digitalWrite(status_led_pin, HIGH);
    status_led_state = HIGH;
  } else {
    digitalWrite(status_led_pin, LOW);
    status_led_state = LOW;
  }
}

inline void connection_down()
{
  // Connection has been lost for a while.

  interval_millis = 2000;
  if (status_led_state != LOW) {
    // Turn off LED
    digitalWrite(status_led_pin, LOW);
    status_led_state = LOW;
  }
}

// Change a string to a percent.
// str must be char[3] or more.
// The percent should be 0 to 100.
// If it is not, return -1.
int str_to_percent(char *str)
{
  int percent = 0;

  percent += (str[0] - '0') * 100;
  percent += (str[1] - '0') * 10;
  percent += (str[2] - '0');

  if (percent > 100 || percent < 0)
    return -1;

  return percent;
}
