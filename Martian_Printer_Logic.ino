class MartianStepper {
  const int STEP_DELAY = 1; // time (mircoseconds) motor is on; 1 mircos is fastest repsonse
  int dirPin;               // motor driver direction pin
  int stepPin;              // motor driver step pin

  float inTravel = 1.0f;    // Percent of steps to adjust, initially 100%
  long steps;               // max steps to be taken by this motor
  int rpm;                  // revolutions per minute
  long pulseDelay;          // time (mircoseconds) until motor turns on; a speed variable 
  unsigned long prev = 0;   // helper variable to store elapsed time (microseconds)
  unsigned long count;               // counter to indicate how many steps have been taken
 
  public: 
    /**
     * Constructor for a Martian Printer Stepper Motor
     * 
     * int stepPin: the driver's step pin
     * int dirPin: the dirver's direction pin
     * int enablePin: the driver's enable pin
     * int rpm: initial motor speed in rpm
     * int steps: initial maximum steps that can be taken
     */
    MartianStepper::MartianStepper(int stepPin, int dirPin, int enablePin, int rpm, long steps) {
      this->dirPin = dirPin;
      this->stepPin = stepPin;
      pinMode(stepPin, OUTPUT);
      pinMode(dirPin, OUTPUT);
      pinMode(enablePin, OUTPUT);
  
      digitalWrite(dirPin, LOW);
      digitalWrite(enablePin, LOW);
  
      this->steps = steps * inTravel;
      this->rpm = rpm;
      pulseDelay = ((60L * 1000L * 1000L) / (200L * rpm)) - STEP_DELAY;
    } // end constructor
  
    /**
     * Method to set the percentage of steps travelled
     * 
     * int inTravel: the new percentage of steps to travel
     */
    void setInTravel(float inTravel) {
      this->inTravel = inTravel;
      steps = steps * inTravel;
      Serial.print("Inches to travel: "); Serial.println(inTravel);
    } // end setInTravel()
  
    /**
     * Method to set the motor's speed in rpm
     * 
     * int rpm: the new speed of the motor
     */
    void setMotorSpeed(int rpm) {
      this->rpm = rpm;
      pulseDelay = ((60L * 1000L * 1000L) / (200L * rpm)) - STEP_DELAY;
      Serial.print("Speed (in rpm): "); Serial.println(rpm);
    } // end setMotorSpeed()

    /**
     * Method to return the motor's speed
     * 
     * returns the motor's speed in rpm
     */
    int getMotorSpeed() {
      return rpm;
    }
  
    /**
     * Method to set the maximum number of steps this motor can take
     * 
     * int steps: the new maximum steps that can be taken by this motor
     */
    void setSteps(long steps) {
      this->steps = steps * inTravel;
      Serial.print("Maximum steps to take: "); Serial.println(steps);
    } // end setSteps()
  
    /**
     * Method to return the maximum number of steps this motor can take
     * 
     * returns the maximum steps that this motor can take
     */
    long getSteps() {
      return steps;
    } // end getSteps()
  
    /**
     * Method to return the current steps taken, given by the variable count
     * 
     * returns the current amount of steps taken
     */
    long getCount() {
      return count;
    } // end getCount()
  
    /**
     * Method to reset count to 0, indicating that no steps have been taken since the reset
     */
    void resetCount() {
      count = 0;
    } // end resetCount()
  
    /**
     * Method to move the motor forward, incrementing the number of steps taken each time
     */
    void forward() {
      // state machine logic for simultaneous motor usage
      digitalWrite(dirPin, LOW);
      digitalWrite(stepPin, HIGH);
      if (micros() - prev > pulseDelay) {
        digitalWrite(stepPin, LOW);
        delayMicroseconds(STEP_DELAY);
        prev = micros();
        count++;
      }
    } // end forward()
  
    /**
     * Method to move the motor backwards, incrementing the number of steps taken each time
     */
    void reverse() {
      // state machine logic for simultaneous motor usage
      digitalWrite(dirPin, HIGH);
      digitalWrite(stepPin, HIGH);
      if (micros() - prev > pulseDelay) {
        digitalWrite(stepPin, LOW);
        delayMicroseconds(STEP_DELAY);
        prev = micros();
        count++;
      }
    } // end reverse()
}; // end MartianStepper class

