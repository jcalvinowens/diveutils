/*
 * Altitude calculator: This tool uses a set of dives and surface intervals you
 * provide to calculate minimum ambiant pressures tolerable without exceeding
 * the M-values (+GF) for various Haldanian decompression models.
 *
 * Copyright (C) 2017 Calvin Owens <jcalvinowens@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>

#include "args.h"
#include "haldanian.h"

static double __get_palt(double pressure)
{
	if (pressure == 0.0)
		return HUGE_VAL;

	return (1.0 - pow(pressure / 1.01325, 0.190284));
}

static double get_palt_ft(double pressure)
{
	return __get_palt(pressure) * 145366.45;
}

static double get_palt_m(double pressure)
{
	return __get_palt(pressure) * 44307.693;
}

static const char *usage =
"This tool uses a set of dives and surface intervals you provide to calculate\n"
"minimum ambiant pressures tolerable without exceeding the M-values (+GF) for\n"
"various Haldanian decompression models.\n";

int main(int argc, char **argv)
{
	const struct compartment *compartment;
	double *ceils, *ptr;
	const double *load;
	struct params params = {
		.usage = usage,

		.surface = 1.013,
		.ppn2 = 0.79,
		.gf = 1.0,
		.negvals = 0,
	};

	parse_arguments(argc, argv, &params);

	puts("Computing loads...");
	compute_loading(params.tissues, params.vectors, params.nr_vectors);

	puts("Computing ceilings...");
	ceils = calloc(params.tissues->model->count, sizeof(double));
	compute_ceilings(params.tissues, params.gf, ceils);

	ptr = ceils;
	for_each_tissue(params.tissues, compartment, load) {
		double tmp = *ptr++ * (1.0 / 0.79);
		tmp = params.negvals ? tmp : (tmp > 0.0 ? tmp : 0.0);

		printf("Load: %.3f Ceil: %.3f %5.fft %5.fm\n", *load, tmp,
		       get_palt_ft(tmp), get_palt_m(tmp));
	}

	return 0;
}
