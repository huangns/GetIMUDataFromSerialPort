#ifndef __UART_NET_H
#define __UART_NET_H

//signed char	SendUARTMessageLength(const unsigned long ulChannelNo, const char chrMessage[],const unsigned short usLen);
signed char SendUARTMessageLength(unsigned char ulChannelNo, unsigned char chrSendBuffer[],const unsigned short usLen);
unsigned short CollectUARTData(const unsigned long ulChannelNo, char chrUARTBufferOutput[]);
signed char OpenCOMDevice(const unsigned long ulPortNo);
signed char SetBaundrate(const unsigned long ulPortNo,const unsigned long ulBaundrate);
signed char OpenCOMDevice(const unsigned long ulPortNo,const unsigned long ulBaundrate);
void CloseCOMDevice(void);
void read(unsigned short regId,unsigned short regNum);
//void CloseCom();

#endif
