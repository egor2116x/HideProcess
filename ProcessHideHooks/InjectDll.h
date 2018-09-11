#pragma once

#ifdef PROCESSHIDELIBRARY_EXPORTS  
#define PROCESSHIDELIBRARY_EXPORTS __declspec(dllexport)   
#else  
#define PROCESSHIDELIBRARY_EXPORTS __declspec(dllimport)   
#endif 

bool PROCESSHIDELIBRARY_EXPORTS Start();
