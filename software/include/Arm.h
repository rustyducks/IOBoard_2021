#pragma once

#include "Dynamixel/DynamixelSerial.h"
#include "VacuumSystem.h"
#include "TeensyStep.h"
#include "ducklink/Communication.h"

#define ARM1_ZDYN_NEUTRAL 512   //500
#define ARM2_ZDYN_NEUTRAL 512

#define ARM1_YDYN_NEUTRAL 512   //500
#define ARM2_YDYN_NEUTRAL 502

#define STEPPER_HOME_SPEED 1000
#define STEPPER_MAX_ACC 10000
#define STEPPER_MAX_SPEED 4000

#define STEP_PER_MM 41.18

#define TIME_STEPPER_DISABLE 100000  // stepper automatically disable after 100s

class ProcHome;

class Arm;

extern Arm arm1;
extern Arm arm2;

class Arm{
public:
Arm(Gpios::Signal pumpPin, Gpios::Signal valvePin, Pressure::Sensor pressureSensor, HardwareSerial* dynamixelSerial, 
    unsigned int zAxisStepPin, unsigned int zAxisDirPin,
    Gpios::Signal zAxisEnablePin, Gpios::Signal zAxisLimitSwitchPin, unsigned int zRotDynamixelId, unsigned int yRotDynamixelId, StepControl& controller, protoduck::ArmID arm_id);

void init();
void loop();

bool isMoving();

void enableZMotor(bool enable);

void grasp();
void drop();

void startPump(bool start){vacuumSystem_.startPump(start);}
void openValve(bool open){vacuumSystem_.openValve(open);}
bool isPumpStarted() {return vacuumSystem_.isPumpOn();}
bool isValveOpen() {return vacuumSystem_.isValveOpen();}
bool isZMotorEnabled() {return !gpios.read(zAxisEnablePin);}
bool zStopStatus() {return gpios.read(zAxisLimitSwitchPin_);}

enum eJoint{
    PRISMATIC_Z = 0,
    REVOLUTE_Z = 1,
    REVOLUTE_Y = 2
};

void sendPositionCommand(const eJoint joint, const float command);
bool isNear(eJoint joint, float precision);
void rotateIdle();
float getPosition(const eJoint joint);
float pressure() {return vacuumSystem_.getPressure();}

protoduck::ArmID getId() { return ARM_ID;}

// void setZPrimsaticSpeed(int speed);
void resetZPrimaticPosition();

protected:

float z_rot_cmd(float input) {
    if (this == &arm1) {
        return input;
    } else {
        return ARM1_ZDYN_NEUTRAL + ARM2_ZDYN_NEUTRAL - input;
    }
}

float y_rot_cmd(float input) {
    if (this == &arm1) {
        return input;
    } else {
        return ARM1_YDYN_NEUTRAL + ARM2_YDYN_NEUTRAL - input;
    }
}

void reset_stepper_timeout() {time_z_cmd = millis();}

DynamixelSerial dynamixel_;
HardwareSerial* dynamixelSerial_;
unsigned int zAxisRotDynamixelId_;
unsigned int yAxisRotDynamixelId_;

VacuumSystem vacuumSystem_;

Stepper zAxisStepper_;
Gpios::Signal zAxisEnablePin;
Gpios::Signal zAxisLimitSwitchPin_;
bool isStepperInSpeedMode_;
StepControl& controller;

uint32_t time_z_cmd;

bool watchZStop;

float last_commands[3];

const protoduck::ArmID ARM_ID;

friend ProcHome;

};