///////////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
///////////////////////////////////////                          \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
///////////////////////////////////////  MARTIAN PRINTER LOGIC   \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
///////////////////////////////////////                          \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
///////////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

// declare global variables and instantiate motors
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38

#define E_STEP_PIN         26
#define E_DIR_PIN          28
#define E_ENABLE_PIN       24

// For Reference:
// 30rpm xMotor is 1in/s or 26
// 400rpm extruder or 350

MartianStepper extruder(E_STEP_PIN, E_DIR_PIN, E_ENABLE_PIN, 350, 0);
MartianStepper xMotor(X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN, 416, 0);
const long DIST = 2575 * 16L;
long xMax = DIST;
int original;
long x;

/**
 * Set up the program
 */
void setup() {
  Serial.begin(9600);
  long eSteps = 5496L * 16L;      // 5496 steps/rev large gear; 16 rev = 1 inch vertical travel
  long xSteps = xMax;              // 630 steps = 200 mm x-travel
  Serial.print("Extruder: ");
  extruder.setSteps(eSteps);
  Serial.print("xMotor: ");
  xMotor.setSteps(xSteps);
}


/**
 * When initiated, the Arduino will wait for a command, and execute it depending on the 
 * command sent. You can break out of any action by entering ' '/spacebar into the Serial port
 */
void loop() {
  char command = Serial.read();          
  switch (command) {
    case 's':
      Serial.println("Extrude");
      while (extruder.getCount() < extruder.getSteps()) {
        extruder.forward();
        if (Serial.read() == ' ') {
          break;
        }
      }
      extruder.resetCount();
      break;
    case 'w': 
      Serial.println("Retract");
      while (extruder.getCount() < extruder.getSteps()) {
        extruder.reverse();
        if (Serial.read() == ' ') {
          break;
        }
      }
      extruder.resetCount();
      break;
    case 'd':
      Serial.println("Forward");
        while (xMotor.getCount() < xMax) {
          xMotor.forward();
          if (Serial.read() == ' ') {
            xMax = xMax - xMotor.getCount();
            x += xMotor.getCount();
            break;
          }
        }
        if (xMotor.getCount() >= xMax) {
          xMax = DIST;
        }
        xMotor.resetCount();
      break;
    case 'a':
      Serial.println("Reverse");
      while (xMotor.getCount() < x) {
        xMotor.reverse();
        if (Serial.read() == ' ') {
          xMax = xMax + xMotor.getCount();
          x -= xMotor.getCount();
          break;
        }
      }
      xMotor.resetCount();
      break;
    case 'f': 
      Serial.println("Beginning print...");
      while (xMotor.getCount() < xMax) {
          extruder.forward();
          xMotor.forward();
          if (Serial.read() == ' ') {
            xMax = xMax - xMotor.getCount();
            x += xMotor.getCount();
            Serial.println("Print paused.");
            goto bailout;
          }
      }
      delay(500);
      x += xMotor.getCount();
      Serial.println(x);
      extruder.resetCount();
      xMotor.resetCount();
      original = xMotor.getMotorSpeed();
      xMotor.setMotorSpeed(1600);
      while (xMotor.getCount() < x) {
        xMotor.reverse();
      }
      xMax = DIST;
      x = 0;
      xMotor.setMotorSpeed(original);
      Serial.println("Layer complete");
      bailout: // goto pointer
        xMotor.resetCount();
      break;
    case 'p':
      Serial.print("xMotor: ");
      xMotor.setMotorSpeed(xMotor.getMotorSpeed() + 5);
      break;
    case 'o':
      Serial.print("xMotor: ");
      if (xMotor.getMotorSpeed() - 5 > 0) {
        xMotor.setMotorSpeed(xMotor.getMotorSpeed() - 5);
      }
      break;
    case 'i':
      Serial.print("Extruder: ");
      extruder.setMotorSpeed(extruder.getMotorSpeed() + 5);
      break;
    case 'u':
      Serial.print("Extruder: ");
      if (extruder.getMotorSpeed() - 5 > 0) {
        extruder.setMotorSpeed(extruder.getMotorSpeed() - 5);
      }
      break;
  }    
} // end loop()
