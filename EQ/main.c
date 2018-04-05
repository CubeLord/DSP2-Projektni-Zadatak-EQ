//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 2
// * Godina: 2017
// *
// * Zadatak: Ekvalizacija audio signala
// * Autor:
// *                                                                          
// *                                                                          
/////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "print_number.h"
#include "math.h"
#include "string.h"

#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 16000L

#define PI 3.14159265

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];
Int16 dirakSample[AUDIO_IO_SIZE];

//za frek odabiranja 8000
//float alphaLP = 0.8712037;
//float alphaHP = - 0.8540809; //da li on prepoznaje minus?
//float alphaPeek = 0.6969612;
//float betaPeek = 0.860742;
//float alphaPeek2 = 0.5219542;
//float betaPeek2 = - 0.4222229; //da li on prepoznaje minus?

//za frek odabiranja 16000
float alphaLP = 0.9335353;
float alphaHP = 0.0392900;
float alphaPeek = 0.8372424;
float betaPeek = 0.9645574 ;
float alphaPeek2 = 0.7340693;
float betaPeek2 = 0.5374835;

//0.1 -> jako siroko
//0.9 jako usko

//za LP i HP 0.9 je jako ravan, a 0.1 jako strm

//float alphaLP = 0.8886;
//float alphaHP = 0.0392;
//float alphaPeek = 0.81;
//float betaPeek = 0.934;
//float alphaPeek2 = 0.7915;
//float betaPeek2 = 0.447;


Int16 EQLP[AUDIO_IO_SIZE];
Int16 EQHP[AUDIO_IO_SIZE];
Int16 EQPeek[AUDIO_IO_SIZE];
Int16 EQPeek2[AUDIO_IO_SIZE];

Int16 EQx_historyLP[2];
Int16 EQy_historyLP[2];
Int16 EQx_historyHP[2];
Int16 EQy_historyHP[2];
Int16 EQx_historyPeek[2];
Int16 EQy_historyPeek[2];
Int16 EQx_historyPeek2[2];
Int16 EQy_historyPeek2[2];



Int16 x_historyLP[2];
Int16 y_historyLP[2];
Int16 x_historyHP[2];
Int16 y_historyHP[2];
Int16 x_historyPeek[2];
Int16 y_historyPeek[2];
Int16 coeffLP[4];
Int16 coeffHP[4];
Int16 coeffPeek[6];
Int16 coeffPeek2[6];


Int16 ShellBufferLP[AUDIO_IO_SIZE];
Int16 ShellBufferHP[AUDIO_IO_SIZE];
Int16 ShellBufferPeek[AUDIO_IO_SIZE];

