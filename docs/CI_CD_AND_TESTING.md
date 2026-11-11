# CI/CD and Testing Implementation

## 1. Application and deployment model

The Railway Reservation System is a standalone C++17 console application using local file storage and simulated payment processing. The architecture specification does not describe a network-hosted service. Accordingly, this pipeline builds, tests, packages, and publishes downloadable application bundles; it does not deploy a server.

## 2. CI/CD workflow

Workflow: `.github/workflows/cpp-build.yml`

Triggers:
- Pull requests targeting `main`
- Pushes to `main`
- Version tags matching `v*`
- Manual workflow dispatch

Jobs:
1. **Build, test, and package:** Ubuntu and Windows build matrix; CMake configuration; application and test compilation; CTest execution; platform ZIP packaging and artifact upload.
2. **Static analysis and coverage:** Cppcheck; GCC coverage-instrumented build; CTest; gcovr text, XML, and HTML coverage reports.
3. **Release:** On a version tag only, after both build/test and quality jobs pass, publish the Linux and Windows packages as GitHub Release assets.

## 3. Tool substitutions from the Software Test Plan

The submitted Test Plan names Jenkins, GoogleTest, and SonarQube (or equivalent). The current repository pipeline uses:
- **GitHub Actions** for CI orchestration instead of Jenkins.
- **CMake + CTest and repository test executables** for automated tests. The existing test sources use custom test harnesses; this must not be described as GoogleTest unless the test framework is actually migrated.
- **Cppcheck** for static analysis. It provides useful static checks but is not a full SonarQube replacement or quality-gate dashboard.
- **gcovr** over GCC coverage instrumentation for line and branch coverage reports.

These are implementation choices that should be reflected in the final SRS/Test Plan revision and accepted by the project evaluator. Do not claim Jenkins, GoogleTest, or SonarQube ran in this workflow.

## 4. Automated test suites

CMake registers the project test executables with CTest. The two administrator security test sources are included in addition to the previously registered suites:
- `AdminAuthManagerTests`
- `AdminTrainMenuSecurityTests`

The exact number of passing suites and the coverage percentages must be taken from the latest completed workflow run, not copied from an earlier run.

## 5. Reports

The quality job generates and uploads:
- Unit-test execution output
- System-validation scope/limitations note
- gcovr coverage summary
- XML coverage data
- HTML coverage report
- Code-coverage and branch-coverage summaries

Reports are retained as workflow artifacts. The coverage target in the Test Plan is at least 70% branch coverage. A report below 70% is a measured gap, not a pass.

## 6. Release limitations

The release job is tag-triggered and depends on the build/test and quality jobs. Until a version-tag workflow completes successfully, release publication is configured but not verified. No deployment to a production server is applicable to the current offline console architecture.

## 7. Evidence links

- Repository: https://github.com/Railway-Reservation-Team/Railway-Reservation-Team
- CI/CD workflow: https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/actions/workflows/cpp-build.yml
- Pull request: https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/pull/79
