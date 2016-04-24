#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
/*
** dumping rtlsdr output in various form to plot with gnuplot
*/
uint16_t maglut[256*256];
void mk_maglut()
	{
	int	i, q;

	for (i = 0; i <= 255; i++)
		{
		for (q = 0; q <= 255; q++)
			{
			int mag, mag_i, mag_q;

			mag_i = (i * 2) - 255;
			mag_q = (q * 2) - 255;

			mag = (int) round((sqrt((mag_i*mag_i)+(mag_q*mag_q)) * 258.433254) - 365.4798);

			maglut[(i*256)+q] = (uint16_t) ((mag < 65535) ? mag : 65535);
			}
		}
	}
int16_t philut[256*256];
void mk_philut()
	{
	int	i, q, phi;

	for (i = 0; i <= 255; i++)
		{
		for (q = 0; q <= 255; q++)
			{
			float mag_i, mag_q;

			mag_i = (i * 2) - 255;
			mag_q = (q * 2) - 255;

			phi = atanf(mag_q/mag_i)*180.0/3.141592;
			if (mag_i > 0.0)
				{
				if (mag_q < 0)
					phi += 360;
				}
			else	{
				phi += 180;
				}
			philut[(i*256)+q] = (int16_t) phi;
			}
		}
	}

void constellation()
	{
	unsigned char buf[256];
	int	i, len;

	while ((len = read(0, buf, sizeof(buf))) > 0)
		{
		for (i = 0; i < len; i += 2)
			{
			printf("%d %d\n", buf[i], buf[i+1]);
			}
		}
	}
void magnitude()
	{
	uint16_t buf[128];
	int	i, len;

	mk_maglut();
	while ((len = read(0, buf, sizeof(buf))) > 0)
		{
		len /= 2;
		for (i = 0; i < len; i += 1)
			{
			printf("%d\n", maglut[buf[i]]/258);
			}
		}
	}
void threshold()
	{
	int	i, len, tot_len;
	uint16_t buf[256];


	mk_maglut();
	tot_len = 0;
	while ((len = read(0, buf, sizeof(buf))) > 0)
		{
		len /= 2;
		for (i = 0; i < len; i += 1)
			{
			if ((maglut[buf[i]]/258) > 50)
				{
				printf("dd skip=%d bs=2 count=4096\n", tot_len+i);
				i += 4096;
				}
			}
		tot_len += len;
		}
	}
void phi()
	{
	int	i, len, tot_len;
	uint16_t buf[256];
	int iqi, iqq;

	mk_philut();
	tot_len = 0;
	while ((len = read(0, buf, sizeof(buf))) > 0)
		{
		len /= 2;
		for (i = 0; i < len; i += 1)
			{
#ifdef VERBOSE
iqi= buf[i] >> 8; iqq=buf[i] & 0xff;
iqi = 2*iqi -255; iqq=2*iqq-255;
printf("%d %d -> %d\n", iqi, iqq, philut[buf[i]]);
#endif
			printf("%d\n", philut[buf[i]]);
			}
		}
	}
void dphi()
	{
	int	i, len, tot_len;
	uint16_t buf[256];
	int	phi, phi0=0;

	mk_philut();
	tot_len = 0;
	while ((len = read(0, buf, sizeof(buf))) > 0)
		{
		len /= 2;
		for (i = 0; i < len; i += 1)
			{
			phi = (360 + philut[buf[i]] - phi0) % 360;
			if (phi > 180)
				phi = 360 - phi;
			printf("%d\n", phi);
			phi0 = philut[buf[i]];
			}
		}
	}
void usage(char *str)
	{
	fprintf(stderr, "%s: usage is '%s -[mcpt]'\n", str, str);
	fprintf(stderr, "\t-m : magnitude\n");
	fprintf(stderr, "\t-c : constellation\n");
	fprintf(stderr, "\t-p : phi\n");
	fprintf(stderr, "\t-d : delta phi\n");
	fprintf(stderr, "\t-t : threshold\n");
	}
void main(int argc, char *argv[])
	{
	char c;

	if (argc != 2)
		{
		usage(argv[0]);
		exit(-1);
		}

	c = getopt(argc, argv, "mcpdt");
	switch(c)
		{
		case 'm':
			magnitude();
			break;

		case 'c':
			constellation();
			break;

		case 'p':
			phi();
			break;

		case 'd':
			dphi();
			break;

		case 't':
			threshold();
			break;

		default:
			usage(argv[0]);
			exit(-1);
			break;
		}
	exit(0);
	}