void main( void )
{   
	int i;
	int keyOld = -1;
	char c;

	int k[4] = {32767, 32767, 32767, 32767};
	int selected = 0;

    /* Inicijalizaija razvojne ploce */
    EZDSP5535_init( );

    /* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
    EZDSP5535_SAR_init();

    /* Inicijalizacija LCD kontrolera */
    initPrintNumber();

	printf("\n Ekvalizacija audio signala \n");
		
    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();
	
    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

    aic3204_dma_init();
    
    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

    //Postavljanje History buffera na 0
    for(i = 0; i < 2; i++)
    {
    	x_historyLP[i] = 0;
    	y_historyLP[i] = 0;

    	x_historyHP[i] = 0;
    	y_historyHP[i] = 0;

    	x_historyPeek[i] = 0;
    	y_historyPeek[i] = 0;


    	EQx_historyLP[i] = 0;
    	EQy_historyLP[i] = 0;
    	EQx_historyHP[i] = 0;
    	EQy_historyHP[i] = 0;
    	EQx_historyPeek[i] = 0;
    	EQy_historyPeek[i] = 0;
    	EQx_historyPeek2[i] = 0;
    	EQy_historyPeek2[i] = 0;

    }

    for(i = 0; i < AUDIO_IO_SIZE; i++)
        {
        	dirakSample[i] = (i == 0? 16000 : 0);
        }


    Uint16 key;
    clearLCD();
    printChar('3');
    printChar('2');
    printChar('7');
    printChar('6');
    printChar('7');
    while(1)
    {
    	aic3204_read_block(sampleBufferL, sampleBufferR);

    			key = EZDSP5535_SAR_getKey();


				switch (key) {
					case SW1:
						if (keyOld != 1)
						{
							if (++selected > 3) selected = 0;
							printf("\nSelected: %d\n", selected);
							clearLCD();

							if ((int)k[selected]/10000 != 0)
							{
								c = (int)(k[selected]/10000) + '0';
								printChar(c);
							}
							c = (int)(k[selected]%10000)/1000 + '0';
							printChar(c);
							c = (int)(k[selected]%1000)/100 + '0';
							printChar(c);
							c = (int)(k[selected]%100)/10 + '0';
							printChar(c);
							c = (int)(k[selected])%10 + '0';
							printChar(c);


							printf("\nk = [%d, %d, %d, %d]\n", k[0], k[1], k[2], k[3]);
							keyOld = 1;
						}
						break;
					case SW2:
						if (keyOld != 2)
						{
							k[selected]-=3277;
							if(k[selected] < 0) k[selected] = 32767;
							clearLCD();

							//OVDJE UBACITI ZA PISANJE NA PLOCICU
							if ((int)k[selected]/10000 != 0)
							{
								c = (int)(k[selected]/10000) + '0';
								printChar(c);
							}
							c = (int)(k[selected]%10000)/1000 + '0';
							printChar(c);
							c = (int)(k[selected]%1000)/100 + '0';
							printChar(c);
							c = (int)(k[selected]%100)/10 + '0';
							printChar(c);
							c = (int)(k[selected])%10 + '0';
							printChar(c);


							printf("\nk = [%d, %d, %d, %d]\n", k[0], k[1], k[2], k[3]);
							keyOld = 2;
						}
						break;
					default:
						keyOld = -1;
					}
				//printf("\nk = [%d, %d, %d, %d]\n", k[0], k[1], k[2], k[3]);
//				printf("\n%d\n", k[selected]);
//				c = (int)(k[selected]/10000) + '0';
//				printf("\n\tfirst char:%c", c);
//				c = (int)(k[selected]%10000)/1000 + '0';
//				printf("\n\tsecond char:%c", c);
//				c = (int)(k[selected]%1000)/100 + '0';
//				printf("\n\tthird char:%c", c);
//				c = (int)(k[selected]%100)/10 + '0';
//				printf("\n\tforth char:%c", c);
//				c = (int)(k[selected])%10 + '0';
//				printf("\n\tlast char: %c", c);



    			/* Your code here */
    	    	/* Generisati koeficijente filtara za karakteristiku alpa = 0.3.
    	    	 * Iscrtati prenosnu karakteristiku shelving filtra za K = 8192 (sto odgovara vrednosti 0.25 skaliranoj na opseg int16) i K = 24576
    	    	 * Za potrebe iscrtavanja prenosne karakteristike filtra izracunati impulsni odziv dovodjenjem dirakovog impulsa u trajanju od N odbiraka na ulaz filtra.*/
    	    	calculateShelvingCoeff(0.3, coeffLP);
    	    	for (i = 0; i < AUDIO_IO_SIZE; i++)
    	    	{
    	    		ShellBufferLP[i] = shelvingLP(dirakSample[i], coeffLP ,x_historyLP, y_historyLP, 8192);
    	    	}






    	    	/* Generisati koeficijente filtra za karakteristiku alpha = -0.3.
    	    	 * Iscrtati prenosnu karakteristiku shelving filtra za K = 8192 (sto odgovara vrednosti 0.25) i K = 24576 (0.75).
    	    	 * Za potrebe iscrtavanja prenosne karakteristike filtra izracunati impulsi odziv dovodjenjem dirakovog impulsa u trajanju od N odabiraka na ulaz filtra*/
    	    	calculateShelvingCoeff(-0.3, coeffHP);
    	    	for (i = 0; i < AUDIO_IO_SIZE; i++)
				{
					ShellBufferHP[i] = shelvingHP(dirakSample[i], coeffHP ,x_historyHP, y_historyHP, 24576);
				}






    	    	/* Generisati koeficijente filtra za karakteristiku alpha = 0.7 i beta = 0. (greska?)
    	    	 * Iscrtati prenosnu karakteristiku shelving filtra za k = 8192 i k = 24576.
    	    	 * Za potrebe iscrtavanja prenosne karakteristike filtra izracunati impulsni odziv dovodjenjem dirakovog impulsa u trajanju od N odbiraka na ulaz filtra */
    	    	calculatePeekCoeff(0.7, 0, coeffPeek);
    	    	for (i = 0; i < AUDIO_IO_SIZE; i++)
				{
					ShellBufferPeek[i] = shelvingPeek(dirakSample[i], coeffPeek ,x_historyPeek, y_historyPeek, 8192);
				}





    	    	calculateShelvingCoeff(alphaLP, coeffLP);
    	    	for (i = 0; i < AUDIO_IO_SIZE; i ++)
    	    	{
    	    		//EQLP[i] = shelvingLP(sampleBufferL[i], coeffLP, EQx_historyLP, EQy_historyLP, k[0]);
    	    		EQLP[i] = shelvingLP(dirakSample[i], coeffLP, EQx_historyLP, EQy_historyLP, k[0]);
    	    	}

    	    	calculatePeekCoeff(alphaPeek, betaPeek, coeffPeek);
				for (i = 0; i < AUDIO_IO_SIZE; i ++)
				{
					EQPeek[i] = shelvingPeek(EQLP[i], coeffPeek, EQx_historyPeek, EQy_historyPeek, k[1]);
				}

    	    	calculatePeekCoeff(betaPeek2, betaPeek2, coeffPeek2);
				for (i = 0; i < AUDIO_IO_SIZE; i ++)
				{
					EQPeek2[i] = shelvingPeek(EQPeek[i], coeffPeek2, EQx_historyPeek2, EQy_historyPeek2, k[2]);
				}

    	    	calculateShelvingCoeff(alphaHP, coeffHP);
				for (i = 0; i < AUDIO_IO_SIZE; i ++)
				{
					EQHP[i] = shelvingHP(EQPeek2[i], coeffHP, EQx_historyHP, EQy_historyHP, k[3]);
				}





//    	    	printf("\n One Sample done \n");

    			aic3204_write_block(EQHP, EQHP);
	}

    	
	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}


