#include "ESP32HimemDeclarations.h"

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

int ESP32Himem::begin(size_t ranges, bool option) {

  iRanges = constrain(ranges, 1, 8);
  iFullBufferSize = ESP_HIMEM_BLKSZ * iRanges;
  iBufferSize = iFullBufferSize;
  iOption = option;
  
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

#ifndef ESP32HIMEM_NO_RANGE_CHECK  
  if ( pos >= iSize ) {
    // Serial.printf("remap: po OOB = %l\n", pos);
    return ESP32HIMEM_OUT_OF_BOUND;
  }
#endif

  int rc;
  
  if ( iOption ) {
    // this is a "remap less" option - block is determined by the full memory range
    size_t  block = pos / iFullBufferSize;
    
    if ( (block != iPosition / iFullBufferSize) || force ) {
      
      if ( iBase ) {
        rc = lesp_himem_unmap(iRange, iBase, iFullBufferSize);
        if ( rc != ESP_OK ) return ESP32HIMEM_ERR_UNMAP;
        iBase = NULL;
        // Serial.printf("remap: lesp_himem_unmap OK\n");
      }

      rc = lesp_himem_map(iHandle, iRange, block * iFullBufferSize, 0, iFullBufferSize, 0, (void**)&iBase);
      if ( rc != ESP_OK  ) return ESP32HIMEM_ERR_MAP;
      // Serial.printf("remap: lesp_himem_map OK. iBase = %x\n", (uint32_t) iBase);
    }
    
    iPtr = iBase + (pos % iFullBufferSize);
  }
  else {
    // this is a "remap more" option - block is determined by a single range
    size_t  block = pos / ESP_HIMEM_BLKSZ;
    
    if ( (block != iPosition / ESP_HIMEM_BLKSZ) || force ) {
      
      if ( iBase ) {
        rc = lesp_himem_unmap(iRange, iBase, iBufferSize);
        if ( rc != ESP_OK ) return ESP32HIMEM_ERR_UNMAP;
        iBase = NULL;
        // Serial.printf("remap: lesp_himem_unmap OK\n");
      }

      iBufferSize = iFullBufferSize;
      
      // Serial.printf("remap: iBufferSize %d\n", iBufferSize);
      
      if ( block * ESP_HIMEM_BLKSZ + iBufferSize >= iSize ) iBufferSize = iSize - block * ESP_HIMEM_BLKSZ;

      // Serial.printf("remap: ESP_HIMEM_BLKSZ %d, iBufferSize %d\n", ESP_HIMEM_BLKSZ, iBufferSize);
      
      rc = lesp_himem_map(iHandle, iRange, block * ESP_HIMEM_BLKSZ, 0, iBufferSize, 0, (void**)&iBase);
      if ( rc != ESP_OK  ) return ESP32HIMEM_ERR_MAP;
      // Serial.printf("remap: lesp_himem_map OK. iBase = %x\n", (uint32_t) iBase);
    }
    
    iPtr = iBase + (pos % ESP_HIMEM_BLKSZ);
  }

  iPosition = pos;
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

