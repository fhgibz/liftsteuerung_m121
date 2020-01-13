/**
* @file MainController.c
* @brief Zustandsmaschine für Haupt Steuerung
*
* @date  24.11.2019 - Erstellen des templates
* @author
*/

#include "LiftLibrary.h"
#include "AppIncludes.h"

// nützliche Macros
#define DOOR_CLOSE_TIMEOUT				20000
#define IS_BUTTON_PRESS(msg)			((msg->Id == ButtonEvent) && (msg->MsgParamHigh == Pressed))
#define IS_RESERVATION(buttonMask)		((buttonMask)&0xF0)
#define IS_TARGET_SELECTION(buttonMask)	((buttonMask)&0x0F)

#define countof(array)					(sizeof(array)/sizeof(array[0]))
#define NOT_PENDING(floor)				!(_mainCtrl.pendingRequests&(1<<floor))

// private Funktionen
FloorType GetFloorReservation(uint8_t msgParam);
FloorType GetTargetSelection(uint8_t msgParam);

void MainCtrl_Initializing(Message* msg);
void MainCtrl_AwaitDoorClosed(Message* msg);
void MainCtrl_AwaitElevatorRequest(Message* msg);
void MainCtrl_AwaitTargetSelection(Message* msg);
void MainCtrl_ElevatorMoving(Message* msg);
void MainCtrl_ElevatorHasArrived(Message* msg);
void MainCtrl_DoorIsOpen(Message* msg);
void MainCtrl_CheckingScheduler(Message* msg);

Boolean Enqueue(FloorType floor);
Boolean Dequeue(FloorType* floor);




void SysState_Initializing(Message* msg);
void SysCtrl_Stopped(Message* msg);

typedef struct {
	Fsm fsm;
	FloorType floor;
	uint8_t timer;
} MainController;

MainCtrl _mainCtrl = {
	.currentFloor = Floor0,
	.nextFloor = Floor0,
	.timer = 0,
	.pendingRequests = 0,
	.fsm = {.Next = 0, .RxMask = 0xFF, .CurrentState = SysState_Initializing},
	
};

void SysState_Initializing(Message* msg){
	if(msg->Id == LiftStarted){
		ClrIndicatorElevatorState(Floor0);
		ClrIndicatorElevatorState(Floor1);
		ClrIndicatorElevatorState(Floor2);
		ClrIndicatorElevatorState(Floor3);
		
		ClrIndicatorFloorState(Floor0);
		ClrIndicatorFloorState(Floor1);
		ClrIndicatorFloorState(Floor2);
		ClrIndicatorFloorState(Floor3);
		
		SetState(&_mainCtrl.fsm, MainCtrl_Initializing);
	}
}

void MainCtrl_Initializing(Message* msg)
{
	Usart_PutChar(0x80);
	Usart_PutChar(msg->Id);
	if( msg->Id == LiftCalibrated)
	{
		_mainCtrl.currentFloor = Floor0;
		SetState(&_mainCtrl.fsm, MainCtrl_CheckingScheduler)
		return;
	}
}

void MainCtrl_CheckingScheduler(Message* msg){
	if( msg->Id == Message_ElevatorReady)
	{
		EnableStatusUpdate = true;
		
		//ToDo Scheduler abfragen ob etwas darin vorhanden ist (false) ersetzen
		if(_mainCtrl.nextFloor != NULL){
			
			//ToDo das nächste Stockwerk aus dem Scheduler im _mainCtrl.nextFloor speichern, wird dann abgearbeitet
			// Comment von Flo: Ich glaube ist schon bei MainCtrl_AwaitElevatorRequest passiert, muss hier nicht nochmals gemacht werden
			
			// floorCalled zurücksetzen
			if(_mainCtrl.nextFloor = 0) {
				_mainCtrl.floor1Called = false;
			}
			else if(_mainCtrl.nextFloor = 1) {
				_mainCtrl.floor2Called = false;
			}
			else if(_mainCtrl.nextFloor = 2) {
				_mainCtrl.floor3Called = false;
			}
			else if(_mainCtrl.nextFloor = 3) {
				_mainCtrl.floor4Called = false;
			}
			
			else{
				
			}
			
			//Check ob Türe geschlossen ist vor Abfahrt
			SendEvent(SignalSourceApp, Message_MoveTo, _mainCtrl.nextFloor, _mainCtrl.currentFloor);
			SetState(&_mainCtrl.fsm, MainCtrl_ElevatorHasArrived);
			return;
		}
		
		SetState(&_mainCtrl.fsm, MainCtrl_AwaitElevatorRequest);
		return;
	}
}

