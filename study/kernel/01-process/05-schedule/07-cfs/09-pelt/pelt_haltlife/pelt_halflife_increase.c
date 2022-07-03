#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#define HALFLIFE_32 32
#define LOAD_AVG_MAX_32 47742

#define HALFLIFE_8 8
#define LOAD_AVG_MAX_8 12326

#define HALFLIFE_4 4
#define LOAD_AVG_MAX_4 6430

#define HALFLIFE_3 3
#define LOAD_AVG_MAX_3 4959

#define HALFLIFE_2 2
#define LOAD_AVG_MAX_2 3493


void calc_converged_max(double y)
{
	int n = -1;
	/* first period */
	long max = 1024;
	long last = 0, y_inv = ((1UL << 32) - 1) * y;

	for (; ; n++) {
		if (n > -1) {
			max = ((max * y_inv) >> 32) + 1024;
			/* This is the same as: max = max*y + 1024; */
		}
		if (last == max)
			break;

		last = max;
	}
	n--;

	printf("#define LOAD_AVG_MAX %ld\n", max);
	printf("#define LOAD_AVG_MAX_N %d\n\n", n);
}


int util_avg_from_sudden_running_all_time(double y, int periods, int load_avg_max) {
	int i;
	double util_avg;
	double util_sum = 0;

	for (i = 0; i < periods; i++) {
		util_sum += 1024 * pow(y, i);

		util_avg = util_sum / load_avg_max;

		printf("util_sum=%d, periods=%d, util_avg=%%%d\n", (int)util_sum, i+1, (int)(util_avg * 100));
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

	printf("y^%d=0.5, periods=%d\n", choose, periods);

	switch(choose) {
		case 32:
			y = pow(0.5, 1/(double)HALFLIFE_32);
			calc_converged_max(y); //47742
			util_avg_from_sudden_running_all_time(y, periods, LOAD_AVG_MAX_32);
			break;
		case 8:
			y = pow(0.5, 1/(double)HALFLIFE_8);
			calc_converged_max(y); //12326
			util_avg_from_sudden_running_all_time(y, periods, LOAD_AVG_MAX_8);
			break;
		case 4:
			y = pow(0.5, 1/(double)HALFLIFE_4);
			calc_converged_max(y); //6430
			util_avg_from_sudden_running_all_time(y, periods, LOAD_AVG_MAX_4);
			break;
		case 3:
			y = pow(0.5, 1/(double)HALFLIFE_3);
			calc_converged_max(y); //4959
			util_avg_from_sudden_running_all_time(y, periods, LOAD_AVG_MAX_3);
			break;
		case 2:
			y = pow(0.5, 1/(double)HALFLIFE_2);
			calc_converged_max(y); //3493
			util_avg_from_sudden_running_all_time(y, periods, LOAD_AVG_MAX_2);
			break;
		default: break;
	}
}
