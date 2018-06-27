#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "stdio.h"
#include "snow_tiger.h"
int main()
{
	printf("Initing...\n");
	ILI9341_Init();
	printf("Done, waiting\n");
	//HAL_Delay(1000);
	printf("Filling...\n");
	//HAL_Delay(1000);
	ILI9341_Fill_Screen(WHITE);
	//HAL_Delay(1000);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_2);
	
	ILI9341_Draw_Text("XYU",10,10,BLACK,1,RED);
	printf("Done\n");
	HAL_Delay(1000);
	ILI9341_Draw_Image((const char*)snow_tiger,SCREEN_VERTICAL_2);
}
