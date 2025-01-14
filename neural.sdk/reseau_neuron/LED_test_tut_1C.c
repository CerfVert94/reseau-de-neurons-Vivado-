/*
 * LED_test.c
 *
 *  Created on: 	13 June 2013
 *      Author: 	Ross Elliot
 *     Version:		1.1
 */
 
/********************************************************************************************
* VERSION HISTORY
********************************************************************************************
* v1.1 - 27 January 2014
* 	GPIO_DEVICE_ID definition updated to reflect new naming conventions in Vivado 2013.3
*		onwards.
*
*	v1.0 - 13 June 2013
*		First version created.
*******************************************************************************************/

/********************************************************************************************
 * This file contains an example of using the GPIO driver to provide communication between
 * the Zynq Processing System (PS) and the AXI GPIO block implemented in the Zynq Programmable
 * Logic (PL). The AXI GPIO is connected to the LEDs on the ZedBoard.
 *
 * The provided code demonstrates how to use the GPIO driver to write to the memory mapped AXI
 * GPIO block, which in turn controls the LEDs.
 ********************************************************************************************/

/* Include Files */
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
#include "xil_printf.h"
#include <math.h>
#include <stdio.h>
#include "xtime_l.h"
#include "data.h"
/* Definitions */
#define GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID	/* GPIO device that LEDs are connected to */
#define LED 0xC3									/* Initial LED value - XX0000XX */
#define LED_DELAY 100000000							/* Software delay length */
#define LED_CHANNEL 1								/* GPIO port for LEDs */
#define printf xil_printf							/* smaller, optimised printf */

XGpio Gpio;											/* GPIO Device driver instance */


#define IMGWIDTH 29
#define IMGHEIGHT 29
#define tour 10000

#define SIGMOID(x) (1.7159*tanh(0.66666667*x))
#define DSIGMOID(S) (0.66666667/1.7159*(1.7159+(S))*(1.7159-(S)))

void calculateLayer1(float* input, float* Layer1_Neurons_CPU);
void calculateLayer2(float* Layer1_Neurons_CPU, float* Layer1_Weights_CPU, float* Layer2_Neurons_CPU);
void calculateLayer3(float* Layer2_Neurons_CPU, float* Layer2_Weights_CPU, float* Layer3_Neurons_CPU);
void calculateLayer4(float* Layer3_Neurons_CPU, float* Layer3_Weights_CPU, float* Layer4_Neurons_CPU);
void calculateLayer5(float* Layer4_Neurons_CPU, float* Layer4_Weights_CPU, double* Layer5_Neurons_CPU);

void InitHostMem(float *Layer1_Weights_CPU,float *Layer2_Weights_CPU, float *Layer3_Weights_CPU,float *Layer4_Weights_CPU);

