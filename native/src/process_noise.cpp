#include "./process_noise.h"



double randZeroToOne()
{
    return rand() / (RAND_MAX + 1.);
}

double exp_sample(double mean)
{
    double rand_number = randZeroToOne();
    return -mean * log(1 - rand_number);
}

int getLaplaceNoise(double sensitivity, double epsilon)
{
    double e1, e2;
    double scale = sensitivity / epsilon;
    e1 = exp_sample(scale);
    e2 = exp_sample(scale);
    return (int)(e1 - e2);

}

double getLaplaceNoiseRange(float sensitivity, float epsilon, float prob)
{
   
    float scale = sensitivity / epsilon;
    float loc = 0;

    laplace_distribution<> lp_dist(loc, scale);

    // Distributional properties
    float laplace_quantile = (1 - prob);

    float max_noise = quantile(lp_dist, 1 - laplace_quantile/2);

    float min_noise = quantile(lp_dist, laplace_quantile);

    return max_noise;
}

