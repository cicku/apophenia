//types.h			  	Copyright 2005 by Ben Klemens. Licensed under the GNU GPL.
#ifndef __apop_estimate__
#define __apop_estimate__

#include <gsl/gsl_rng.h>
#include <gsl/gsl_matrix.h>

/**\defgroup types Types defined by Apophenia. 

The basic story for a statistical analysis is that the researcher
assembles a data set into an \ref apop_data structure, then sends it to
an \ref apop_model so that the model's parameters can be estimated,
and that is returned in an \ref apop_estimate structure.

Supporting these main structures are a few more structures you'd only have to worry about 
for fine tuning.
The \ref apop_name
structure is an organized list of row and column names; functions that
take an \ref apop_data set try to automatically handle the names for you.
The \ref apop_inventory structure lists the elements that an \ref
apop_estimate has generated (no model produces everything). Model
estimates accept an inventory, but all work fine if you send in
NULL. Finally, the more elaborate models, such as MLEs, require some parameters to run,
in which case you will need to fill out an \ref apop_estimation_params
form and hand it in to the mode.


\li Data 
The \ref apop_data structure adds a touch of metadata on
top of the basic \c gsl_matrix. It includes an \ref apop_name structure
(see below), and a table for non-numeric variables. See \ref data_struct.

\li Models 
The \ref apop_model structure encapsulates a description of the world
in which the data and the parameters produce observed outcomes. The
apop_model.estimate() method takes in data and produces an \ref
apop_estimate. See \ref models.

\li Estimates
The \ref apop_estimate structure returns all the data one would want
from a regression or ML estimation, including the parameters estimated,
the variance/covariance matrix, the residuals, et cetera. The structure
includes instances of both of the strucutres below. See \ref inv_and_est.

\li Names 
The \ref apop_name structure has three components: a list of column
names, a list of row names, and a list of dependent variable names. It
is intended to accompany the <tt>gsl_matrix</tt> structure, which holds
all the other information about a data aaray such as the number of rows
and columns.  See \ref names.

\li Inventory
The \ref apop_inventory structure serves two purposes. It is an input
to a regression or ML estimation, tells the function what output you
would like the <tt>apop_estimate</tt> output to include. It is also an
output from these functions, since the returned <tt>apop_estimate</tt>
will include its own <tt>apop_inventory</tt>,  which can be used later on
to test whether any given element is in use. See \ref inv_and_est.

\li Estimate parameters
The \ref apop_estimation_params are the details for how an \ref
apop_estimate should do its work; currently it is just the specifications
for tolerances, step sizes, starting points, et cetera, for \ref apop_maximum_likelihood.
 See \ref inv_and_est.

*/

/** The structure has two uses. The first is to tell the regression/MLE
functions what you would like to receive in return. Alternatively, you
can just send in a <tt>NULL</tt> pointer, and the functions will return
everything apropos.

The second is for the internal workings of the \ref apop_estimate
structure, giving a list of the elements of the structure which are
actually in use. For example, the regressions won't return a log
likelihood, and the ML estimates won't return an  R^2.


\b the elements 
\verbatim
int     parameters, covariance, confidence, predicted, residuals, log_likelihood;
\endverbatim

There is one element for each element of the \ref apop_estimate structure.

If the <tt>apop_inventory</tt> will be sent in to a regression/MLE
function, set the appropriate element to either zero or one if you would
like the function to return the designated \ref apop_estimate element.

The \ref apop_estimate structure itself has an <tt>apop_inventory</tt>
element named <tt>uses</tt> embedded within it. Those elements for
which <tt>uses.elmt</tt> are zero are unallocated pointers (so be careful:
precede all dereferences with an <tt>if(est->uses.element)</tt> clause).

<b>functions</b><br>
\code
void apop_inventory_copy(apop_inventory in, apop_inventory *out);
\endcode
Copy the input inventory list to a new output list. Notice that the input list is an actual inventory, while the output is a pointer to an inventory (since it will be modified).

\code
void apop_inventory_set(apop_inventory *out, int value);
\endcode
Set all of the elements of the inventory to the value given, e.g.,
<tt>apop_set_inventory(&want_all, 1)</tt>. Clearly, <tt>value</tt>
should either be zero or one.

<b>notes </b><br>

Unlike almost everything else in the GSL and Apophenia, it is
generally assumed that <tt>apop_inventory</tt>s are not pointers, but
are automatically allocated. Notably, this is true of the <tt>uses</tt>
element of the \ref apop_estimate structure; therefore, to check whether
the variance-covariance matrix of an <tt>apop_estimate*</tt> is present,
for example, you would look at <tt>est->uses.covariance</tt>.


It may sometimes be useful to manipulate The ["apop_estimate"] structure's
internal <tt>apop_inventory</tt> element to your own benefit. For
example, if you set <tt>est->uses.residuals = 0</tt> before calling
<tt>apop_print_estimate(est, NULL)</tt>, then the residuals won't get
printed. But be careful: if you then call <tt>apop_estimate_free(est)</tt>,
then the residuals won't get freed, either.
*/
typedef struct apop_inventory{
	int	parameters, covariance, confidence, predicted, residuals, log_likelihood, names;
} apop_inventory;

#include <string.h>