int LEDOutputExample(void)
{
	// Timers
	XTime tStart, tStop;
	XTime_GetTime(&tStart);

	volatile int Delay;
	int Status;
	float	Layer1_Neurons_CPU[IMGWIDTH*IMGHEIGHT],
			Layer2_Neurons_CPU[6*13*13],
			Layer3_Neurons_CPU[50*5*5],
			Layer4_Neurons_CPU[100];
	double Layer5_Neurons_CPU[10];
	double scoremax = 0.0;
	int i, j;

	xil_printf("--------------------------------------------\r\n\n");
	xil_printf("IP test begin.\r\n");
	xil_printf("--------------------------------------------\r\n\n");

	xil_printf("GPIO INIT START.\r\n");


	Status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XGpio_SetDataDirection(&Gpio, LED_CHANNEL, 0x00);


	xil_printf("GPIO INIT DONE.\r\n");
	xil_printf("--------------------------------------------\r\n\n");

	//char buffer[64];

		/* GPIO driver initialisation */
		Status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*Set the direction for the LEDs to output. */
		//int led = LED;
		int indexmax = -1;
		//InitHostMem(Layer1_Weights_CPU, Layer2_Weights_CPU,Layer3_Weights_CPU,Layer4_Weights_CPU);
		//for (j = 0; j < 100; j++)
		{
			//xil_printf("Test : %.2f %.2f %.2f %.2f\r\n", Layer1_Weights_CPU[0], Layer2_Weights_CPU[0], Layer3_Weights_CPU[0], Layer4_Weights_CPU[0]);
			calculateLayer1(Input, Layer1_Neurons_CPU);
	//		xil_printf("Calcul Layer 1 - fini\r\n");
			calculateLayer2(Layer1_Neurons_CPU, Layer1_Weights_CPU, Layer2_Neurons_CPU);
		//	xil_printf("Calcul Layer 2 - fini\r\n");
			calculateLayer3(Layer2_Neurons_CPU, Layer2_Weights_CPU, Layer3_Neurons_CPU);
			//xil_printf("Calcul Layer 3 - fini\r\n");
			calculateLayer4(Layer3_Neurons_CPU, Layer3_Weights_CPU, Layer4_Neurons_CPU);
			//xil_printf("Calcul Layer 4 - fini\r\n");
			calculateLayer5(Layer4_Neurons_CPU, Layer4_Weights_CPU, Layer5_Neurons_CPU);
			//xil_printf("Calcul Layer 5 - fini\r\n");

			scoremax = -12345;
			indexmax = -1;

			for(i=0;i<10;i++)
			{
				xil_printf("%d score max = %d\r\n",i, (int)(scoremax*10000000));
				//xil_printf("%d : %f : cur = %d\r\n",i,Layer5_Neurons_CPU[i]);
				if(Layer5_Neurons_CPU[i]>scoremax )
				{
					scoremax = Layer5_Neurons_CPU[i];
					indexmax = i;
				}
			}

		}
		XTime_GetTime(&tStop);
		/*sprintf(buffer, "Output took %llu clock cycles.", 2*(tStop - tStart));
		xil_printf("%s\r\n", buffer);
		sprintf(buffer, "Output took %f us", 1.0*(tStop - tStart)/(COUNTS_PER_SECOND/1000000));
		xil_printf("%s\r\n", buffer);*/
		if(indexmax >= 0 && indexmax <=9 && scoremax > 0)
			XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, indexmax);
		else
			XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0xFF);

		xil_printf("le resultat est %d\r\n", indexmax);

		xil_printf("Cycles d'horloge : %llu \n\r", (int)(2 * (tStop - tStart)));
		xil_printf("Temps d'execution : %llu us\n\r",(int)(1.0 * (tStop - tStart) / (COUNTS_PER_SECOND / 1000000)));
		/* Loop forever blinking the LED. */
		while (1) {

				/* Wait a small amount of time so that the LED blinking is visible. */
				//for (Delay = 0; Delay < LED_DELAY; Delay++);

		}

		return XST_SUCCESS; /* Should be unreachable */
}

/* Main function. */
int main(void){

	int Status;

	/* Execute the LED output. */
	Status = LEDOutputExample();
	if (Status != XST_SUCCESS) {
		xil_printf("GPIO output to the LEDs failed!\r\n");
	}

	return 0;
}






void calculateLayer1(float* input, float* Layer1_Neurons_CPU){
	memcpy(Layer1_Neurons_CPU, input, IMGWIDTH*IMGHEIGHT*sizeof(float));
}

void calculateLayer2(float* Layer1_Neurons_CPU, float* Layer1_Weights_CPU, float* Layer2_Neurons_CPU){
		float somme;
	int i,j,k,m,n;
	for(i=0;i<6;i++)
		for(j=0;j<13;j++)
			for(k=0;k<13;k++){
				somme = Layer1_Weights_CPU[26*i];
				for(m=0;m<5;m++)
					for(n=0;n<5;n++)
						somme += Layer1_Weights_CPU[26*i+5*m+n+1] * Layer1_Neurons_CPU[29*(m+2*j)+n+2*k];
				Layer2_Neurons_CPU[13*13*i+13*j+k] = (float) SIGMOID(somme);
			}
}

