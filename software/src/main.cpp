#include <Arduino.h>

#include "Arm.h"
#include "Hat.h"
#include "PinLayout.h"
#include "ducklink/Communication.h"
#include "procedures/ProcedureManager.h"
#include "Gpios.h"
#include "Pressure.h"

uint32_t status_time = 0;

protoduck::Message cmd;

unsigned long lastSent = 0;

void setup() {
    gpios.init();
    gpios.setMode(Gpios::LED, OUTPUT);
    communication.init(115200);
    pressure.begin();
    pressure.tare();
    hat.init();
    arm.init();
    procedure_manager.init();

    for(int i=0; i<5; i++) {
        gpios.toggle(Gpios::LED);
        delay(200);
    }
    
}

void loop() {

    arm.loop();
    hat.loop();
    procedure_manager.loop();

    Communication::eMessageStatus msgStatus = communication.checkMessages(cmd);
    if (msgStatus == Communication::eMessageStatus::NEW_MSG) {
        if (cmd.msg_type() == protoduck::Message::MsgType::COMMAND) {
            if (cmd.has_arm()) {
                arm.sendPositionCommand(Arm::eJoint::PRISMATIC_Z, std::round(cmd.arm().traZ()));
                arm.sendPositionCommand(Arm::eJoint::REVOLUTE_Z, std::round(cmd.arm().rotZ()));
                arm.sendPositionCommand(Arm::eJoint::REVOLUTE_Y, std::round(cmd.arm().rotY()));
                arm.startPump(cmd.arm().pump());
                arm.openValve(cmd.arm().valve());
            } else if (cmd.has_hat()) {
                hat.extendTo(cmd.hat().height());
                hat.startPump(cmd.hat().pump());
                hat.openValve(cmd.hat().valve());
            } else if(cmd.has_procedure()) {
                protoduck::Procedure::Proc procedure = cmd.procedure().get_proc();
                auto param = cmd.procedure().get_param();
                auto arm_id = cmd.procedure().get_arm_id();
                procedure_manager.setProcedure(procedure, arm_id, param);
            }
        }
    }

    if(millis() - status_time > 200) {
        communication.sendHatStatus(hat);
        communication.sendArmStatus(arm);
        communication.sendProcedureStatus(procedure_manager);
        status_time = millis();
        gpios.toggle(Gpios::LED);
    }    
}
