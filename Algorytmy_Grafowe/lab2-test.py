from test import run_tests_in_directory
from dimacs import *
from lab2.Ford import fordFulkerson

run_tests_in_directory("lab2/flow", loadDirectedWeightedGraph, fordFulkerson)
