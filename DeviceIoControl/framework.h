//////////////////// framework.h ////////////////////

#pragma once

#define WIN32_LEAN_AND_MEAN

#define NOMINMAX
#if defined(_WIN64)
#define _AMD64_
#else
#define _X86_
#endif

#include <cstdint>
#include <iostream>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include <DirectXMath.h>
#include <ShellScalingApi.h>

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <cctype>
#include <future>
#include <atomic>
#include <optional>
#include <functional>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Shcore.lib")