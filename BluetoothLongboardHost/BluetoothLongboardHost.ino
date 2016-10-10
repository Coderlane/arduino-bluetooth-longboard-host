#include <Connection.h>
#include <Motor.h>
#include <HardwareBluetoothRN42.h>

// Must be a digital pin
const int motor_pin = 10;      // D10

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

HardwareBluetoothRN42 bluetooth(Serial1,
    status_pin, RN42_MODE_AC_MASTER, "BlueHost");
Connection connection(bluetooth, 1);
Motor motor(motor_pin, 5, 7);

void connection_up();
void connection_lost();
void connection_down();

void setup()
{
  delay(1000);
  analogReference(DEFAULT);
  pinMode(status_led_pin, OUTPUT);
  pinMode(10, OUTPUT);

  motor.setup();

  // Initialize the bluetooth module
  bluetooth.setup();

  // Set the COD
  /*
  bluetooth.enterCommand();
  bluetooth.setCod("08050C");
  bluetooth.exitCommand();
  */
  bluetooth.setTimeout(50);
}

void loop ()
{
  long cur_millis = millis();
  motor.process();
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
  int percent = 0, rc;
  // Connection is up.

  recent_disconnect = 0;
  interval_millis = 100;

  rc = connection.read(&percent);
  if (rc < 0)
    return;

  Serial.print("Requesting: ");
  Serial.println(percent);

  // TODO: Handle malformed requests
  motor.request(percent);

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

  Serial.println("Lost");
  if (recent_disconnect > 1) {
    // If we are down for half a second disable the motor.
    motor.request(0);
  }

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
  Serial.println("Down");
  if (status_led_state != LOW) {
    // Turn off LED
    digitalWrite(status_led_pin, LOW);
    status_led_state = LOW;
  }
}
