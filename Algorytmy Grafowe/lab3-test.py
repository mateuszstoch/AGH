from lab3.FordConn import findGraphConnectivity
from dimacs import *
from test import run_tests_in_directory

run_tests_in_directory("lab3/connectivity",
                       loadWeightedGraph, findGraphConnectivity)
