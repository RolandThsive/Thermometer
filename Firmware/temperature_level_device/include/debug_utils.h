#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#define DEBUG 1 // Change to 0 to disable

#if DEBUG
  #define DEBUG_BEGIN(baud) Serial.begin(baud)
  // __VA_ARGS__ captures all arguments passed to the macro
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG_BEGIN(baud) 
  #define DEBUG_PRINT(...)    
  #define DEBUG_PRINTLN(...)  
#endif

#endif