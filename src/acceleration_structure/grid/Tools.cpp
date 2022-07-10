#include "Common.h"
#include "Tools.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

void SubdivideVertexStr(char* str, char* subStr[])
{
    char* p = str;
    subStr[0] = str;
    int figuerIndex = 0;
    while (*p != '\0')
    {
        if (*p == ' ' || *p == '\n')
        {
            *p = '\0';
            subStr[++figuerIndex] = p + 1;                        
        }
        p++;
    }
}

Scalar custom_atof(const char* str)
{
	char *p = (char *) str;

	// Handle optional sign
	int negative = 0;
	switch (*p) 
	{
	case '-': negative = 1; // Fall through to increment position
	case '+': p++;
	}	

	// Process string of digits
	Scalar number = 0.;
	int num_digits = 0;	
	while (isdigit(*p))
	{
		number = number * Scalar(10.) + (*p - '0');
		p++;
		num_digits++;
	}

	// Process decimal part
	int num_decimals = 0;
	int exponent = 0;
	if (*p == '.') 
	{
		p++;
		while (isdigit(*p))
		{
			number = number * Scalar(10.) + (*p - '0');
			p++;
			num_digits++;
			num_decimals++;
		}

		exponent -= num_decimals;
	}

	// Correct for sign
	if (negative) 
		number = -number;

	// Process an exponent string
	if (*p == 'e' || *p == 'E') 
	{
		// Handle optional sign
		negative = 0;
		switch(*++p) 
		{   
		  case '-': negative = 1;   // Fall through to increment pos
		  case '+': p++;
		}

		// Process string of digits
		int n = 0;
		while (isdigit(*p)) 
		{   
			n = n * 10 + (*p - '0');
			p++;
		}

		if (negative)
			exponent -= n;
		else
			exponent += n;
	}

	// Scale the result
	Scalar p10 = exponent > 0 ? Scalar(10.) : Scalar(0.1);
	int n = exponent;
	if (n < 0) 
		n = -n;
	while (n) 
	{
		if (n & 1) 
		{
			number *= p10;
		}
		n >>= 1;
		p10 *= p10;
	}

	return number;
}

int cvt(Scalar arg, int ndigits, int& decpt, int& sign, char* buf)
{
    decpt = 0;

    // Get the sign.
    sign = 0;
    if (arg < 0)
    {
        sign = 1;
        arg = -arg;
    }

    // Split the Scalar into integral part and fraction part.
    Scalar frac = 0;
    Scalar intg = 0;
    frac = modf(arg, &intg);
        
    char* p = &buf[CVTBUFSIZE];
    char* q = buf;
    if (intg != 0) 
    {        
        // Convert the integral part to string if integral part isn't 0.
        // First, put the string at the end of the buffer.
        // Because the digit is fetched from low to high, 
        // and the end position is not determined yet.
        p = &buf[CVTBUFSIZE];
        while (intg != 0) 
        {
            Scalar digit = modf(intg / 10, &intg);
            *--p = char((int)((digit + .03) * 10) + '0');
            decpt++;
        }

        // After the conversion, copy to the start of the buffer.
        q = buf;
        while (p < &buf[CVTBUFSIZE])
            *q++ = *p++;
    } 
    else if (frac > 0)
    {
        // If < 1, skip the non-significant 0 of the fraction part.
        Scalar temp;
        while ((temp = frac * 10) < 1) 
        {
            frac = temp;
            decpt--;
        }
    }

    // Convert the fraction part to string.
    while (q <= &buf[ndigits])
    {
        frac *= 10;
        frac = modf(frac, &intg);
        *q++ = char((int)intg + '0');
    }    

    // Round.
	q = &buf[ndigits];
	*q += 5;
	while (*q > '9') 
	{
		*q = '0';
		if (q > buf)
			++(*--q);
		else 
		{
			*q = '1';
			decpt++;
		}
	}

	// Eliminate 0 at the end.
	q = &buf[ndigits - 1];
	while (*q == '0' && q-- > buf);
    *(++q)= '\0';

    return int(q - buf);
}

int Scalar2str_g(Scalar arg, char* buf)
{
	if (arg == 0)
	{
		buf[0] = '0';
		buf[1] = '\0';
		return 1;
	}

	// Get significant digits.
	char cvtBuf[CVTBUFSIZE];
	int decpt = 0;
	int sign = 0;
	int len = cvt(arg, 6, decpt, sign, cvtBuf);

	// Get real buf.
	char* p = cvtBuf;
	char* q = buf;
	if (decpt < -4 || decpt > 6)
	{
		// Scientific.		
		if (sign == 1)
		{
			*q++ = '-';
		}

		// First digit and dot.
		*q++ = *p++;
		if (p < cvtBuf + len)
		{
			*q++ = '.';
		}

		// Copy the next digits.
		while (p < cvtBuf + len)
		{
			*q++ = *p++;
		}

		// E.
		*q++ = 'e';

		// Exponential.
		int exp = decpt - 1;
		if (exp < 0)
		{
			*q++ = '-';
			exp = -exp;
		}
		p = &buf[BUFSIZE];
		while (exp > 0)
		{
			*(--p) = exp % 10 + '0';
			exp /= 10;
		}
		while (p < buf + BUFSIZE)
			*q++ = *p++;
        
		*q = '\0';
	}
	else
	{
		if (sign == 1)
		{
			*q++ = '-';
		}

		// Get the integral part.
		if (decpt <= 0)
		{
			*q++ = '0';
		}
		else
		{
			for (int i = 0; i < decpt; ++i)
				*q++ = *p++;
		}

		// Dot.
		if (p < cvtBuf + len)
		{
			*q++ = '.';
		}

		// Get the fraction part.
		for (int i = 0; i > decpt; --i)
			*q++ = '0';
		while (p < cvtBuf + len)
			*q++ = *p++;

        *q = '\0';
	}

	return int(q - buf);
}

int vertex2str_g(Scalar* vertex, char* buf)
{
    char* p = buf;    
    int length = Scalar2str_g(*vertex, p);
    p += length;
    *p++ = ' ';

    length = Scalar2str_g(*(vertex + 1), p);
    p += length;
    *p++ = ' ';

    length = Scalar2str_g(*(vertex + 2), p);
    p += length;
    *p++ = '\r';
    *p++ = '\n';
    *p = '\0';
    return int(p - buf);
}