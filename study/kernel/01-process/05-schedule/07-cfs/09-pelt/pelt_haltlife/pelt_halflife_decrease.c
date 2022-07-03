#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define HALFLIFE_32 32
#define HALFLIFE_8 8
#define HALFLIFE_4 4
#define HALFLIFE_3 3
#define HALFLIFE_2 2

int util_avg_from_sudden_sleep_all_time(double y, int periods) {
	int i;
	double util_avg;
	double util_sum = 0;

	for (i = 0; i < periods; i++) {
		util_avg = 1 * pow(y, i);
		printf("periods=%d, util_avg=%%%d\n", i+1, (int)(util_avg * 100));
	}

	return 0;
}

void main(int argc, char *argv[])
{
	double y;
	int choose = 32, periods = 200;

	if (argc == 2) {
		choose = atoi(argv[1]);
	}

	if (argc == 3) {
		choose = atoi(argv[1]);
		periods = atoi(argv[2]);
	}

	printf("y^%d=0.5, periods=%d\n\n", choose, periods);

	switch(choose) {
		case 32:
			y = pow(0.5, 1/(double)HALFLIFE_32);
			util_avg_from_sudden_sleep_all_time(y, periods);
			break;
		case 8:
			y = pow(0.5, 1/(double)HALFLIFE_8);
			util_avg_from_sudden_sleep_all_time(y, periods);
			break;
		case 4:
			y = pow(0.5, 1/(double)HALFLIFE_4);
			util_avg_from_sudden_sleep_all_time(y, periods);
			break;
		case 3:
			y = pow(0.5, 1/(double)HALFLIFE_3);
			util_avg_from_sudden_sleep_all_time(y, periods);
			break;
		case 2:
			y = pow(0.5, 1/(double)HALFLIFE_2);
			util_avg_from_sudden_sleep_all_time(y, periods);
			break;
		default: break;
	}
}
