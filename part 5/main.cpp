#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <sstream>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <string>
#include <chrono>
// minisat
#include <minisat/core/Solver.h>
#include <minisat/core/SolverTypes.h>
// my functions
#include "myFunctions.hpp"

/*
defining global variables to communicate between threads
*/
// shared inputs V and E
int V;
std::vector<std::pair<int,int>> E;
// execution times
std::chrono::microseconds cnfThreadTime(0);
std::chrono::microseconds approx1ThreadTime(0);
std::chrono::microseconds approx2ThreadTime(0);
// results variables
std::pair<int, std::vector<int>> result_cnf;
std::pair<int, std::vector<int>> result_a1;
std::pair<int, std::vector<int>> result_a2;
// mutex and conditional variable for synchronization
std::mutex mtx;
std::condition_variable cv;
// boolean variable to make sure V and E are ready
bool inputs_ready = false;
// atomic flag to indicate whether program should exit
std::atomic<bool> should_exit(false);

/*
creat three functions to handle thread for each method
*/
void cnfSatThread(){
    int timeout = 15;
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] {return inputs_ready || should_exit.load();});
    if (!should_exit.load()){
        auto start = std::chrono::high_resolution_clock::now();  // start time
        result_cnf = binarySearchCoverage(V, E, timeout);
        auto end = std::chrono::high_resolution_clock::now();  // end time
        cnfThreadTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        inputs_ready = false;
    }
}

void approx1Thread(){
    std::unique_lock<std::mutex> lock(mtx);
    auto start = std::chrono::high_resolution_clock::now();  // start time
    result_a1 = approxVC1(E);
    auto end = std::chrono::high_resolution_clock::now();  // end time
    approx1ThreadTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
}

void approx2Thread(){
    std::unique_lock<std::mutex> lock(mtx);
    auto start = std::chrono::high_resolution_clock::now();  // start time
    result_a2 = approxVC2(E);
    auto end = std::chrono::high_resolution_clock::now();  // end time
    approx2ThreadTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
}


int main(){
    // send output to a file
    std::ofstream output("./output.csv");
    // headers
    output << "V,approx1-ratio,approx2-ratio,cnf-sat-time,approx1-time,approx2-time" << std::endl;
    // thread for I/O
    std::thread ioThread([&](){
        while (true){
            std::string line;
            std::getline(std::cin,line);
            if (std::cin.eof()){
                break;
            }
            if (line.empty()){
                continue;
            }
            // parse input of V and E
            char firstChar = line[0]; // V or E
            std::istringstream iss(line.substr(1)); // remainder of input
            // get V
            if (firstChar == 'V'){
                iss >> V;
            }
            // get E
            if (firstChar == 'E'){
                E.clear(); // for each new E
                std::string tempStr;
                iss >> tempStr; // reading in input string
                // remove curly brackets
                tempStr = tempStr.substr(1, tempStr.length()-1);
                std::istringstream edgeStr(tempStr);
                // structure of E to parse E to vector
                char comma1, comma2, comma3, openAngle, closeAngle;
                int x,y;
                while (edgeStr >> openAngle >> x >> comma1 >> y >> closeAngle >> comma2){
                    E.emplace_back(x,y);
                }
                // get last entry
                while (edgeStr >> openAngle >> x >> comma3 >> y >> closeAngle){
                    E.emplace_back(x,y);
                }
                {
                    std::unique_lock<std::mutex> lock(mtx);
                     inputs_ready = true;
                }
                // notify all threads when daya is ready
                cv.notify_all();
                // run threads
                std::thread thread1(cnfSatThread);
                std::thread thread2(approx1Thread);
                std::thread thread3(approx2Thread);
                // join threads after complete
                thread1.join();
                thread2.join();
                thread3.join();
                //if minimum vertex cover exists, print the vertices
                if (result_cnf.first > 0){
                    std::cout << "CNF-SAT-VC: ";
                    for (int i = 0; i < result_cnf.first; i++) {
                        std::cout << result_cnf.second[i];
                        if (i < result_cnf.first - 1){
                            std::cout << ",";
                        }
                    }
                std::cout << std::endl;
                }
                if (result_a1.first > 0){
                    std::cout << "APROX-VC-1: ";
                    for (int i = 0; i < result_a1.first; i++){
                        std::cout << result_a1.second[i];
                        if (i < result_a1.first - 1){
                            std::cout << ",";
                        }
                    }
                    std::cout << std::endl;
                }
                if (result_a2.first > 0){
                    std::cout << "APPROX-VC-2: ";
                    for (int i = 0; i < result_a2.first; i++){
                        std::cout << result_a2.second[i];
                        if (i < result_a2.first - 1){
                            std::cout << ",";
                        }
                    }
                    std::cout << std::endl;
                }
                // get cover ratio
                double approx1_ratio = static_cast<double>(result_a1.first) / result_cnf.first;
                double approx2_ratio = static_cast<double>(result_a2.first) / result_cnf.first;

                output << V << ", " 
                       << approx1_ratio << ", " << approx2_ratio << ", " 
                       << cnfThreadTime.count()/1000.0 << ", " << approx1ThreadTime.count()/1000.0 << ", " << approx2ThreadTime.count()/1000.0 
                       << std::endl;

                // clear memory
                V = 0;
                E.clear();
                result_cnf.second.clear();
                result_a1.second.clear();
                result_a2.second.clear();

            }
        }
        output.close();
        {
            std::unique_lock<std::mutex> lock(mtx);
            should_exit.store(true);
            inputs_ready = true;
        }
        cv.notify_all();
    });
    ioThread.join();
    return 0;
}
