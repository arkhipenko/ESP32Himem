/*
  ESP32 Himem Library. (C) Anatoli Arkhipenko 2021
  
  Access to HIMEM area on ESP32 microcontrollers from Arduino Core as Stream.

  CHANGE LOG:
  
  2021-07-15:
    v0.0.1 - Initial Release
    v0.0.2 - Added ability to change number of ranges
  
  2021-07-
    v0.0.3 - Added ability to chose range allocation strategy
           - Added ability to skip out-of-bounds checking

  2022-04-09
    v0.0.4 - Carve out ESP32HimemDeclarations.h for PlatfomIO compatibility


*/

#ifndef _ESP32HIMEM_H_
#define _ESP32HIMEM_H_

// #define ESP32HIMEM_NO_RANGE_CHECK

#define ESP32HIMEM_RANGES       8

#define ESP32HIMEM_OK           (0)
#define ESP32HIMEM_NOT_PRESENT  (-1)
#define ESP32HIMEM_OUT_OF_BOUND (-2)
#define ESP32HIMEM_ERR_ALLOC    (-3)
#define ESP32HIMEM_ERR_UNMAP    (-4)
#define ESP32HIMEM_ERR_MAP      (-5)

#define ESP32HIMEM_REMAP_LESS   true
#define ESP32HIMEM_REMAP_MORE   false

#include <Arduino.h>
#include "lesp_himem.h"
#include <esp_himem.h>


class ESP32Himem : public Stream {
  public:
    ESP32Himem();
    virtual ~ESP32Himem();
  
    virtual int       begin(size_t ranges = ESP32HIMEM_RANGES, bool option = ESP32HIMEM_REMAP_LESS);
    
    virtual int       available() { return ( iPosition > iSize-1 ? 0 : 1 ); };
    virtual int       read();
    virtual size_t    write(uint8_t data);
    virtual int       peek();
    virtual void      flush() {};
    virtual size_t    size() { return iSize; };
    virtual size_t    physSize() { return esp_himem_get_phys_size(); };
    virtual int       seek(size_t position);
    virtual uint8_t&  operator[](size_t pos);
    size_t            bufferSize() { return iBufferSize; };
    size_t            bufferIndex() { return iPosition % (iOption ? iFullBufferSize : ESP_HIMEM_BLKSZ); };
    uint8_t*          pointer() { return iPtr; };
    
  private:
    int               remap(size_t pos, bool force = false);
    
    size_t            iSize;
    size_t            iPosition;
    uint8_t*          iPtr;
    bool              iOption;
    
    esp_himem_handle_t      iHandle;
    esp_himem_rangehandle_t iRange;   //Handle for the actual RAM.
    uint8_t*          iBase;
    size_t            iBufferSize;
    size_t            iFullBufferSize;
    size_t            iRanges;
};
#endif
