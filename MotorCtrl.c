/*
* MotorCtrl.c
*
* Created: 09.12.2019 09:52:43
*  Author: xxx
*/

#include "AppIncludes.h"



MotorController _motorCtrl =
{
	.start = Floor2,
	.target = Floor0,
	.fsm  = { .Next = 0, .CurrentState = MotorCtrl_Initializing, .RxMask = 0xFF },
};

void NotifyCalibrationDone(uint8_t currentPos, uint8_t targetPostion)
{
	FloorType floor = (FloorType)currentPos/POS_STEPS_PER_FLOOR;
	SetDisplay(floor);
	if( ((currentPos %floor) == 0 ) && floor == Floor0 )
	{
		SendEvent(SignalSourceEnvironment, LiftCalibrated, currentPos, targetPostion);
	}
}

void MotorCtrl_Initializing(Message* msg)
{
	if( msg->Id == LiftStarted)
	{
		SetDoorState(DoorClosed, Floor0);
		SetDoorState(DoorClosed, Floor1);
		SetDoorState(DoorClosed, Floor2);
		SetDoorState(DoorClosed, Floor3);
		
		CalibrateElevatorPosition(NotifyCalibrationDone);
		return;
	}
	if( msg->Id == LiftCalibrated )
	{
			Usart_PutChar(0x19);
			Usart_PutChar(msg->Id);
		SetDisplay(Floor0);
		SetState(&_motorCtrl.fsm, MotorCtrl_Stopped);
		SendEvent(SignalSourceApp, Message_ElevatorReady, Floor0, 0);
	}
}

void OnElevatorPositionChanged(uint8_t currentPos, uint8_t targetPos)
{
	SendEvent(SignalSourceElevator, Message_PosChanged, currentPos, targetPos);
}


void MotorCtrl_Stopped(Message* msg)
{
	Usart_PutChar(0x11);
		Usart_PutChar(msg->Id);
	Usart_PutChar(msg->MsgParamLow);

	if( msg->Id == Message_MoveTo && msg->MsgParamLow < 4)
	{
		Usart_PutChar(0x12);
		_motorCtrl.target = (FloorType)msg->MsgParamLow;
		_motorCtrl.start = (FloorType)msg->MsgParamHigh;
		SetState(&_motorCtrl.fsm, MotorCtrl_Moving);
		MoveElevator(_motorCtrl.target * POS_STEPS_PER_FLOOR, OnElevatorPositionChanged );
		//Set Speed Depending on Distance
		int distance = _motorCtrl.target - _motorCtrl.start;
		
		/*if(distance == 3 || distance == -3){
			SetElevatorSpeed(SpeedType.VeryFast);
		}
		if(distance == 2 || distance == -2){
			SetElevatorSpeed(SpeedType.Fast);
		}
		if(distance == 1 || distance == -1){
			SetElevatorSpeed(SpeedType.Medium);
		}*/
	}
	
	if(msg->Id == OpenDoor){
		_motorCtrl.start = (FloorType)msg->MsgParamLow;
		SetDoorState(DoorOpen, _motorCtrl.start);
	}
	
	if(msg->Id == CloseDoor){
		_motorCtrl.start = (FloorType)msg->MsgParamLow;
		SetDoorState(DoorClosed, _motorCtrl.start);
	}
	
}


void MotorCtrl_Moving(Message* msg)
{
	if( msg->Id == Message_PosChanged && msg->MsgParamLow == msg->MsgParamHigh)
	{
		_motorCtrl.target = (FloorType)msg->MsgParamLow/POS_STEPS_PER_FLOOR;
		SetDoorState(DoorOpen, _motorCtrl.target);
		_motorCtrl.start = _motorCtrl.target;
		SendEvent(SignalSourceApp, SetDoorOpenTimer, _motorCtrl.target, 0);
		SetState(&_motorCtrl.fsm, MotorCtrl_Stopped);
	}
}


