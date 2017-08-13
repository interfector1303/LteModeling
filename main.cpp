#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <list>
#include <map>
#include <random>
#include <chrono>

#include "CsvFile.h"

void checkExponental()
{
	double l_lambda = 0.5;
	int l_seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine l_generator(l_seed);
	std::exponential_distribution<double> l_expDistribution(l_lambda);

	std::list<double> l_times;
	std::map<double, int> l_stats;
	int l_n = 1000;

	for (int i = 0; i < l_n; ++i)
	{
		l_times.push_back(l_expDistribution(l_generator));
	}

	for (double i = 0.0; i < 5; i += 0.1)
	{
		for (double t : l_times)
		{
			if (t < i)
			{
				l_stats[i]++;
			}
		}
	}

	try
	{

		CsvFile l_expGraph("graphs\/exp_graph_lambda_0_5.csv", "\t");

		l_expGraph << "t" << "p_{exp}, {/Symbol l}=0.5" << "p_{theor}, {/Symbol l}=0.5" << endrow;

		for (auto l_st : l_stats)
		{
			l_expGraph << l_st.first << ((double)l_st.second / l_n) << (1 - exp(-l_lambda * l_st.first)) << endrow;
			/*std::cout << l_st.first << ":" << (double)l_st.second / l_n << " ## " << (1 - exp(-l_lambda * l_st.first)) << std::endl;*/
		}
	}
	catch (std::exception& l_ex)
	{
		std::cout << l_ex.what() << std::endl;
	}
}

std::list<double> generatePoissonProcess(const double p_duration, const double p_lambda)
{
	int l_seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine l_generator(l_seed);
	std::exponential_distribution<double> l_expDistribution(p_lambda);

	std::list<double> l_arrivalTimes;
	double l_t = l_expDistribution(l_generator);

	while (l_t < p_duration)
	{
		l_arrivalTimes.push_back(l_t);
		l_t += l_expDistribution(l_generator);
	}

	return l_arrivalTimes;
}

long long factorial(long long p_n)
{
	return (p_n == 1 || p_n == 0) ? 1 : factorial(p_n - 1) * p_n;
}

double poissonProbability(const int p_k, const double p_lambda)
{
	return pow(p_lambda, p_k) * exp(-p_lambda) / factorial(p_k);
}

void checkPossonProcess()
{
	int l_n = 100000;
	int l_lambda = 4;
	int l_duration = 10;

	std::map<int, int> l_stats;

	for (int i = 0; i < l_n; ++i)
	{
		std::list<double> l_poissonProcess = generatePoissonProcess(l_duration, l_lambda);
		l_stats[l_poissonProcess.size()]++;
	}

	try
	{

		CsvFile l_graph("graphs\/poisson_process.csv", "\t");

		l_graph << "n" << "p_{exp}" << "p_{theor}" << endrow;

		for (auto l_st : l_stats)
		{
			l_graph << l_st.first << (double)l_st.second / l_n << poissonProbability(l_st.first, l_lambda * l_duration) << endrow;
		}
	}
	catch (std::exception& l_ex)
	{
		std::cout << l_ex.what() << std::endl;
	}
}

void main()
{
	checkExponental();
	checkPossonProcess();
}