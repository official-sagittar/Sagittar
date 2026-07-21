#ifndef PCH_H
#define PCH_H

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <vector>
#ifdef EXTERNAL_TUNE
    #include <deque>
    #include <filesystem>
    #include <fstream>
    #include <thread_pool/thread_pool.h>
    #include <thread_pool/version.h>
#endif
#ifdef TEST
    #include <filesystem>
    #include <fstream>
    #include <map>
    #include <regex>
#endif

#endif
