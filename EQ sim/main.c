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

#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L

#define PI 3.14159265

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];
Int16 dirakSample[AUDIO_IO_SIZE];

Int16 x_historyLP[2];
Int16 y_historyLP[2];
Int16 x_historyHP[2];
Int16 y_historyHP[2];
Int16 x_historyPeek[2];
Int16 y_historyPeek[2];
Int16 coeffLP[4];


Int16 ShellBufferLP[AUDIO_IO_SIZE];
Int16 ShellBufferHP[AUDIO_IO_SIZE];
Int16 ShellBufferPeek[AUDIO_IO_SIZE];

void main( void )
{
	int i;
	/* Inicijalizaija razvojne ploce */
    EZDSP5535_init( );
	
	/* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
    EZDSP5535_SAR_init();

    /* Inicijalizacija LCD kontrolera */
    initPrintNumber();

	printf("\n Ekvalizacija audio signala \n");

    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();

	aic3204_set_input_filename("../Multi_Tone.pcm");
    aic3204_set_output_filename("../output1.pcm");

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
    }

    for(i = 0; i < AUDIO_IO_SIZE; i++)
    {
    	dirakSample[i] = (i == 0? 32767 : 0);
    }

    while(1)
    {
    	aic3204_read_block(sampleBufferL, sampleBufferR);

    	/* Your code here */
    	/* Generisati koeficijente filtara za karakteristiku alpa = -0.3.
    	 * Iscrtati prenosnu karakteristiku shelving filtra za K = 8192 (sto odgovara vrednosti 0.25 skaliranoj na opseg int16) i K = 24576
    	 * Za potrebe iscrtavanja prenosne karakteristike filtra izracunati impulsni odziv dovodjenjem dirakovog impulsa u trajanju od N odbiraka na ulaz filtra.*/
    	calculateShelvingCoeff(-0.3,coeffLP);
    	for (i = 0; i < AUDIO_IO_SIZE; i++)
    	{
    		ShellBufferLP[i] = shelvingLP(dirakSample[i], coeffLP ,x_historyLP, y_historyLP, 8192);
    	}






    	/* Generisati koeficijente filtra za karakteristiku alpha = -0.3.
    	 * Iscrtati prenosnu karakteristiku shelving filtra za K = 8192 (sto odgovara vrednosti 0.25) i K = 24576 (0.75).
    	 * Za potrebe iscrtavanja prenosne karakteristike filtra izracunati impulsi odziv dovodjenjem dirakovog impulsa u trajanju od N odabiraka na ulaz filtra*/
    	calculateShelvingCoeff(-0.3, coeffLP);
    	for (i = 0; i < AUDIO_IO_SIZE; i++)
    	    	{
    				ShellBufferHP[i] = shelvingLP(dirakSample[i], coeffLP ,x_historyHP, y_historyHP, 8192);
    	    	}






    	/* Generisati koeficijente filtra za karakteristiku alpha = 0.7 i beta = 0. (greska?)
    	 * Iscrtati prenosnu karakteristiku shelving filtra za k = 8192 i k = 24576.
    	 * Za potrebe iscrtavanja prenosne karakteristike filtra izracunati impulsni odziv dovodjenjem dirakovog impulsa u trajanju od N odbiraka na ulaz filtra */
    	calculatePeekCoeff(0.7, 0.3, coeffLP);
    	for (i = 0; i < AUDIO_IO_SIZE; i++)
    	    	{
    	    		ShellBufferPeek[i] = shelvingLP(dirakSample[i], coeffLP ,x_historyPeek, y_historyPeek, 8192);
    	    	}






		aic3204_write_block(sampleBufferR, sampleBufferR);
	}

    	
	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}

