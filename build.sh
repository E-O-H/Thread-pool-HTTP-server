#!/bin/sh

g++ -std=c++0x -pthread threadSafeKVStore.hpp threadSafeKVStore.cpp threadPoolServer.hpp threadPoolServer.cpp threadSafeQueue.hpp httpProcessingFunc.hpp httpProcessingFunc.cpp requestHandler.hpp requestHandler.cpp fileSystemIO.hpp fileSystemIO.cpp main.cpp -o runme
