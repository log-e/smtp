cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(smtp VERSION 0.1.0 LANGUAGES CXX)
add_executable(mailbox main.cpp smtp.cpp)