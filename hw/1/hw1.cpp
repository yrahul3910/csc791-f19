#include <random>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

class Col {
protected:
	int n;
};

class Sym : Col {
};

class Some : Col {
};

class Num : public Col {
    double mean;
	double M2;  // M_{2, n} from Wikipedia

public:
	Num() {
		mean = 0;
		n = 0;
		M2 = 0;
	}

	// Returns sample sd
	double get_var() {
		if (n < 2) return 0;
		return M2 / (n - 1);
	}

	// Returns mean
	double get_mean() {
		return mean;
	}
	
	/**
	 * Updates the mean and M2 values using
	 * Welford's online algorithm.
     * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
	 *
	 * @param val - The new value
	 */
	void operator+=(double val) {
		n++;
		double delta = val - mean;
		mean += delta / n;
		M2 += delta * (val - mean);
	}
	
	/**
	 * Updates the mean and standard deviance using
	 * Welford's online algorithm.
	 *
	 * @param val - The new value
	 */
	void operator-=(double val) {
		if (n < 2) {
			n = 0;
			mean = 0;
			M2 = 0;
		} else {
			n--;
			double delta = val - mean;
			mean -= delta / n;
			M2 -= delta * (val - mean);
		}
	}
};

int main() {
	// Generate 100 random numbers
	// from https://stackoverflow.com/a/13445752
	double lower_bound = 0;
	double upper_bound = 100;
	std::uniform_int_distribution<int> dist(lower_bound, upper_bound);
    std::random_device rnd;
	std::mt19937 re(rnd());
	
	std::vector<int> rand;

	for (int i = 0; i < 100; i++)
		rand.push_back(dist(re));

	// Add numbers one by one to Num instance, while caching
	// the mean and sd every 10 numbers
	Num num;
	std::vector<double> mu, sd;
	
	for (int i = 0; i < 100; i++) {
		num += rand[i];

		if ((i + 1) % 10 == 0) {
			// Round to 2 decimal places
			mu.push_back(std::roundf(num.get_mean() * 100) / 100);
			sd.push_back(std::roundf(std::sqrt(num.get_var()) * 100) / 100);
		}
	}
	
	std::cout << "After adding numbers:\nmu = [";
	for (auto&& x : mu) std::cout << x << " ";
	std::cout << "]\nsd = [";
	
	for (auto&& x : sd) std::cout << x << " ";
	std::cout << "]\n";

	// Remove numbers one by one, checking that the mean and sd
	// are the same every 10 numbers
	std::vector<double> mu_sub, sd_sub;	
	
	for (int i = 99; i >= 9; --i) {
		if ((i + 1) % 10 == 0) {
            mu_sub.push_back(std::roundf(num.get_mean() * 100) / 100);
            sd_sub.push_back(std::roundf(std::sqrt(num.get_var()) * 100) / 100);
		}
		num -= rand[i];
	}
	
	std::reverse(mu_sub.begin(), mu_sub.end());
	std::reverse(sd_sub.begin(), sd_sub.end());
	
	std::cout << "\nAfter removing numbers:\nmu = [";
	for (auto&& x : mu_sub) std::cout << x << " ";
	std::cout << "]\nsd = [";
	
	for (auto&& x : sd_sub) std::cout << x << " ";
	std::cout << "]\n";
	return 0;
}
