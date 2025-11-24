from lab3.FordConn import findGraphConnectivity
from lab3.StoeraWagnera import stoerWagner
from dimacs import *
from test import run_tests_in_directory

print("Ford")
run_tests_in_directory("lab3/connectivity",
                       loadWeightedGraph, findGraphConnectivity)
print("Wagner")
run_tests_in_directory("lab3/connectivity",
                       loadWeightedGraph, stoerWagner)
