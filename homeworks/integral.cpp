#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

class Timer {
    private:
    std::chrono::steady_clock::time_point begin;

    public:
    Timer() {
        begin = std::chrono::steady_clock::now();
    }
    ~Timer() {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsedTime = end - begin;
        std::cout << "Calculations time: " << elapsedTime.count() << std::endl;
    }
};

double integrationFunction(double x) 
{
    return 4. / (1. + x * x);
}

void computeIntegral(double leftBound, double rightBound, double &sum, int threadNumber,
                     int processorsCount, int fragmentsNumber, std::mutex &mutex) 
{
    double s = 0;
    double h = (rightBound - leftBound) / fragmentsNumber;
    for (int i = threadNumber; i < fragmentsNumber; i+=processorsCount) {
        double fragment = leftBound + 0.5 * h + h * i;
        s += integrationFunction(fragment) * h;
    }
    std::unique_lock<std::mutex> lock(mutex);
    sum += s;
};



int main(const int argc, char const** argv) {

    const int processorsCount = std::thread::hardware_concurrency();
    std::cout << "Processors count: " << processorsCount << std::endl;

    int leftBound = std::atoi(argv[1]);
    int rightBound = std::atoi(argv[2]);
    int fragmentsNumber = std::atoi(argv[3]);
    
    std::vector<std::thread> vec;
    std::mutex mutex;
    double sum = 0;
    
    {
        Timer timer;
        for (int i = 0; i < processorsCount; ++i) {
            vec.emplace_back(std::thread(computeIntegral, leftBound, rightBound, std::ref(sum),
            i, processorsCount, fragmentsNumber, std::ref(mutex)));
        }
        for (auto &i : vec) {
            i.join();
        }
    }
    std::cout << "Integral sum: " << sum << std::endl;
    return 0;
}