/*
* ScheduleController.c
*
* Created: 13/01/2020 08:38:37
*  Author: Nicolas
*/
#include "LiftLibrary.h"
#include "AppIncludes.h"

#define IS_BUTTON_PRESS(msg)			((msg->Id == ButtonEvent) && (msg->MsgParamHigh == Pressed))
#define IS_RESERVATION(buttonMask)		((buttonMask)&0xF0)
#define IS_TARGET_SELECTION(buttonMask)	((buttonMask)&0x0F)
#define countof(array)					(sizeof(array)/sizeof(array[0]))

Boolean Enqueue(FloorType floor);
Boolean Dequeue(FloorType* floor);
void ScheduleCtrl_AwaitElevatorRequest(Message* msg);
FloorType GetFloorReservation(uint8_t msgParam);
FloorType GetTargetSelection(uint8_t msgParam);

typedef struct{
	Fsm fsm;
	uint8_t pendingRequests;
	FloorType ElevatorNextPosQ[4];
	uint8_t qIn;
	uint8_t qOut;
	uint8_t timer;
}ScheduleController;

ScheduleController _scheduleCtrl = {
	.pendingRequests = 0,
	.ElevatorNextPosQ = {0,0,0,0},
	.qIn = 0,
	.qOut = 0,
	.timer = 0,
	.fsm = {.Next = 0, .RxMask = 0xFF, .CurrentState = ScheduleCtrl_AwaitElevatorRequest},
};


void registerScheduler()
{
	RegisterFsm(&_scheduleCtrl.fsm);
}


void ScheduleCtrl_AwaitElevatorRequest(Message* msg)
{
	Usart_PutChar(0x80);
	Usart_PutChar(msg->Id);

	if( IS_BUTTON_PRESS( msg ) )
	{
		Usart_PutChar(0x81);
		if( IS_RESERVATION(msg->MsgParamLow))
		{

			FloorType requestedFloor = GetFloorReservation(msg->MsgParamLow);
			SetIndicatorFloorState(requestedFloor);
			Enqueue(requestedFloor);
			SendEvent(SignalSourceApp, RequestPending, requestedFloor, 0);
		}
		if( IS_TARGET_SELECTION(msg->MsgParamLow))
		{
			FloorType requestedFloor = (FloorType)msg->MsgParamLow;
			SetIndicatorElevatorState(requestedFloor);
			Enqueue((requestedFloor));
			SendEvent(SignalSourceApp, RequestPending, requestedFloor, 0);

		}
		
	}
	
	if(msg->Id == GetNextFloor)
	{
		Usart_PutChar(0x82);
		if(_scheduleCtrl.qIn != _scheduleCtrl.qOut){
			SendEvent(SignalSourceApp, RequestPending, (FloorType)_scheduleCtrl.ElevatorNextPosQ, msg->MsgParamHigh);
		}
		else{
			_scheduleCtrl.timer = StartTimer(1000);
		}
		
	}
}

Boolean Enqueue(FloorType floor)
{
	uint8_t nextIn = (_scheduleCtrl.qIn + 1)%countof(_scheduleCtrl.ElevatorNextPosQ);
	if( nextIn != _scheduleCtrl.qOut)
	{
		_scheduleCtrl.ElevatorNextPosQ[_scheduleCtrl.qIn] = floor;
		_scheduleCtrl.qIn = nextIn;
		return true;
	}
	return false;
}


Boolean Dequeue(FloorType* floor)
{
	if( _scheduleCtrl.qIn != _scheduleCtrl.qOut) //wenn queue nicht leer
	{
		*floor = _scheduleCtrl.ElevatorNextPosQ[_scheduleCtrl.qOut];
		_scheduleCtrl.qOut = (_scheduleCtrl.qOut + 1)%countof(_scheduleCtrl.ElevatorNextPosQ);
		return true;
	}
	return false;
}