/** A data set is assumed to be a matrix where each row is a single
observation and each column is a variable. Usually there is only one
dependent variable (the value to be predicted), which is the first column;
the independent variables (the predictors) follow thereafter.

This structure holds the names of these variables. You can fill it quickly
with \ref apop_db_get_names after running a query, or add names manually
with \ref apop_name_add .

Typically, the row names are not used, but they are there for your convenience.  
\ingroup names
*/
typedef struct apop_name{
	char ** colnames;
	char ** rownames;
	char ** depnames;
	char ** catnames;
	int colnamect, depnamect, rownamect, catnamect;
} apop_name;

apop_name * apop_name_alloc(void);
int apop_name_add(apop_name * n, char *add_me, char type);
void  apop_name_free(apop_name * free_me);
void  apop_name_print(apop_name * n);
void  apop_name_stack(apop_name * n1, apop_name *n2, char type);
void apop_name_rm_columns(apop_name *n, int *drop);

/** Parameters for running estimations. No estimation uses all of them.
  E.g., the MLE functions don't look at preserve_data but OLS and GLS do, while OLS and GLS ignore all the other params.
 \ingroup inv_and_est
 */
typedef struct apop_estimation_params{
	int 	method;
	double *starting_pt; 
	double 	step_size; 
	double 	tolerance; 
	int 	verbose;
	int 	preserve_data;
} apop_estimation_params;

/** Regression and MLE functions return this structure, which includes
the various elements that one would want from a model estimate.

If you need control of the types of information these functions return,
see the \ref apop_inventory page. [If you don't, just send <tt>NULL</tt>
every time a function asks for an <tt>apop_inventory*</tt> structure.]

<b>An example</b><br>

The \ref apop_OLS page has a sample program which uses an <tt>apop_estimate</tt> structure.

\param parameters 	The vector of coefficients or parameters estimated by the regression/MLE. Usually has as many dimensions as your data set has columns.
\param predicted 	The most likely values of the dependent variable. Has as many dimensions as your data set has columns.
\param residuals 	The actual values of the dependent var minus the predicted. Has as many dimensions as your data set has columns.
\param covariance 	The variance-covariance matrix (remember the variance is just the covariance of a variable with itself).
\param confidence 	The two-tailed test of the hypothesis that the variable is zero. One element for each parameter.
\param status		The return status from the estimate that had populated this apop_estimate, if any.
\ingroup inv_and_est
*/
typedef struct apop_estimate{
	gsl_vector 	*parameters, *confidence, *predicted, *residuals;
	gsl_matrix 	*covariance;
	double		log_likelihood;
	apop_inventory	uses;
	apop_name	*names;
	int		    status;
    struct apop_data   *data;
    struct apop_model  *model;
    struct apop_estimation_params  *estimation_params;
} apop_estimate;

/**
Gathers together a <tt>gsl_matrix</tt>, an \ref apop_name structure, and a space for a table of non-numeric data.
\ingroup data_struct
*/
typedef struct apop_data{
    gsl_matrix  *data;
    apop_name   *names;
    char        ***categories;
    int         catsize[2];
} apop_data;

/** This is an object to describe a model whose parameters are to be
estimated. It would primarily be used for maximum likelihood estimation,
but is intended to have anything else you would want a probability
distribution to have too, like a random number generator.  

\param name	The model name. You have 100 characters. 
\param parameter_ct	The number of parameters. If this is -1, it will be dynamically set to the size of the given data set minus one.
\param inventory_filter	an \ref apop_inventory indicating what the estimate fn is programmed to return.
\param estimate		the estimator fn, which is all most users will care about.
\param log_likelihood	the likelihood fn given data 
\param 	dlog_likelihood	the derivative of the likelihood fn
\param 	fdf	Do both of the above at once. Can be NULL if it'd just call them separately. 
\param 	constraint	The constraints to the parameters, if any. Really only necessary for MLEs.
\param rng 	a random number generator. 

\ingroup models
 */
typedef struct apop_model{
	char	name[101]; 
	int	parameter_ct;
	apop_inventory inventory_filter;
	apop_estimate *	(*estimate)(apop_data * data, apop_inventory *uses, void *parameters);
	double 	(*log_likelihood)(const gsl_vector *beta, void *d);
	void 	(*dlog_likelihood)(const gsl_vector *beta, void *d, gsl_vector *gradient);
	void 	(*fdf)( const gsl_vector *beta, void *d, double *f, gsl_vector *df);
    double  (*constraint)(gsl_vector *beta, void * d, gsl_vector *returned_beta);
	double (*rng)(gsl_rng* r, double *a);
} apop_model;

apop_estimate * apop_estimate_alloc(apop_data * data, apop_model model, apop_inventory *uses, apop_estimation_params *params);
void 		apop_estimate_free(apop_estimate * free_me);
void 		apop_estimate_print(apop_estimate * print_me);

apop_inventory * apop_inventory_alloc(int value);
void 		apop_inventory_copy(apop_inventory in, apop_inventory *out);
void 		apop_inventory_set(apop_inventory *out, int value);
apop_inventory apop_inventory_filter(apop_inventory *in, apop_inventory filter);

void        apop_data_free(apop_data *freeme);
apop_data * apop_matrix_to_data(gsl_matrix *m);
apop_data * apop_data_from_matrix(gsl_matrix *m);
apop_data * apop_data_alloc(int size1, int size2);
apop_data * apop_data_stack(apop_data *m1, apop_data * m2, char posn);
void        apop_data_rm_columns(apop_data *d, int *drop);
void apop_data_memcpy(apop_data **out, apop_data *in);

void apop_model_memcpy(apop_model *out, apop_model in); //this is in apop_estimate.c.
#endif
