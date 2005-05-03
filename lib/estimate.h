//estimate.h			  	Copyright 2005 by Ben Klemens. Licensed under the GNU GPL.
#ifndef __apop_estimate__
#define __apop_estimate__

#include <gsl/gsl_matrix.h>

typedef struct apop_i{
	int	parameters, covariance, confidence, predicted, residuals, log_likelihood;
} apop_inventory;

typedef struct apop_e{
	gsl_vector 	*parameters, *params, *confidence, *predicted, *residuals;
	gsl_matrix 	*covariance, *cov;
	double		log_likelihood;
	apop_inventory	uses;
} apop_estimate;

apop_estimate *	apop_estimate_alloc(int data_size, int param_size, apop_inventory uses);
void 		apop_estimate_free(apop_estimate * free_me);
void 		apop_print_estimate(apop_estimate * print_me, char *out);

void 		apop_copy_inventory(apop_inventory in, apop_inventory *out);
void 		apop_set_inventory(apop_inventory *out, int value);
#endif