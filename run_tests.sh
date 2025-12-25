#!/bin/bash

# CPP Python Parser - Test Runner
# Runs all test files and reports results

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

echo "=========================================="
echo "  CPP Python Parser - Test Suite"
echo "=========================================="
echo ""

# Check if parser exists
if [ ! -f "./cpp_parser" ]; then
    echo -e "${RED}Error: cpp_parser not found. Please build first:${NC}"
    echo "  g++ -std=c++20 -O2 -o cpp_parser main.cpp"
    exit 1
fi

# Function to run a single test
run_test() {
    local test_file=$1
    local test_name=$(basename "$test_file")
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    echo -n "Testing $test_name... "
    
    if ./cpp_parser "$test_file" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ PASS${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

# Find and run all test files
echo -e "${BLUE}Running test suite...${NC}"
echo ""

# Test categories
declare -a CATEGORIES=(
    "tests/expressions"
    "tests/statements"
    "tests/classes"
    "tests/comprehensive"
    "tests/operators"
    "tests/control_flow"
    "tests/async_await"
    "tests/async_comprehensions"
    "tests/augmented_assignment"
    "tests/multi_param_subscripts"
    "tests/imports"
    "tests/set_literals"
    "tests/try_except"
    "tests/union_types"
    "tests/walrus_operator"
    "tests/type_annotations"
)

for category in "${CATEGORIES[@]}"; do
    if [ -d "$category" ]; then
        echo -e "${YELLOW}Category: $category${NC}"
        
        # Find all .py files in category
        while IFS= read -r test_file; do
            run_test "$test_file"
        done < <(find "$category" -name "*.py" -type f | sort)
        
        echo ""
    fi
done

# Run any top-level test files
if ls tests/*.py 1> /dev/null 2>&1; then
    echo -e "${YELLOW}Category: tests (root)${NC}"
    for test_file in tests/*.py; do
        run_test "$test_file"
    done
    echo ""
fi

# Summary
echo "=========================================="
echo "  Test Results Summary"
echo "=========================================="
echo -e "Total tests:  ${BLUE}$TOTAL_TESTS${NC}"
echo -e "Passed:       ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed:       ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✓ All tests passed!${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}✗ Some tests failed.${NC}"
    exit 1
fi
