#ifndef PCH_H
#define PCH_H

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#ifdef DEBUG
    #include <cassert>
#endif
#ifdef EXTERNAL_TUNE
    #include <deque>
#endif
#ifdef TEST
    #include <filesystem>
    #include <fstream>
    #include <map>
    #include <regex>
#endif

#endif
