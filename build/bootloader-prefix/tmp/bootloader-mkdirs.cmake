# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/esp_v5/idf/components/bootloader/subproject"
  "D:/project/ESP32/WIFI_config/build/bootloader"
  "D:/project/ESP32/WIFI_config/build/bootloader-prefix"
  "D:/project/ESP32/WIFI_config/build/bootloader-prefix/tmp"
  "D:/project/ESP32/WIFI_config/build/bootloader-prefix/src/bootloader-stamp"
  "D:/project/ESP32/WIFI_config/build/bootloader-prefix/src"
  "D:/project/ESP32/WIFI_config/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/project/ESP32/WIFI_config/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/project/ESP32/WIFI_config/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