void calculateLayer3(float* Layer2_Neurons_CPU, float* Layer2_Weights_CPU, float* Layer3_Neurons_CPU){
	float somme;
	int i,j,k,m,n;
	for( i=0;i<50;i++)
		for(j=0;j<5;j++)
			for(k=0;k<5;k++){
				somme = Layer2_Weights_CPU[26*6*i];

				for( m=0;m<5;m++)
					for( n=0;n<5;n++){
						somme += Layer2_Weights_CPU[26*6*i+1+6*(n+5*m)	] * Layer2_Neurons_CPU[13*13*0+13*(2*j+m)+(2*k+n)];
						somme += Layer2_Weights_CPU[26*6*i+1+6*(n+5*m)+1] * Layer2_Neurons_CPU[13*13*1+13*(2*j+m)+(2*k+n)];
						somme += Layer2_Weights_CPU[26*6*i+1+6*(n+5*m)+2] * Layer2_Neurons_CPU[13*13*2+13*(2*j+m)+(2*k+n)];
						somme += Layer2_Weights_CPU[26*6*i+1+6*(n+5*m)+3] * Layer2_Neurons_CPU[13*13*3+13*(2*j+m)+(2*k+n)];
						somme += Layer2_Weights_CPU[26*6*i+1+6*(n+5*m)+4] * Layer2_Neurons_CPU[13*13*4+13*(2*j+m)+(2*k+n)];
						somme += Layer2_Weights_CPU[26*6*i+1+6*(n+5*m)+5] * Layer2_Neurons_CPU[13*13*5+13*(2*j+m)+(2*k+n)];

					}
				Layer3_Neurons_CPU[5*5*i+5*j+k] = (float) SIGMOID(somme);
			}
}

void calculateLayer4(float* Layer3_Neurons_CPU, float* Layer3_Weights_CPU, float* Layer4_Neurons_CPU){
	float somme;
	int i, j, k, m;
	for( i=0;i<100;i++){
		somme = Layer3_Weights_CPU[i*(1+50*25)];
		for( j=0;j<50;j++)
			for( k=0;k<5;k++)
				for ( m=0;m<5;m++)
					somme += Layer3_Weights_CPU[i*(1+50*25)+1 + m + k*5 + j*25] * Layer3_Neurons_CPU[m+5*k+25*j];

		Layer4_Neurons_CPU[i] = (float) SIGMOID(somme);
	}

}

void calculateLayer5(float* Layer4_Neurons_CPU, float* Layer4_Weights_CPU, double* Layer5_Neurons_CPU){
	float somme;
	int i, j;
	for( i=0;i<10;i++){
		somme = Layer4_Weights_CPU[101*i];
		for( j=0;j<100;j++)
			somme += Layer4_Weights_CPU[1+101*i+j] * Layer4_Neurons_CPU[j];
		Layer5_Neurons_CPU[i] = SIGMOID(somme);
	}
}

void InitHostMem(float *Layer1_Weights_CPU,float *Layer2_Weights_CPU,float *Layer3_Weights_CPU,float *Layer4_Weights_CPU)
{
	// initial layer 1 weight
	FILE * pFile1 = fopen ("lw1.wei","rb");
	if (pFile1 != NULL)
	{
	    int i;
	    for(i=0;i<156;++i)
		fread(&(Layer1_Weights_CPU[i]),sizeof(float),1,pFile1);
		fclose (pFile1);
	}

	// initial layer 2 weight
	FILE * pFile2 = fopen ("lw2.wei","rb");
	if (pFile2 != NULL)
	{
		fread(Layer2_Weights_CPU,sizeof(float),7800,pFile2);
		fclose (pFile2);
	}
	// initial layer 3 weight
	FILE * pFile3 = fopen ("lw3.wei","rb");
	if (pFile3 != NULL)
	{
		fread(Layer3_Weights_CPU,sizeof(float),125100,pFile3);
		fclose (pFile3);
	}
	// initial layer 4 weight
	FILE * pFile4 = fopen ("lw4.wei","rb");
	if (pFile4 != NULL)
	{
		fread(Layer4_Weights_CPU,sizeof(float),1010,pFile4);
		fclose (pFile4);
	}
}

void readIn(float *layer1)
{
	FILE *fp;
	fp=fopen("in.neu","rb");
	if(fp)
	{
		fread(layer1,sizeof(float),29*29,fp);
		fclose(fp);
	}
}

void output(double *final)
{
	FILE *fp=0;
	fp=fopen("out.res","wb");
	if(fp)
	{
		fwrite(final,sizeof(double),10,fp);
		fclose(fp);
	}
}
