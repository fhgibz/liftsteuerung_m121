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
}MainCtrl;

extern MainCtrl _mainCtrl;

uint8_t FindBit(uint8_t value);

#endif