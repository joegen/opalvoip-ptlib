/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.org> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * Extract DTMF signals from 16 bit PCM audio
 *
 * Originally written by Poul-Henning Kamp <phk@freebsd.org>
 * Made into a C++ class by Roger Hardiman <roger@freebsd.org>, January 2002
 *
 * $Log: dtmf.cxx,v $
 * Revision 1.3  2002/01/24 10:40:17  rogerh
 * Add version log
 *
 *
 */

#ifdef __GNUC__
#pragma implementation "dtmf.h"
#endif

#include <ptlib.h>
#include <ptclib/dtmf.h>

/* Integer math scaling factor */
#define FSC	(1<<12)

/* This is the Q of the filter (pole radius) */
#define POLRAD .99

#define P2 ((int)(POLRAD*POLRAD*FSC))

PDTMFDecoder::PDTMFDecoder() {
	// Initialise the class
	int i,kk;
	for (kk = 0; kk < 8; kk++) {
		y[kk] = h[kk] = k[kk] = 0;
	}

	nn = 0;
	ia = 0;
	so = 0;

	for (i = 0; i < 256; i++) {
		key[i] = '?';
	}

	/* We encode the tones in 8 bits, translate those to symbol */
	key[0x00] = '\0';

	key[0x11] = '1'; key[0x12] = '4'; key[0x14] = '7'; key[0x18] = '*';
	key[0x21] = '2'; key[0x22] = '5'; key[0x24] = '8'; key[0x28] = '0';
	key[0x41] = '3'; key[0x42] = '6'; key[0x44] = '9'; key[0x48] = '#';
	key[0x81] = 'A'; key[0x82] = 'B'; key[0x84] = 'C'; key[0x88] = 'D';

	/* The frequencies we're trying to detect */
	/* These are precalculated to save processing power */
	/* static int dtmf[8] = {697, 770, 852, 941, 1209, 1336, 1477, 1633}; */
	/* p1[kk] = (-cos(2 * 3.141592 * dtmf[kk] / 8000.0) * FSC) */
	p1[0] = -3497; p1[1] = -3369; p1[2] = -3212; p1[3] = -3027;
	p1[4] = -2384; p1[5] = -2040; p1[6] = -1635; p1[7] = -1164;
}

static int join(int a,int b)
{
  return (b << 8) | a;
}

PString PDTMFDecoder::Decode(const void *buf, PINDEX bytes) {
	int x;
	int s, kk;
	int c, d, f, n;
	unsigned char *buffer = (unsigned char*)buf;

	PString keyString;

	PINDEX pos;
	for (pos = 0; pos < (bytes-1); pos = pos+2) {

		/* Convert to our format */
		x = join(buffer[pos],buffer[pos+1]) / (32768/FSC);

		/* Input amplitude */
		if (x > 0)
			ia += (x - ia) / 128;
		else
			ia += (-x - ia) / 128;

		/* For each tone */
		s = 0;
		for(kk = 0; kk < 8; kk++) {

			/* Turn the crank */
			c = (P2 * (x - k[kk])) / FSC;
			d = x + c;
			f = (p1[kk] * (d - h[kk])) / FSC;
			n = x - k[kk] - c;
			k[kk] = h[kk] + f;
			h[kk] = f + d;

			/* Detect and Average */
			if (n > 0)
				y[kk] += (n - y[kk]) / 64;
			else
				y[kk] += (-n - y[kk]) / 64;

			/* Threshold */
			if (y[kk] > FSC/10 && y[kk] > ia)
				s |= 1 << kk;
		}

		/* Hysteresis and noise supressor */
		if (s != so) {
			nn = 0;
			so = s;
		} else if (nn++ == 520 && key[s]) {
			PTRACE(1,"Got 16bit PCM DTMF " << key[s]);
			keyString = keyString + key[s];
		}
	}
	return keyString;
}
