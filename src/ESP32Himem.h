/*



*/

#ifndef _ESP32HIMEM_H_
#define _ESP32HIMEM_H_

#define ESP32HIMEM_RANGES       8

#define ESP32HIMEM_OK           (0)
#define ESP32HIMEM_NOT_PRESENT  (-1)
#define ESP32HIMEM_OUT_OF_BOUND (-2)
#define ESP32HIMEM_ERR_ALLOC    (-3)
#define ESP32HIMEM_ERR_UNMAP    (-4)
#define ESP32HIMEM_ERR_MAP      (-5)

#include <Arduino.h>
#include "lesp_himem.h"
#include <esp_himem.h>


class ESP32Himem : public Stream {
  public:
    ESP32Himem();
    virtual ~ESP32Himem();
  
    virtual int       begin(size_t ranges = ESP32HIMEM_RANGES);
    
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
    size_t            bufferIndex() { return iPosition % ESP_HIMEM_BLKSZ; };
    uint8_t*          pointer() { return iPtr; };
    
  private:
    int               remap(size_t pos, bool force = false);
    
    size_t            iSize;
    size_t            iPosition;
    uint8_t*          iPtr;
    
    esp_himem_handle_t      iHandle;
    esp_himem_rangehandle_t iRange;   //Handle for the actual RAM.
    uint8_t*          iBase;
    size_t            iBufferSize;
    size_t            iRanges;
};


ESP32Himem::ESP32Himem() {
  iPosition = 0;
  iBase = NULL;
}

ESP32Himem::~ESP32Himem() {
  //  todo: release all memory
  
  if ( iBase ) {
    lesp_himem_unmap(iRange, iBase, iBufferSize);
    iBase = NULL;
  }
  
  lesp_himem_free_map_range (iRange);
  
  // finally free the memory
  lesp_himem_free(iHandle);
}

int ESP32Himem::begin(size_t ranges) {

  iRanges = constrain(ranges, 1, 8);
  lesp_himem_init();

  iSize = esp_himem_get_phys_size();
  if ( iSize == 0 ) return ESP32HIMEM_NOT_PRESENT;
  // Serial.printf("begin: iSize = %d\n", iSize);
  
  int rc = lesp_himem_alloc(iSize, &iHandle);
  if ( rc != ESP_OK ) return ESP32HIMEM_ERR_ALLOC;
  // Serial.printf("begin: lesp_himem_alloc OK\n");
  
  rc = lesp_himem_alloc_map_range(ESP_HIMEM_BLKSZ * iRanges, &iRange);
  if ( rc != ESP_OK ) return ESP32HIMEM_ERR_ALLOC;
  // Serial.printf("begin: lesp_himem_alloc_map_range OK\n");
  
  return remap(iPosition, true);
}


int ESP32Himem::remap(size_t pos, bool force) {
  
  if ( pos >= iSize ) {
    // Serial.printf("remap: po OOB = %l\n", pos);
    return ESP32HIMEM_OUT_OF_BOUND;
  }
  
  int rc;
  
  size_t  block = pos / ESP_HIMEM_BLKSZ;
  // size_t  cblck = iPosition / ESP_HIMEM_BLKSZ;
  
  // Serial.printf("remap: curr block %d, req block %d\n", cblkc, iBlock);
  
  if ( force || block != iPosition / ESP_HIMEM_BLKSZ ) {
    
    if ( iBase ) {
      rc = lesp_himem_unmap(iRange, iBase, iBufferSize);
      if ( rc != ESP_OK ) return ESP32HIMEM_ERR_UNMAP;
      iBase = NULL;
      // Serial.printf("remap: lesp_himem_unmap OK\n");
    }

    iBufferSize = ESP_HIMEM_BLKSZ * iRanges;
    
    // Serial.printf("remap: iBufferSize %d\n", iBufferSize);
    
    if ( block * ESP_HIMEM_BLKSZ + iBufferSize >= iSize ) iBufferSize = iSize - block * ESP_HIMEM_BLKSZ;

    // Serial.printf("remap: ESP_HIMEM_BLKSZ %d, iBufferSize %d\n", ESP_HIMEM_BLKSZ, iBufferSize);
    
    rc = lesp_himem_map(iHandle, iRange, block * ESP_HIMEM_BLKSZ, 0, iBufferSize, 0, (void**)&iBase);
    if ( rc != ESP_OK  ) return ESP32HIMEM_ERR_MAP;
    // Serial.printf("remap: lesp_himem_map OK. iBase = %x\n", (uint32_t) iBase);
  }
  
  iPtr = iBase + (pos % ESP_HIMEM_BLKSZ);
  iPosition = pos;
  // iBlock = block;
  // Serial.printf("remap: iPtr = %x iPosition = %d\n", (uint32_t) iPtr, iPosition);
  
  return ESP32HIMEM_OK;
}

int ESP32Himem::read() {
  if ( available() ) {
    int c = *iPtr;
    remap(iPosition+1);
    return c;
  }
  return (-1);
}

int ESP32Himem::peek() {
  if ( available() ) return *iPtr;
  return (-1);
}

size_t ESP32Himem::write(uint8_t data) {
  if ( available() ) {
    *iPtr = data;
    remap(iPosition+1);
    return 1;
  }
  return 0;
}

int ESP32Himem::seek(size_t position) {
    return remap(position);
}

uint8_t&  ESP32Himem::operator[](size_t pos) {
  remap(pos);
  return (uint8_t&) *iPtr;
}

#endif
