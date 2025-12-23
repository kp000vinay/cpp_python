#!/bin/bash
# Run all tests and report results

cd "$(dirname "$0")/.."
EXEC="./cpp_project.exe"
PASSED=0
FAILED=0
TOTAL=0

echo "=== Running Parser Tests ==="
echo ""

# Run tests in main directory
for test_file in tests/test_*.py; do
    if [ ! -f "$test_file" ]; then
        continue
    fi

    test_name=$(basename "$test_file" .py)
    TOTAL=$((TOTAL + 1))

    echo -n "Testing $test_name... "

    if $EXEC "$test_file" > /dev/null 2>&1; then
        echo "✓ PASSED"
        PASSED=$((PASSED + 1))
    else
        echo "✗ FAILED"
        FAILED=$((FAILED + 1))
        echo "  Error output:"
        $EXEC "$test_file" 2>&1 | head -3 | sed 's/^/    /'
    fi
done

# Run tests in feature directories
for feature_dir in tests/*/; do
    if [ ! -d "$feature_dir" ] || [ "$feature_dir" = "tests/tests/" ]; then
        continue
    fi

    feature_name=$(basename "$feature_dir")
    echo ""
    echo "=== Testing $feature_name ==="

    for test_file in "$feature_dir"test_*.py; do
        if [ ! -f "$test_file" ]; then
            continue
        fi

        test_name=$(basename "$test_file" .py)
        TOTAL=$((TOTAL + 1))

        echo -n "Testing $feature_name/$test_name... "

        if $EXEC "$test_file" > /dev/null 2>&1; then
            echo "✓ PASSED"
            PASSED=$((PASSED + 1))
        else
            echo "✗ FAILED"
            FAILED=$((FAILED + 1))
            echo "  Error output:"
            $EXEC "$test_file" 2>&1 | head -3 | sed 's/^/    /'
        fi
    done
done

echo ""
echo "=== Test Summary ==="
echo "Total: $TOTAL"
echo "Passed: $PASSED"
echo "Failed: $FAILED"

if [ $FAILED -eq 0 ]; then
    echo ""
    echo "All tests passed! ✓"
    exit 0
else
    echo ""
    echo "Some tests failed. ✗"
    exit 1
fi
