/** \file apop_update.c The \c apop_update function.

(c) 2007 Ben Klemens. Licensed under the GNU GPL v2.

The header is in asst.h.
 */
#include <apop.h>

static void write_double(const double *draw, apop_data *d){
  int i;
  int size = (d->vector ? d->vector->size : 0) + (d->matrix ? d->matrix->size1 * d->matrix->size2 : 0);
  gsl_vector *v = gsl_vector_alloc(size);
    for (i=0; i< v->size; i++)
        gsl_vector_set(v, i, draw[i]);

    apop_data *d2   = apop_data_unpack(v, (d->vector ? d->vector->size : 0) , (d->matrix ? d->matrix->size1: 0) , (d->matrix ?d->matrix->size2 : 0));
    apop_data_memcpy(d,d2);
    apop_data_free(d2);
    gsl_vector_free(v);
}

static apop_model *check_conjugacy(apop_data *data, apop_model prior, apop_model likelihood){
  apop_model   *outp;
    if (!strcmp(prior.name, "Gamma distribution") && !strcmp(likelihood.name, "Exponential distribution")){
        outp = apop_model_copy(prior);
        apop_vector_increment(outp->parameters->vector, 0, data->matrix->size1*data->matrix->size2);
        apop_vector_increment(outp->parameters->vector, 1, apop_matrix_sum(data->matrix));
        return outp;
    }
    if (!strcmp(prior.name, "Beta distribution") && !strcmp(likelihood.name, "Binomial distribution")){
        outp = apop_model_copy(prior);
        double  y   = apop_matrix_sum(data->matrix);
        apop_vector_increment(outp->parameters->vector, 0, y);
        apop_vector_increment(outp->parameters->vector, 1, data->matrix->size1*data->matrix->size2 - y);
        return outp;
    }
    if (!strcmp(prior.name, "Beta distribution") && !strcmp(likelihood.name, "Bernoulli distribution")){
        outp = apop_model_copy(prior);
        double  sum     = 0;
        int     i, j, n = (data->matrix->size1*data->matrix->size2);
        for (i=0; i< data->matrix->size1; i++)
            for (j=0; j< data->matrix->size2; j++)
                if (gsl_matrix_get(data->matrix, i, j))
                    sum++;
        apop_vector_increment(outp->parameters->vector, 0, sum);
        apop_vector_increment(outp->parameters->vector, 1, n - sum);
        return outp;
    }
    return NULL;
}

/** Take in a prior and likelihood distribution, and output a posterior
 distribution.

This function first checks a table of conjugate distributions for the pair
you sent in. If the names match the table, then the function returns a
closed-form model with updated parameters.  If the parameters aren't in
the table of conjugate priors/likelihoods, then it uses Markov Chain Monte
Carlo to sample from the posterior distribution, and then outputs an \c
apop\_histogram model for further analysis. Notably, it can be used as
the input to this function, so you can chain Bayesian updating procedures.

\param data     The input data, that will be used by the likelihood function
\param  prior   The prior \c apop_model
\param likelihood The likelihood \c apop_model. If the system needs to
estimate the posterior via MCMC, this needs to have a \c draw method.
\param prior_eps    The \c apop_model for the prior model
\param likelihood_eps    The \c apop_model for the likelihood model
\param starting_pt      The first parameter to check in the MCMC routine
\param r        A \c gsl_rng, already initialized (e.g., via \c apop_rng_alloc).
\param periods How many steps should the MCMC chain run?
\param burnin  What <em>percentage</em> of the periods should be ignored as initialization. That is, this is a number between zero and one.
\param histosegments If outputting a \c apop\_histogram, how many segments should it have?
\return an \c apop_model struct, including a \c model element and correctly specified parameters.
\todo The \c apop_update routine has an internal table of conjugate prior/posteriors (in its static \c check_conjugacy subfuction), and that list can always be longer.
*/
apop_model * apop_update(apop_data *data, apop_model prior, apop_model likelihood, 
                        apop_data *starting_pt, gsl_rng *r, int periods, double burnin, int histosegments){
  apop_model *maybe_out = check_conjugacy(data, prior, likelihood);
    if (maybe_out) return maybe_out;
  double        ratio;
  int           i;
  int           vs  = likelihood.vbase >= 0 ? likelihood.vbase : data->matrix->size2;
  int           ms1 = likelihood.m1base >= 0 ? likelihood.m1base : data->matrix->size2;
  int           ms2 = likelihood.m2base >= 0 ? likelihood.m2base : data->matrix->size2;
  double        *draw               = malloc(sizeof(double)* (vs+ms1*ms2));
  apop_data     *candidate_param    = apop_data_alloc(vs , ms1 , ms2),
                *current_param      = apop_data_alloc(vs , ms1 , ms2);
  apop_data     *out                = apop_data_alloc(0, periods*(1-burnin), vs+ms1*ms2);
    if (starting_pt)
        apop_data_memcpy(current_param, starting_pt);
    else{
        if (current_param->vector) gsl_vector_set_all(current_param->vector, 1);
        if (current_param->matrix) gsl_matrix_set_all(current_param->matrix, 1);
    }
    for (i=0; i< periods; i++){     //main loop
        prior.draw(draw, r, &prior);
        write_double(draw, candidate_param);
        ratio = likelihood.log_likelihood(candidate_param, data,&likelihood)
                    -likelihood.log_likelihood(current_param, data, &likelihood);
        if (gsl_isnan(ratio)){
            apop_error(0, 'c',"Trouble evaluating the likelihood function at vector beginning with %g or %g. Maybe offer a new starting point.\n", current_param->vector->data[0], candidate_param->vector->data[0]);
            return NULL;
        }
        if (ratio >=0 || log(gsl_rng_uniform(r)) < ratio)
            apop_data_memcpy(current_param,candidate_param);
        if (i >= periods * burnin){
            APOP_ROW(out, i-(periods *burnin), v)
            gsl_vector *vv = apop_data_pack(current_param);
            gsl_vector_memcpy(v, vv);
            gsl_vector_free(vv);
        }
    }
    apop_model *outp   = apop_histogram_params_alloc(out, histosegments, NULL);
    apop_data_free(out);
    return outp;
}
