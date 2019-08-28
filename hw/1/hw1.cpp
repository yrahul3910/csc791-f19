#include <random>
#include <iostream>
#include <algorithm>
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
    double var;  // sample variance
	double M2;  // M_{2, n} from Wikipedia

public:
	Num() {
		mean = 0;
		var = 0;
		n = 0;
		M2 = 0;
	}

	// Returns sample sd
	double get_var() {
		return var;
	}

	// Returns mean
	double get_mean() {
		return mean;
	}

	/**
	 * Updates the mean and standard deviance using
	 * Welford's online algorithm.
     * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
	 *
	 * @param val - The new value
	 */
	void update(double val, bool add=true) {
		// Update n
		if (add)
			++n;
		else
			--n;

        double delta = val - mean;

		// Update mean
        if (add)
    		mean += delta / n;
		else
			mean -= delta / n;

		// Update sample sd
		if (add)
			M2 += delta * (val - mean);
		else
			M2 -= delta * (val - mean);

        if (M2 < 0)
            var = 0;
        else
    		var = M2 / (n - 1);
	}
};

int main() {
	// Generate 100 random numbers
	// from https://stackoverflow.com/a/13445752
	double lower_bound = 0;
	double upper_bound = 100;
	std::uniform_real_distribution<double> dist(lower_bound, upper_bound);
    std::random_device rnd;
	std::mt19937 re(rnd());
	
	std::vector<double> rand;

	for (int i = 0; i < 100; i++)
		rand.push_back(dist(re));

	// Add numbers one by one to Num instance, while caching
	// the mean and sd every 10 numbers
	Num num;
	std::vector<double> mu, var;
	for (int i = 0; i < 100; i++) {
		num.update(rand.at(i));

		if ((i + 1) % 10 == 0) {
			mu.push_back(num.get_mean());
			var.push_back(num.get_var());
		}
	}

	// Remove numbers one by one, checking that the mean and sd
	// are the same every 10 numbers
	for (int i = 99; i >= 9; --i) {
		num.update(rand.at(i), false);

		if ((i + 1) % 10 == 0) {
            double mean = num.get_mean();
            double sd = std::sqrt(num.get_var());
            double prev_mean = mu.at((i + 1) / 10 - 1);
            double prev_sd = std::sqrt(var.at((i + 1) / 10 - 1));

            double mean_dev = std::abs((mean - prev_mean) / prev_mean);
            double sd_dev = std::abs((sd - prev_sd) / prev_sd);

            if (mean_dev <= 0.05 && sd_dev <= 0.05)
                std::cout << "Test for " << i + 1 << " numbers passed.\n";
            else {
                std::cout << "Test for " << i + 1 << " numbers failed. ";
                std::cout << "Deviation of mean: " << (int)(mean_dev * 1e4) / 100.0 << "%";
                std::cout << ", Deviation of sd: " << (int)(sd_dev * 1e4) / 100.0 << "%\n";
            }
		}
	}
	return 0;
}
