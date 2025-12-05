import time
import sys
from pathlib import Path
from dimacs import *


class TestFailedError(Exception):
    def __init__(self, message):
        super().__init__(message)
        self.message = message

    def __str__(self):
        return self.message


def process_file(test_data, RESOULT, test_function):
    V, L = test_data
    test_func_output = test_function(V, L)
    if RESOULT != test_func_output:
        raise TestFailedError(
            f"Expected: {RESOULT}, Output: {test_func_output}")
    return


def run_tests_in_directory(directory_path_str: str, load_function, test_function):
    directory_path = Path(directory_path_str)

    if not directory_path.exists():
        print(
            f"Błąd: Ścieżka '{directory_path}' nie istnieje.", file=sys.stderr)
        return
    if not directory_path.is_dir():
        print(
            f"Błąd: Ścieżka '{directory_path}' nie jest katalogiem.", file=sys.stderr)
        return

    files_to_test = [f for f in directory_path.iterdir() if f.is_file()]

    if not files_to_test:
        print("Nie znaleziono plików do testowania.")
        return

    total_start_time = time.perf_counter()
    tests_run = 0
    tests_passed = 0
    tests_skipped = 0
    tests_failed = 0

    for file_path in files_to_test:

        tests_run += 1
        try:
            test_data = load_function(file_path)
            resoult = int(readSolution(file_path))
        except:
            print(
                f"[SKIP]  {file_path.name} : Brak pliku rozwiązania lub błąd odczytu.")
            tests_skipped += 1
            continue
        start_time = time.perf_counter()

        try:
            process_file(test_data, resoult, test_function)

            end_time = time.perf_counter()
            duration = end_time - start_time
            print(f"[OK] {file_path.name} : {duration:.4f}s")
            tests_passed += 1

        except KeyboardInterrupt:
            end_time = time.perf_counter()
            print(
                f"\n[SKIP]  {file_path.name}")
            tests_skipped += 1

        except Exception as e:
            end_time = time.perf_counter()
            duration = end_time - start_time
            print(f"[FAIL]  {file_path.name}")
            print(e)
            tests_failed += 1

    total_end_time = time.perf_counter()
    total_duration = total_end_time - total_start_time
    print("\n" + "=" * 30)
    print(f"Całkowity czas: {total_duration:.4f}s")
    print(f"Tests count:    {tests_run}")
    print(f"[OK]:      {tests_passed}")
    print(f"[SKIP]: {tests_skipped}")
    print(f"[FAILED]:   {tests_failed}")
    print("=" * 30)