void MainCtrl_AwaitElevatorRequest(Message* msg)
{
	Usart_PutChar(0xA0);
	Usart_PutChar(msg->Id);

	if( IS_BUTTON_PRESS( msg ) )
	{
		if( IS_RESERVATION(msg->MsgParamLow))
		{
			FloorType reservation = GetFloorReservation(msg->MsgParamLow);
			if( reservation != _mainCtrl.currentFloor )
			{
				//ToDo: Reservations (Wenn jemand den Lift bestellt) in den Scheduler einfügen
				
				if (_mainCtrl.elevatorGoingUp == NULL){
					_mainCtrl.elevatorGoingUp = true;
				}
				
				if(reservation = 0) {
					_mainCtrl.floor1Called = true;
				}
				else if(reservation = 1) {
					_mainCtrl.floor2Called = true;
				}
				else if(reservation = 2) {
					_mainCtrl.floor3Called = true;
				}
				else if(reservation = 3) {
					_mainCtrl.floor4Called = true;
				}
				
				//Scheduler Logik
				if(_mainCtrl.elevatorGoingUp == true){
					int theFloor = _mainCtrl.currentFloor;
					while(theFloor + 1 < 5){
						switch(theFloor) {
							case 1:
								if(_mainCtrl.floor2Called == true){
									_mainCtrl.nextFloor = 1;
									theFloor = 5;
									}; break;
							case 2:
								if(_mainCtrl.floor3Called == true){
									_mainCtrl.nextFloor = 2;
									theFloor = 5;
								}; break;
							case 3:
								if(_mainCtrl.floor4Called == true){
									_mainCtrl.nextFloor = 3;
									theFloor = 5;
								}; break;
							case 4:
								_mainCtrl.elevatorGoingUp = false;
								if(_mainCtrl.floor3Called == true){
									_mainCtrl.nextFloor = 2;
									theFloor = 5;
								}; break;
							default: _mainCtrl.nextFloor = 0; break;
						}
						theFloor = theFloor + 1;
					}
				}
				if (_mainCtrl.elevatorGoingUp == false){
					int theFloor = _mainCtrl.currentFloor;
					while(theFloor - 1 > -2){
						switch(theFloor) {
							case 2:
							if(_mainCtrl.floor3Called == true){
								_mainCtrl.nextFloor = 2;
								theFloor = -2;
							}; break;
							case 1:
							if(_mainCtrl.floor2Called == true){
								_mainCtrl.nextFloor = 1;
								theFloor = -2;
							}; break;
							case 0:
							if(_mainCtrl.floor1Called == true){
								_mainCtrl.nextFloor = 0;
								theFloor = -2;
							}; break;
							case -1:
							_mainCtrl.elevatorGoingUp = true;
							if(_mainCtrl.floor2Called == true){
								_mainCtrl.nextFloor = 1;
								theFloor = -2;
							}; break;
							default: _mainCtrl.nextFloor = 0; break;
						}
						theFloor = theFloor - 1;
					}
				}
				
				//ToDo: Nächstes reserviertes Stockwerk aus dem Scheduler rausholen und in _main.Ctrl.NextFloor übergeben
				// wurde oben in Scheduler Logik schon gemacht
			}
		}
	}
	
	if(msg->Id == DoorEmergencyBreak){
		SetState(&_mainCtrl, MainCtrl_Initializing);
		SendEvent(SignalSourceApp, LiftStarted, 0, 0);
		
	}
}

void MainCtrl_ElevatorHasArrived(Message* msg){
	//Nachdem der Lift am Ziel Ort angekommen ist wird der Timer für die offene Türe gesetzt
	if(msg->Id == SetDoorOpenTimer){
		_mainCtrl.currentFloor = (FloorType)msg->MsgParamLow;
		SendEvent(SignalSourceApp, OpenDoor, _mainCtrl.currentFloor, 0);
		_mainCtrl.timer = StartTimer(5000);
		SetState(&_mainCtrl.fsm, MainCtrl_DoorIsOpen);
	}
}

void MainCtrl_DoorIsOpen(Message* msg){
	//Wenn Türe offen ist und jemand den gleichen Knopf drückt wie den currentFloor, öffnet sich die Tür wieder
	if( IS_BUTTON_PRESS (msg)){
		if(IS_RESERVATION(msg->MsgParamLow)){
			FloorType reservation = GetFloorReservation(msg->MsgParamLow);
			if( reservation == _mainCtrl.currentFloor ){
				SendEvent(SignalSourceApp, OpenDoor, reservation, 0);
				_mainCtrl.timer = StartTimer(5000);
			}
		}
	}
	
	//Wenn der Timer für die offene Tür ablauft
	if( msg->Id == TimerEvent )
	{
		_mainCtrl.timer = StopTimer();
		Usart_PutChar(0xA1);
		Usart_PutChar(_mainCtrl.currentFloor);
		SendEvent(SignalSourceApp, CloseDoor, _mainCtrl.currentFloor, 0);
		SetState(&_mainCtrl.fsm, MainCtrl_CheckingScheduler);
	}
}

FloorType GetFloorReservation(uint8_t buttonEventParameter )
{
	return FindBit(buttonEventParameter) - 4;
}

FloorType GetTargetSelection(uint8_t buttonEventParameter )
{
	return FindBit(buttonEventParameter);
}


Boolean Enqueue(FloorType floor)
{
	uint8_t nextIn = (_mainCtrl.qIn + 1)%countof(_mainCtrl.ElevatorNextPosQ);
	if( nextIn != _mainCtrl.qOut)
	{
		_mainCtrl.ElevatorNextPosQ[_mainCtrl.qIn] = floor;
		_mainCtrl.qIn = nextIn;
		return true;
	}
	return false;
}


Boolean Dequeue(FloorType* floor)
{
	if( _mainCtrl.qIn != _mainCtrl.qOut)
	{
		*floor = _mainCtrl.ElevatorNextPosQ[_mainCtrl.qOut];
		_mainCtrl.qOut = (_mainCtrl.qOut + 1)%countof(_mainCtrl.ElevatorNextPosQ);
		return true;
	}
	return false;
}

uint8_t FindBit(uint8_t value)
{
	if( value == 0) return 0xFF;
	uint8_t pos = 0;
	while(!(value&1))
	{
		value >>=1;
		pos++;
	}
	return pos;
}