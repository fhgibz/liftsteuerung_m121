/**
 * @file MainController.c
 * @brief Zustandsmaschine für Haupt Steuerung
 *
 * @date  24.11.2019 - Erstellen des templates
 * @author  
 */

#include "LiftSimulationCommon.h" 
#include "LiftLibrary.h"

void SysState_Initializing(Message* msg);
void SysCtrl_Stopped(Message* msg);

typedef struct {
	Fsm fsm;
	FloorType floor;
	uint8_t timer;
} MainController;

static MainController _sysControl = {
	.floor = Floor0,
	.timer = 0xFF,
	.fsm = {
		.CurrentState = SysState_Initializing,
		.Next = 0,
		.RxMask = 0xff
	}
};

void SysState_Initializing(Message* msg){
	if(msg->Id == LiftStarted){
		ClrIndicatorElevatorState(Floor0);
		ClrIndicatorElevatorState(Floor1);
		ClrIndicatorElevatorState(Floor2)
		ClrIndicatorElevatorState(Floor3)
		
		ClrIndicatorFloorState(Floor0);
		ClrIndicatorFloorState(Floor1);
		ClrIndicatorFloorState(Floor2);
		ClrIndicatorFloorState(Floor3);
		
		SetDoorState(DoorClosed, Floor0);
		SetDoorState(DoorClosed, Floor1);
		SetDoorState(DoorClosed, Floor2);
		SetDoorState(DoorClosed, Floor3);
		
		SetState(&_sysControl.fsm, SysCtrl_Stopped);
	}
}
	
void SysCtrl_Stopped(Message* msg){
	if(msg->Id == ButtonEvent){
		
	}

}