#include "processing.h"
#include "iir.h"

void calculateShelvingCoeff(float c_alpha, Int16* output)
{
	/* Your code here */
	output[0] = c_alpha	*32767; //A0
	output[1] = -1		*32767; //A1
	output[2] = 1		*32767; //B0
	output[3] =-c_alpha	*32767; //B1
}

void calculatePeekCoeff(float c_alpha, float c_beta, Int16* output)
{
	/* Your code here */
	output[0] = c_alpha						*32767; //A0
	output[1] = ((c_alpha + 1)*(-c_beta))	*16384; //A1
	output[2] =	1							*32767; //A2
	output[3] =	1							*32767; //B0
	output[4] = ((c_alpha + 1)*(-c_beta))	*16384; //B1
	output[5] = c_alpha						*32767; //B2
}

Int16 shelvingHP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */
	Int16 Buffer;
	Int16 Output;

	Buffer = first_order_IIR(input, coeff, x_history, y_history);
	Output = (k/2)*input + Buffer*(k/2) + (input - Buffer)/2;
	if (Output > 32767)
	{
		Output = 32767;
	}

	return Output;
}

Int16 shelvingLP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */
	Int16 Buffer;
	Int16 Output;
	//take hte history and iir filter first order
	Buffer = first_order_IIR(input, coeff, x_history, y_history);
	Output = input/2 + Buffer/2 + k*(input - Buffer)/2;

	if (Output > 32767)
		{
			Output = 32767;
		}

	return Output;
}

Int16 shelvingPeek(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */
	Int16 Buffer;
	Int16 Output;

	Buffer = second_order_IIR(input, coeff, x_history, y_history);
	Output = input/2 + Buffer/2 + k*(input - Buffer)/2;

	if (Output > 32767)
		{
			Output = 32767;
		}

	return Output;
}
