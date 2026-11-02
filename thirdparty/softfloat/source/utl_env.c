/*============================================================================

Accessors for the thread-local state of SoftFloat (utlib extension).
See utl_env.h.

=============================================================================*/

#include "platform.h"
#include "softfloat.h"
#include "utl_env.h"

uint_fast8_t utl_sf_getRoundingMode(void) { return softfloat_roundingMode; }
void utl_sf_setRoundingMode(uint_fast8_t mode) { softfloat_roundingMode = mode; }

uint_fast8_t utl_sf_getPrecision(void) { return extF80_roundingPrecision; }
void utl_sf_setPrecision(uint_fast8_t precision) { extF80_roundingPrecision = precision; }

uint_fast8_t utl_sf_getFlags(void) { return softfloat_exceptionFlags; }
void utl_sf_setFlags(uint_fast8_t flags) { softfloat_exceptionFlags = flags; }
