// Pin definitions
#define ENCODER_PIN_CLK 2  // Rotary encoder CLK pin
#define ENCODER_PIN_DT 3   // Rotary encoder DT pin
#define WARNING_LED 13     // Red warning LED

// Variables to track encoder data and program state
volatile int pulseCount = 0;      // Number of pulses detected by the encoder
unsigned long lastUpdateTime = 0; // Time of the last RPM calculation
float rpm = 0;                    // Calculated RPM value
bool zeroRPMConfirmed = false;    // Flag to track if 0 RPM is confirmed after 3 seconds
bool warningActive = false;       // Flag to indicate if a warning is active
unsigned long zeroRPMStartTime = 0;  // Time when RPM first dropped to 0
unsigned long warningStartTime = 0;  // Time when the warning started

void setup() {
  // Configure rotary encoder pins
  pinMode(ENCODER_PIN_CLK, INPUT_PULLUP); // Enable pull-up for CLK pin
  pinMode(ENCODER_PIN_DT, INPUT_PULLUP);  // Enable pull-up for DT pin

  // Configure warning LED pin
  pinMode(WARNING_LED, OUTPUT);          // Set warning LED as output
  
  // Start serial communication
  Serial.begin(9600);                    // Set baud rate to 9600

  // Attach an interrupt to the CLK pin to count pulses
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_CLK), countPulses, RISING);
}

void loop() {
  unsigned long currentTime = millis(); // Get the current time in milliseconds

  // Calculate RPM every second
  if (currentTime - lastUpdateTime >= 1000) { 
    rpm = (pulseCount / 20.0) * 60.0; // Convert pulses to RPM (adjust PPR if needed)
    pulseCount = 0;                   // Reset pulse count for the next measurement
    lastUpdateTime = currentTime;     // Update the time for the next calculation

    // Print the calculated RPM to the Serial Monitor
    Serial.print("RPM: ");
    Serial.println(rpm);

    // Check if RPM = 0
    if (rpm == 0) {
      if (!zeroRPMConfirmed) {
        zeroRPMConfirmed = true;             // Start zero RPM detection
        zeroRPMStartTime = currentTime;      // Record the time when RPM dropped to 0
      }
    } else {
      // Reset zero RPM and warning states if RPM recovers
      zeroRPMConfirmed = false;
      warningActive = false;
      digitalWrite(WARNING_LED, LOW);        // Turn off the warning LED
    }
  }

  // Check if RPM has been 0 for 3 seconds
  if (zeroRPMConfirmed && !warningActive && (currentTime - zeroRPMStartTime >= 3000)) {
    warningActive = true;                    // Activate warning
    warningStartTime = currentTime;          // Record the warning start time
    digitalWrite(WARNING_LED, HIGH);         // Turn on the warning LED
    Serial.println("WARNING: RPM is 0! Rest or resume cycling.");
  }

  // If warning is active for 5 seconds and RPM hasn't recovered, send EXIT signal
  if (warningActive && (currentTime - warningStartTime >= 5000)) {
    Serial.println("EXIT");                 // Send exit signal to the computer
    warningActive = false;                  // Reset warning state
    digitalWrite(WARNING_LED, LOW);         // Turn off the warning LED
    while (1);                              // Halt the Arduino program
  }
}

// Interrupt service routine to count pulses
void countPulses() {
  pulseCount++; // Increment pulse count on each rising edge of CLK signal
}
