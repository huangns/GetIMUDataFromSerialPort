
#include "JY901.h"
#include "string.h"

CJY901 ::CJY901 ()
{
}
short CJY901 ::GetShort(unsigned char *byteTemp, char sIndex)
{
	return (((short)byteTemp[sIndex]<<8)|(byteTemp[sIndex++]));
}
void CJY901 ::CopeSerialData(char ucData[],unsigned short usLength)
{
	static unsigned char chrTemp[2000];
	static unsigned char ucRxCnt = 0;	
	static unsigned short usRxLength = 0;
	char index;

	memcpy(chrTemp,ucData,usLength);
	usRxLength += usLength;
	if((usRxLength < 7)&(chrTemp[0] != 0x32)&(chrTemp[1] != 0x03))   //Ä¬ÈÏµØÖ·Îª0x50,¶ÁÃüÁîÂë0x06;
	{
		ucRxCnt=0;
		return;	              
	}
	else
	{
		stcAcc.a[0]=GetShort(chrTemp, 3);      
		stcAcc.a[1]=GetShort(chrTemp, 5);
		stcAcc.a[2]=GetShort(chrTemp, 7);

		stcGyro.w[0]=GetShort(chrTemp, 9);
		stcGyro.w[1]=GetShort(chrTemp, 11);
		stcGyro.w[2]=GetShort(chrTemp, 13);

		stcMag.h[0]=GetShort(chrTemp, 15);
		stcMag.h[1]=GetShort(chrTemp, 17);
		stcMag.h[2]=GetShort(chrTemp, 19);

		stcAngle.Angle[0]=GetShort(chrTemp, 21);
		stcAngle.Angle[1]=GetShort(chrTemp, 23);
		stcAngle.Angle[2]=GetShort(chrTemp, 25);
		stcAngle.T=GetShort(chrTemp, 27);
		//stcPress.lPressure=GetShort(chrTemp, 39)<<8|GetShort(chrTemp, 37);
		//stcPress.lAltitude=GetShort(chrTemp, 43)<<8|GetShort(chrTemp, 41);
		//stcGPSV.sGPSHeight=GetShort(chrTemp,53);
		//stcGPSV.sGPSYaw=GetShort(chrTemp,55);
		//stcGPSV.lGPSVelocity=GetShort(chrTemp, 59)<<8|GetShort(chrTemp, 57);
		usRxLength = 0;
	}
}
CJY901 JY901 = CJY901();