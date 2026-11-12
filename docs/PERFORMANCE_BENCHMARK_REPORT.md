
# Train Search Performance Benchmark Report

## Objective

Evaluate train search performance with different dataset sizes.

## Test Configuration

- Search algorithm: Linear search
- Iterations: 100 per dataset
- Dataset sizes: 1,000, 10,000 and 50,000 trains

## Benchmark Results

| Dataset Size | Average Search Time |
|---|---:|
| 1,000 trains | 0.13764 ms |
| 10,000 trains | 0.925086 ms |
| 50,000 trains | 4.31577 ms |

## Observations

- Search time increases as the dataset grows.
- Search performance remains measurable across all three dataset sizes.
- The results provide a baseline for future optimization.

## Conclusion

The benchmark provides performance measurements for train search at different dataset sizes.
