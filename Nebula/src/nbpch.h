#pragma once

#include <iostream>
#include <functional>

#include <memory>
#include <utility>
#include <algorithm>

#include <chrono>

#include <string>
#include <sstream>
#include <array>
#include <vector>

#include <unordered_map>
#include <unordered_set>

#include <filesystem>

#include "Nebula/Core/Log.h"
#include "Nebula/Debug/Instrumentor.h"
#include "Nebula/Maths/Maths.h"
#include "Nebula/Utils/Arrays.h"

#ifdef NB_WINDOWS
	#include <Windows.h>
#endif //NB_WINDOWS