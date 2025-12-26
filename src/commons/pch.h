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
#include <span>
#include <sstream>
#include <string>
#include <vector>
#ifdef EXTERNAL_TUNE
    #include <deque>
    #include <filesystem>
    #include <fstream>
#endif
#ifdef TEST
    #include <filesystem>
    #include <fstream>
    #include <map>
    #include <regex>
#endif

#endif
