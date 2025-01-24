/*
 * uvfr_vehicle_commands.h
 *
 *  Created on: Nov 27, 2024
 *      Author: byo10
 */

/** @defgroup uvfr_veh_commands UVFR Vehicle Commands
 *
 * @brief A fun lil API which is used to get the vehicle to do stuff
 *
 * This is designed to be portable between different versions of the VCU and PMU
 *
 */

#ifndef INC_UVFR_VEHICLE_COMMANDS_H_
#define INC_UVFR_VEHICLE_COMMANDS_H_

#include "uvfr_global_config.h"
#include "uvfr_utils.h"

#if (UV19_PDU + ECUMASTER_PMU) > 1
#error "Invalid PDU configuration"
#endif

#ifndef uvOpenSDC
#if UV19_PDU
	#define uvOpenSDC(x) _uvOpenSDC_canBased(x)
#elif ECUMASTER_PMU
	#define uvOpenSDC(x) _uvOpenSDC_canBased(x)
#else
	#define uvOpenSDC(x) _uvOpenSDC_internal(x)
#endif
#endif

#ifndef uvCloseSDC
#if UV19_PDU
	#define uvOpenSDC(x) _uvCloseSDC_canBased(x)
#elif ECUMASTER_PMU
	#define uvOpenSDC(x) _uvCloseSDC_canBased(x)
#else
	#define uvOpenSDC(x) _uvCloseSDC_internal(x)
#endif
#endif //Close SDC

#ifndef uvStartFans
#if UV19_PDU
#define uvStartFans(x) _uvStartFans_canBased(x)
#elif ECUMASTER_PMU
#define uvStartFans(x) _uvStartFans_canBased(x)
#else
#define uvStartFans(x) _uvStartFans_internal(x)
#endif
#endif //start fans

#ifndef uvStopFans
#if UV19_PDU
#define uvStopFans(x) _uvStopFans_canBased(x)
#elif ECUMASTER_PMU
#define uvStopFans(x) _uvStopFans_canBased(x)
#else
#define uvStopFans(x) _uvStopFans_internal(x)
#endif
#endif //start fans

#ifndef uvStartCoolantPump
#if UV19_PDU
#define uvStartCoolantPump(x) _uvStartCoolantPump_canBased(x)
#elif ECUMASTER_PMU
#define uvStartCoolantPump(x) _uvStartCoolantPump_canBased(x)
#else
#define uvStartCoolantPump(x) _uvStartCoolantPump_internal(x)
#endif
#endif

#ifndef uvStopCoolantPump
#if UV19_PDU
#define uvStopCoolantPump(x) _uvStopCoolantPump_canBased(x)
#elif ECUMASTER_PMU
#define uvStopCoolantPump(x) _uvStopCoolantPump_canBased(x)
#else
#define uvStopCoolantPump(x) _uvStopCoolantPump_internal(x)
#endif
#endif

#ifndef uvHonkHorn
#if UV19_PDU
#define uvHonkHorn(x) _uvHonkHorn_canBased(x)
#elif ECUMASTER_PMU
#define uvHonkHorn(x) _uvHonkHorn_canBased(x)
#else
#define uvHonkHorn(x) _uvHonkHorn_internal(x)
#endif
#endif

void uvSecureVehicle();


#endif /* INC_UVFR_VEHICLE_COMMANDS_H_ */
