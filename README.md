# ESP32 Himem
### Access to HIMEM area on ESP32 microcontrollers from Arduino Core as Stream
#### Version 0.0.3: 2021-07-18 

### OVERVIEW:
Some ESP32 modules come equipped with PSRAM, and some even have 8 MB of it.

Unfortunately only 4 MB of PSRAM is available via standard PSRAM API.

The "high" 4 MB area requires use of the Espressif's **HIMEM** **API** which (as of Arduino Core 1.0.6) does not work with Arduino Core.

This library breaks that spell and makes the 4 MB of himem area available as a class derived from **Stream** so you can easily use those megabytes as file store or as a memory array. 

API definition is fairly straightforward and all memory remapping is happening behind the scenes. 

Because of the memory remapping the access is rather slow (especially random), so consider this to be a 4 MB fast HDD for ESP32 :)



### API

    ESP32Himem();
    virtual ~ESP32Himem();
      
    virtual int       begin(size_t ranges, bool option); // ranges: 1-8, default 8, option (see below)
    
    virtual int       available();
    virtual int       read();
    virtual size_t    write(uint8_t data);
    virtual int       peek();
    virtual void      flush();
    virtual size_t    size();
    
    virtual size_t    physSize()};
    virtual int       seek(size_t position);
    virtual uint8_t&  operator[](size_t pos);
    size_t            bufferSize();
    size_t            bufferIndex();
    uint8_t*          pointer();


### Note on direct memory access:

After your index been positioned and mapped, it is placed as close to the beginning of mapped memory area as possible (given you have to observe the 32KB alignments) and mapped to an area from 32,768 of up to 262,144 bytes long.

Technically you can address that memory region directly and read/write to that area directly as you would to regular RAM.

Your data is located at `pointer()` position in memory.

You have `bufferSize() - bufferIndex()` bytes of memory available to the **right** of the pointer and `bufferIndex()` bytes available to the **left** of the pointer. **Be careful.** 


### Note on PSRAM

The address space used by PSRAM API and HIMEM API is the same, so allocating a window of memory to look into HIMEM area will take away from the available PSRAM. Arduino is configured to use up to 8 x 32KB memory blocks to access HIMEM. In most cases you only need one, so start `himem.begin(1)` to allocate just 32K and lose *only* 32K of PSRAM. 8 ranges will allocate 262144 bytes and that's how much PSRAM you will lose as well. 

**ALSO:** please initialize HIMEM before doing any PSRAM memory allocations via `ps_malloc`. I have witnessed values changing as the HIMEM window was allocated on top of previously allocated PSRAM memory range. **Be careful.** 

### Note on Range Allocation Option parameter of begin() method

By default, ESP32Himem allocates the maximum available number of memory ranges (8) to access parts of himem area. That gives you a "window" of 262,144 bytes. Since an your index (the current address of the memory being accessed) could fall into any of the 8 standard 32K ranges, there are two separate mapping strategies possible:

1. Map as close to the left (lesser) address to allow as much pointer access to the right as the range allows to.

   This option requires more frequent remapping, and therefore takes a little toll on performance as the full range is remapped for every one of the 8 standard 32K ranges. This is a default option, and could be achieved by using `himem.begin(8);` (or any other number between 1-8) or `himem.begin(8, ESP32HIMEM_REMAP_MORE);`

   Using this option guarantees that you have at least `(N-1)*32K` bytes of memory to the right of the pointer, where N is the total number or standard ranges allocated (1-8). 

2. Map anywhere within the full range. The "catch" here is that the pointer could be anywhere within the range, including at the very far right end of it, which makes use of pointer less efficient, but requires fewer remaps, so improves performance slightly. 

   To use this remap strategy start ESP32Himem with `himem(8, ESP32HIMEM_REMAP_LESS);` option.
   

### Note on Out-of-Bounds checking

By default every range remap operation is checking if the index is out of bounds of available memory. This takes a few microseconds. If you are absolutely sure in your ability to manage the range properly, you can compile the library with
`#define ESP32HIMEM_NO_RANGE_CHECK` compile option and gain a little performance improvement at a price of potential error. Up to you. **Be careful with this one.** 

### Benchmarking (default setup)

```
Himem physical size: 4456448
Himem allocated size: 4456448
Himem max buffer size: 262144
Himem pointer: 3FBC0000

Populating Himem area... 
Start time: 835
Stop  time: 4873

Reading Himem area via Read 
Start time: 4873
Stop  time: 8507

Reading Himem area via [] 
Start time: 8508
Stop  time: 11473

Reading Himem area randomly 1000 times 
Start time: 11473
Stop  time: 21794

Writing and reading Himem area buffer directly 
Start time: 21794
Stop  time: 21827
```

See example for details. 