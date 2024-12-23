include <ESP32Servo.h>
#include <Bluepad32.h>

Servo escOmni;    // Omni wheel motor
Servo escLeft;    // Left wheel motor
Servo escRight;   // Right wheel motor
Servo escAttack;  // Attack motor

// Motor control pins
const int omniPin = 18;
const int leftPin = 19;
const int rightPin = 20;
const int attackPin = 21;

const float sensitivityPercentageBack = 0.5;
const float sensitivityPercentage =0.5;
// Controller pointer
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// ESC initialization flag
bool escArmed = false;

// Function prototypes
void processJoysticks(ControllerPtr ctl);
void processAttackMotor(ControllerPtr ctl);
void armESC();

void setup() {
  Serial.begin(115200);
  
  escOmni.attach(omniPin, 1000, 2000);   // Attach the ESC for the omni wheel
  escLeft.attach(leftPin, 1000, 2000);    // Attach the ESC for the left wheel
  escRight.attach(rightPin, 1000, 2000);   // Attach the ESC for the right wheel
  escAttack.attach(attackPin, 1000, 2000); // Attach the ESC for the attack motor

  // Initialize Bluepad32
  BP32.setup(&onConnectedController, &onDisconnectedController);
}

void loop() {
  // Update controller data
  bool dataUpdated = BP32.update();
  
  // Process controller input
  if (dataUpdated) {
    for (auto ctl : myControllers) {
      if (ctl && ctl->isConnected()) {
        if (!escArmed) {
          armESC();
          escArmed = true;
        }
        processJoysticks(ctl);
        processAttackMotor(ctl);
      }
    }
  }
  
  delay(20);
}

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      break;
    }
  }
  Serial.println("Controller connected");
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      break;
    }
  }
  Serial.println("Controller disconnected");
}

void processJoysticks(ControllerPtr ctl) {
  // Control the wheels using the joystick
  int yAxisLeft = ctl->axisY(); // Assuming this is the Y-axis for forward/backward
  int xAxisLeft = ctl->axisX(); // Assuming this is the X-axis for left/right
  int xAxisRight = ctl->axisRX();

  // Map the joystick values
  // Joystick range from -512 to 512
  float joystickValue = map(yAxisRight, -512, 512, 1, 10); // Map to 1-10 range
  int escValue = map(joystickValue, 1, 10, 1000, 2000); // Map to ESC PWM range

  //stuff for arcade drive
  int processedLeft = yAxisLeft - (xAxisRight * sensitivityPercentage);
  int processedRight = yAxisLeft + (xAxisRight * sensitivityPercentage);
  int processedBack = xAxisRight * sensitivityPercentageBack

  int mappedLeft = map(processedLeft, -512, 512, 1000, 2000);
  int mappedRight = map(processedRight, -512, 512, 1000, 2000);
  int mappedBack = map(processedBack,-512,512,1000,2000);

  
  // Set speeds for the omni wheel and left/right wheels
  escOmni.writeMicroseconds(mappedBack);
  escLeft.writeMicroseconds(mappedLeft);
  escRight.writeMicroseconds(mappedRight);
}

void processAttackMotor(ControllerPtr ctl) {
  // Control the attack motor using the controller buttons
  static int attackSpeed = map(5, 0, 10, 1000, 2000); // Start at neutral speed

  if (ctl->x()) {
    attackSpeed = map(1, 0, 10, 1000, 2000); // X button gives value of 1
    Serial.println("X button pressed: Speed 1");
  } 
  else if (ctl->a()) {
    attackSpeed = map(3, 0, 10, 1000, 2000); // A button gives value of 3
    Serial.println("A button pressed: Speed 3");
  } 
  else if (ctl->b()) {
    attackSpeed = map(10, 0, 10, 1000, 2000); // B button gives value of 10
    Serial.println("B button pressed: Full speed 10");
  } 
  else if (ctl->y()) {
    attackSpeed = map(0, 0, 10, 1000, 2000); // Y button stops the motor
    Serial.println("Y button pressed: Motor stopped");
  }

  // Set the ESC speed for the attack motor
  escAttack.writeMicroseconds(attackSpeed);
}

// Function to arm the ESC (if necessary)
void armESC() {
  // Send a low signal to the ESC for arming
  Serial.println("Arming ESC...");
  escOmni.writeMicroseconds(map(5, 0, 10, 1000, 2000)); // Neutral throttle value
  escLeft.writeMicroseconds(map(5, 0, 10, 1000, 2000));
  escRight.writeMicroseconds(map(5, 0, 10, 1000, 2000));
  escAttack.writeMicroseconds(1000); // Lowest throttle value for arming
  delay(2000); // Wait 2 seconds
  Serial.println("ESC armed.");
}
