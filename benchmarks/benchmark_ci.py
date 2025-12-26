#!/usr/bin/env python3
"""
Performance benchmark script for CI/CD pipelines.
Compares cpp_python parser performance against CPython's ast.parse().

Usage:
    python benchmark_ci.py --output results.json
    python benchmark_ci.py --output results.json --parser ./cpp_parser.exe
"""

import argparse
import ast
import json
import os
import platform
import subprocess
import sys
import tempfile
import time
from datetime import datetime
from statistics import mean, stdev

# Test cases with varying complexity
TEST_CASES = {
    "simple_expression": "x = 1 + 2 * 3",
    
    "function_definition": '''
def calculate(a, b, c):
    """Calculate a complex expression."""
    result = a * b + c
    if result > 100:
        return result - 50
    return result
''',
    
    "class_definition": '''
class MyClass:
    """A sample class."""
    
    def __init__(self, value):
        self.value = value
    
    def get_value(self):
        return self.value
    
    def set_value(self, new_value):
        self.value = new_value
''',
    
    "control_flow": '''
def process_items(items):
    results = []
    for item in items:
        if item > 0:
            results.append(item * 2)
        elif item < 0:
            results.append(item * -1)
        else:
            results.append(0)
    return results
''',
    
    "list_comprehension": '''
squares = [x**2 for x in range(100)]
evens = [x for x in range(100) if x % 2 == 0]
matrix = [[i*j for j in range(10)] for i in range(10)]
''',
    
    "exception_handling": '''
def safe_divide(a, b):
    try:
        result = a / b
    except ZeroDivisionError:
        return None
    except TypeError as e:
        raise ValueError(f"Invalid types: {e}")
    else:
        return result
    finally:
        print("Division attempted")
''',
    
    "decorators": '''
def my_decorator(func):
    def wrapper(arg):
        print("Before")
        result = func(arg)
        print("After")
        return result
    return wrapper

@my_decorator
def greet(name):
    return f"Hello, {name}!"
''',
    
    "async_code": '''
async def fetch_data(url):
    async with aiohttp.ClientSession() as session:
        async for chunk in session.get(url):
            yield chunk

async def main():
    data = await fetch_data("https://example.com")
    return data
''',
}


def generate_large_file(num_functions=100):
    """Generate a large Python file for stress testing."""
    lines = ['"""Auto-generated large Python file for benchmarking."""', '']
    
    for i in range(num_functions):
        lines.extend([
            f'def function_{i}(a, b, c):',
            f'    """Function number {i}."""',
            f'    result = a + b * c',
            f'    if result > {i}:',
            f'        return result - {i}',
            f'    elif result < -{i}:',
            f'        return result + {i}',
            f'    else:',
            f'        return result',
            '',
        ])
    
    # Add a class
    lines.extend([
        'class BenchmarkClass:',
        '    """A class with many methods."""',
        '',
    ])
    
    for i in range(20):
        lines.extend([
            f'    def method_{i}(self, x):',
            f'        """Method number {i}."""',
            f'        return x * {i + 1}',
            '',
        ])
    
    return '\n'.join(lines)


def benchmark_cpython(code: str, iterations: int = 100) -> list:
    """Benchmark CPython's ast.parse() function."""
    times = []
    for _ in range(iterations):
        start = time.perf_counter()
        ast.parse(code)
        end = time.perf_counter()
        times.append((end - start) * 1000)  # Convert to milliseconds
    return times


def benchmark_cpp_parser(code: str, parser_path: str, iterations: int = 100) -> list:
    """Benchmark the cpp_python parser."""
    times = []
    
    # Write code to a temporary file
    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
        f.write(code)
        temp_path = f.name
    
    try:
        for _ in range(iterations):
            start = time.perf_counter()
            result = subprocess.run(
                [parser_path, temp_path],
                capture_output=True,
                text=True
            )
            end = time.perf_counter()
            
            if result.returncode != 0:
                # Parser failed - return None to indicate failure
                return None
            
            times.append((end - start) * 1000)  # Convert to milliseconds
    finally:
        os.unlink(temp_path)
    
    return times


def run_benchmarks(parser_path: str, iterations: int = 50) -> dict:
    """Run all benchmarks and return results."""
    results = {
        "platform": platform.platform(),
        "python_version": platform.python_version(),
        "timestamp": datetime.utcnow().isoformat(),
        "iterations": iterations,
        "tests": [],
        "summary": {}
    }
    
    # Add large file test case
    test_cases = dict(TEST_CASES)
    test_cases["large_file"] = generate_large_file(50)
    
    ratios = []
    
    for name, code in test_cases.items():
        print(f"Benchmarking: {name}...", end=" ", flush=True)
        
        # Benchmark CPython
        cpython_times = benchmark_cpython(code, iterations)
        cpython_mean = mean(cpython_times)
        cpython_std = stdev(cpython_times) if len(cpython_times) > 1 else 0
        
        # Benchmark cpp_parser
        cpp_times = benchmark_cpp_parser(code, parser_path, iterations)
        
        if cpp_times is None:
            print("SKIP (parser failed)")
            continue
        
        cpp_mean = mean(cpp_times)
        cpp_std = stdev(cpp_times) if len(cpp_times) > 1 else 0
        
        ratio = cpp_mean / cpython_mean if cpython_mean > 0 else float('inf')
        ratios.append(ratio)
        
        test_result = {
            "name": name,
            "cpython_mean_ms": cpython_mean,
            "cpython_std_ms": cpython_std,
            "cpp_mean_ms": cpp_mean,
            "cpp_std_ms": cpp_std,
            "ratio": ratio,
            "code_lines": len(code.strip().split('\n'))
        }
        
        results["tests"].append(test_result)
        print(f"done (ratio: {ratio:.2f}x)")
    
    # Calculate summary
    if ratios:
        avg_ratio = mean(ratios)
        if avg_ratio < 1:
            comparison = f"{1/avg_ratio:.2f}x faster than CPython"
        else:
            comparison = f"{avg_ratio:.2f}x slower than CPython"
        
        results["summary"] = {
            "average_ratio": avg_ratio,
            "min_ratio": min(ratios),
            "max_ratio": max(ratios),
            "comparison": comparison
        }
    
    return results


def main():
    parser = argparse.ArgumentParser(description="Benchmark cpp_python parser vs CPython")
    parser.add_argument("--output", "-o", default="benchmark_results.json",
                        help="Output JSON file path")
    parser.add_argument("--parser", "-p", default="./cpp_parser",
                        help="Path to cpp_parser executable")
    parser.add_argument("--iterations", "-i", type=int, default=50,
                        help="Number of iterations per test")
    
    args = parser.parse_args()
    
    # Check if parser exists
    if not os.path.exists(args.parser):
        print(f"Error: Parser not found at {args.parser}")
        sys.exit(1)
    
    print(f"Running benchmarks with {args.iterations} iterations...")
    print(f"Parser: {args.parser}")
    print(f"Platform: {platform.platform()}")
    print(f"Python: {platform.python_version()}")
    print()
    
    results = run_benchmarks(args.parser, args.iterations)
    
    # Write results to JSON
    with open(args.output, 'w') as f:
        json.dump(results, f, indent=2)
    
    print()
    print(f"Results written to: {args.output}")
    print(f"Summary: cpp_parser is {results['summary'].get('comparison', 'N/A')}")


if __name__ == "__main__":
    main()
