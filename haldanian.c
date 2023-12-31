/*
 * haldanian.c: General naive implementation of Haldanian deco models
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

#include "haldanian.h"
#include "constants.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <err.h>

static const double pp_h2o = 0.0627;
static const double pp_co2 = 0.0534;
static const double resp_quot = 0.8;

static double alveolar_pressure(double pressure)
{
	return pressure - pp_h2o + (1.0 - resp_quot) / resp_quot * pp_co2;
}

/*
 * Compute tissue loads after spending some duration(s) at some pressure(s)
 */
void compute_loading(struct tissues *tissues, const struct vector *vectors,
		     int nr_vectors)
{
	const struct compartment *compartment;
	const struct vector *vector;
	double *load;

	for_each_vector(vectors, nr_vectors, vector) {
		for_each_tissue(tissues, compartment, load) {
			double nr = vector->time / compartment->halftime;
			double d = alveolar_pressure(vector->pressure) - *load;

			*load += (1.0 - 1.0 / exp2(nr)) * d;
		}
	}
}

/*
 * Compute the minimum ppressure that can safely be ascended to, given the loads
 * in @tissues as per the model, for each compartment.
 */
void compute_ceilings(const struct tissues *tissues, double gf,
		      double *ceilings)
{
	const struct compartment *compartment;
	const double *load;

	for_each_tissue(tissues, compartment, load) {
		double ceil = *load / compartment->mslope - compartment->mvalue;
		double diff = *load - ceil;

		*ceilings++ = *load - diff * gf;
	}
}

/*
 * Compute how long ambiant pressure can remain at @depth before the ceiling
 * reaches @surface, for each compartment.
 */
void compute_ndls(const struct tissues *tissues, double depth, double surface,
		  double gf, double *ndls)
{
	const struct compartment *compartment;
	const double *load;

	if (surface >= depth)
		errx(2, "Surface %.3f below depth %.3f!", surface, depth);

	depth = alveolar_pressure(depth);

	for_each_tissue(tissues, compartment, load) {
		double tgt;

		if (*load > depth) {
			*ndls++ = nan("");
			continue;
		}

		tgt = compartment->mslope * (surface + gf * compartment->mvalue);
		tgt /= gf * -compartment->mslope + gf + compartment->mslope;

		if (tgt > depth) {
			*ndls++ = HUGE_VAL;
			continue;
		}

		*ndls++ = log2((depth - *load) / (depth - tgt))
			  * compartment->halftime;
	}
}
