#ifndef MAIN_H
#define MAIN_H

#define _DEBUG_DOSSIER

#ifdef _DEBUG_DOSSIER
  #define ON_DEBUG( x ) x
  #define DEBUG_LOG( ... ) print(debug, __FILE__, ':', __LINE__,':', __VA_ARGS__)
#else
  #define ON_DEBUG( ... )
  #define DEBUG_LOG( x ) do {} while(0)
#endif

#include "file/log.h"
#include "server/config.h"
#include "err.h"

#endif
