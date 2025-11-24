from test import run_tests_in_directory
from dimacs import *
import sys
from lab1.DFS import widest_path_binary_dfs
from lab1.Dijkstra import widest_path_dijkstra
from lab1.unionFind import widest_path_union_find

sys.setrecursionlimit(12000)

print("DFS")
run_tests_in_directory("lab1/graphs", loadWeightedGraph,
                       widest_path_binary_dfs)
print("Dijkstra")
run_tests_in_directory("lab1/graphs", loadWeightedGraph,
                       widest_path_dijkstra)
print("FindUnion")
run_tests_in_directory("lab1/graphs", loadWeightedGraph,
                       widest_path_union_find)
