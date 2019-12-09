#include "./process_noise.h"

float getNoiseFromHyper(int N, int v, int L, double prob)
{
    hypergeometric_distribution<> hyper_dist(L, v, N);
    float result = quantile(complement(hyper_dist, prob));
    cout << "Value in prob " << prob << ": " << result << endl;
    return result;
}

double randZeroToOne()
{
    return rand() / (RAND_MAX + 1.);
}

double exp_sample(double mean)
{
    double rand_number = randZeroToOne();
    return -mean * log(1 - rand_number);
}

int laplace_noise(double sensitivity, double epsilon)
{
    double e1, e2;
    double scale = sensitivity / epsilon;
    e1 = exp_sample(scale);
    e2 = exp_sample(scale);
    return (int)(e1 - e2);

    // To UPDATE: participant generate noises for COUNT query as an integer number,
    // take only noise values in range of [min, max]
}

double getNoiseRangeFromLaplace(float sensitivity, float epsilon, float prob)
{
    // float epsilon = 0.1;
    // float sensitivity = 1.0;
    float scale = sensitivity / epsilon;
    float loc = 0;

    laplace_distribution<> lp_dist(loc, scale);
    // cout << "loc " << lp_dist.location() << endl;
    // cout << "scale " << lp_dist.scale() << endl;

    // Distributional properties
    float probability = (1 - prob) / 2;

    float max_noise = quantile(lp_dist, 1 - probability);

    float min_noise = quantile(lp_dist, probability);

    // cout << "max_noise: " << max_noise << endl;
    // cout << "min_noise: " << min_noise << endl;

    return max_noise;
}

vector<double> estimate_conf_interval(double alpha, int PV_answer, int datasize, int PVsize)
{
    chi_squared_distribution<> chi_squared_distribution_min(PV_answer * 2);
    chi_squared_distribution<> chi_squared_distribution_max(PV_answer * 2 + 2);
    float min_answer = (datasize / (2 * PVsize)) * quantile(chi_squared_distribution_min, alpha / 2);
    float max_answer = (datasize / (2 * PVsize)) * quantile(chi_squared_distribution_max, 1 - alpha / 2);
    cout << "min answer= " << min_answer << "; max answer = " << max_answer << endl;
    vector<double> answers = {min_answer, max_answer};
    return answers;
}