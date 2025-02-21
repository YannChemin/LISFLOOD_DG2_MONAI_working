#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../classes/SaveInterval.h"

SaveInterval read_save_interval
(
	const char* input_filename, 
	const char* interval_type
);