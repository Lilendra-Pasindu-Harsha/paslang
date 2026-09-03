# PasLang Embedded Microcontroller & Hardware Control Demo
# Targets ESP32, Raspberry Pi, and Arduino Hardware Controllers.

say "=================================================="
say "   PasLang Embedded Controller Demo (ESP32/RPi)   "
say "=================================================="
say ""

let LED_PIN = 13
let SENSOR_PIN = 34

# 1. Initialize Microcontroller Hardware GPIO Pins
say "Initializing Hardware GPIO Pins..."
pinMode LED_PIN 1
pinMode SENSOR_PIN 0
say ""

# 2. Read Embedded Sensor & Process Signal
say "Reading Sensor Data from Analog Pin 34..."
let raw_val = analogRead SENSOR_PIN
say "Raw Analog Value:"
say raw_val

# Apply Signal Filtering
let filtered_val = mul raw_val 0.0048828
say "Filtered Voltage (V):"
say filtered_val

# 3. Microcontroller Control Logic
if filtered_val > 2.0:
    say "Voltage Threshold Exceeded! Turning ON Alarm LED on Pin 13..."
    digitalWrite LED_PIN 1
else:
    say "Voltage Normal. Keeping Pin 13 LOW..."
    digitalWrite LED_PIN 0

delay 100

say ""
say "Embedded Controller Script Finished!"
