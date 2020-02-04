#include "tinythreads.h"
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>


//Global PP
int pp;

//Create mutex
mutex mutexPrime = MUTEX_INIT;

void LCDInit(void) {
	
	//Set Lowpower Waveform, no frame interrupt, no blanking. LCD Enable
	LCDCRA = (1 << LCDAB) | (1 << LCDEN);
	
	//drive time 300 microseconds, contrast control voltage 3.35 V.
	LCDCCR = (1 << LCDCC0) | (1 << LCDCC1) | (1 << LCDCC2) | (1 << LCDCC3);
	
	//external asynchronous clock source, 1/3 bias, 1/4 duty cycle, 25 segments.
	LCDCRB = (1 << LCDCS) | (1<< LCDMUX0) | (1<< LCDMUX1) | (1 <<LCDPM0) | (1 <<LCDPM1) | (1 <<LCDPM2);
	
	//prescaler setting N=16, clock divider setting D=8
	LCDFRR = (1 << LCDCD0) | (1 << LCDCD1) | (1 << LCDCD2);

}

void writeChar(char ch, int pos)
{
	uint8_t *lcdaddr = (uint8_t *) 0xec;	//The Address to the first position on the LCD.
	uint8_t mask;							//Mask to get only the nibble of a value.
	uint8_t nibbleNumber = 0x0;				//Nibble the number that is sent to the LCD.
	
	//SCC Table with the numbers from 0 to 9.
	uint16_t sccTable[10] = {0x1551, 0x0110, 0x1e11, 0x1B11, 0x0B50, 0x1B41, 0x1F41, 0x0111, 0x1F51, 0x0B51};
	
	// Check if position is outside or not.
	if (pos < 0 || pos > 5) {
		return;
	}
	
	uint16_t number = 0x0;	// The number to print.
	
	// Check if it is a 0 to 9.
	if (ch >= '0' || ch <= '9')
	{
		number = sccTable[ch - '0'];	//Get the number out of the array.
	}
	
	lcdaddr += pos >> 1;	//Point to the right position. Divide by 2 you can say.
	
	//Check if it is odd or even position.
	if (pos % 2 == 0)
	{
		mask = 0xf0;
	}
	else
	{
		mask = 0x0f;
	}
	
	//Will place out the nibbles on the right LCD address for the number.
	for (int i = 0; i < 4; i++ )
	{
		//Masking the smallest byte.
		nibbleNumber = number & 0xf;
		number = number >> 4;
		
		if(pos % 2 != 0)	//Check position.
		{
			nibbleNumber = nibbleNumber << 4;	//Shift the nibble to the right pos.
		}
		
		*lcdaddr = (*lcdaddr & mask) | nibbleNumber;	//Send the nibble.
		lcdaddr += 5;
	}
}

//Calculates the prime.
bool is_prime(long i)
{
	//Loop all the numbers under i and try to divide it with them.
	for(int n = 2; n < i; n++)
	{
		if(i % n == 0)
		{
			return false;
		}
		
	}
	return true;
	
}

void printAt(long num, int pos) {
	//lock the mutex
	lock(&mutexPrime);
	pp = pos;
	writeChar( (num % 100) / 10 + '0', pp);
	pp++;
	writeChar( num % 10 + '0', pp);
	//Unlock the mutex
	unlock(&mutexPrime);
}

//Counts the primes.
void computePrimes(int pos) {
	long n;

	for(n = 1; ; n++) {
		if (is_prime(n)) {
			printAt(n, pos);
//			yield();
		}
	}
}

int main() {
	
	CLKPR = 0x80;
	CLKPR = 0x00;
	
	LCDInit();
	spawn(computePrimes, 0);
	computePrimes(3);
}