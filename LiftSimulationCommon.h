/** 
* @file Definiert gemeinsame Datentypen im Projekt
* @author:
*
*/

#ifndef __LIFT_SIMULATION_COMMON__
#define __LIFT_SIMULATION_COMMON__

typedef struct MainCtrl_tag
{
	Fsm fsm;
	FloorType currentFloor;
	FloorType nextFloor;
	uint8_t timer;
	uint8_t pendingRequests;
	FloorType ElevatorNextPosQ[4];
	uint8_t qIn;
	uint8_t qOut;
	bool floor1Called;
	bool floor2Called;
	bool floor3Called;
	bool floor4Called;
	bool elevatorGoingUp;
}MainCtrl;

extern MainCtrl _mainCtrl;

uint8_t FindBit(uint8_t value);

#endif