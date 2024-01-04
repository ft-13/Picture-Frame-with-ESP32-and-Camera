#ifndef _IT8951_H_
#define _IT8951_H_

uint8_t IT8951_Init(void);
uint16_t IT8951ReadReg(uint16_t usRegAddr);
void IT8951WriteReg(uint16_t usRegAddr,uint16_t usValue);
void GetIT8951SystemInfo(void* pBuf);
void IT8951WaitForDisplayReady();
void IT8951_Cancel();
void IT8951DisplayExample();
void IT8951DisplayExample2();
void IT8951_BMP_Example(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void IT8951Display1bppExample();
void IT8951Display1bppExample2();
void display_buffer(uint8_t* addr, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

#endif
