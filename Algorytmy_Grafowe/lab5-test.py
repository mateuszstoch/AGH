from test import run_tests_in_directory
from dimacs import loadWeightedGraph
from lab5.solution import isChordal, maxClique, graphColoring, vertexCover
import sys
import os

# Add parent directory to path to import test.py and dimacs.py
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def run_chordal_tests():
    print("\n--- Chordal Graph Recognition Tests ---")
    # Wrapper to convert boolean result to int (1 or 0) for comparison

    def chordal_wrapper(V, L):
        return 1 if isChordal(L, V) else 0

    run_tests_in_directory("lab5/chordal", loadWeightedGraph, chordal_wrapper)


def run_maxclique_tests():
    print("\n--- Max Clique Tests ---")
    run_tests_in_directory("lab5/maxclique", loadWeightedGraph, maxClique)


def run_coloring_tests():
    print("\n--- Graph Coloring Tests ---")
    run_tests_in_directory("lab5/coloring", loadWeightedGraph, graphColoring)


def run_vcover_tests():
    print("\n--- Vertex Cover Tests ---")
    run_tests_in_directory("lab5/vcover", loadWeightedGraph, vertexCover)


if __name__ == "__main__":
    run_chordal_tests()
    run_maxclique_tests()
    run_coloring_tests()
    run_vcover_tests()
