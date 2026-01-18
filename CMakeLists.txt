cmake_minimum_required(VERSION 3.16.0)

# Exclude esp_lcd component to avoid compiler crash
set(EXCLUDE_COMPONENTS esp_lcd)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(WifiRepeater)
