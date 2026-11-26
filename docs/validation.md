# Engineering Validation Report

## 1. Validation Overview

This report documents the software verification and
validation activities performed for the reconstructed
Multitasking Trolley for Visually Impaired People firmware.

The validation scope includes Arduino firmware compilation,
memory utilization, independent sensor-processing algorithms,
boundary condition testing, invalid measurement handling,
and continuous integration.

The original physical prototype and the reconstructed
firmware are treated as separate engineering artifacts.

The original prototype photograph and video document
the earlier physical assembly. They do not establish
hardware performance of the reconstructed firmware.

## 2. Validation Objectives

- Verify Arduino Uno firmware compilation.
- Evaluate program storage and static SRAM utilization.
- Verify ultrasonic distance calculations.
- Verify obstacle classification thresholds.
- Verify supplementary IR detection logic.
- Verify load-cell conversion calculations.
- Verify load classification.
- Verify measurement filtering.
- Verify load-change detection.
- Verify timer rollover handling.
- Evaluate automated build reproducibility.
- Identify limitations requiring physical hardware testing.

## 3. Test Environment

| Parameter | Configuration |
|---|---|
| Local operating system | Windows |
| Embedded development environment | Arduino IDE |
| Reference target | Arduino Uno R3 |
| Arduino AVR core | 1.8.8 |
| Load-cell library | HX711 by bogde |
| Desktop test environment | WSL Ubuntu |
| C++ compiler | G++ 15.2.0 |
| C++ standard | C++11 |
| Testing method | C++ assertions |
| Continuous integration | GitHub Actions |
| CI environment | Ubuntu |

Desktop testing uses synthetic inputs rather than
physical sensor measurements.

## 4. Arduino Firmware Compilation

The reconstructed firmware was successfully compiled
for Arduino Uno using Arduino IDE.

### Compilation Results

| Memory Resource | Used | Available | Usage | Result |
|---|---:|---:|---:|---|
| Program storage | 7748 bytes | 32256 bytes | 24% | PASS |
| Static SRAM | 309 bytes | 2048 bytes | 15% | PASS |

### Remaining Memory Capacity

| Resource | Remaining |
|---|---:|
| Program storage | 24508 bytes |
| SRAM available for local variables and runtime use | 1739 bytes |

The compilation results demonstrate that the firmware
fits within the available program storage and static
memory limits of the selected Arduino target.

Runtime stack usage, dynamic memory consumption, and
worst-case memory utilization have not been measured.

### Compilation Evidence

![Arduino Uno Compilation Results](../images/arduino_compilaton_results.png)

## 5. Development Troubleshooting

During local compilation, two development environment
issues were encountered.

| Issue | Resolution |
|---|---|
| sensor_logic.h not found | Placed the sketch and header in the same permanent project directory |
| Undefined references to setup and loop | Opened and compiled the complete Arduino sketch |

Following correction of the local sketch configuration,
the firmware compiled successfully.

These issues were related to the local development
workflow and sketch configuration.

## 6. Automated C++ Regression Testing

The independent sensor logic is implemented in:

[Sensor Logic Header](../firmware/assistive_trolley/sensor_logic.h)

The automated regression test program is available at:

[C++ Test Program](../tests/test_sensor_logic.cpp)

The test program evaluates portable C++ functions
without requiring an Arduino or connected sensors.

### Compilation Command

Run from the repository root:

```bash
g++ -std=c++11 -Wall -Wextra -Werror tests/test_sensor_logic.cpp -o test_sensor_logic
```

### Execution Command

```bash
./test_sensor_logic
```

### Compiler Configuration

| Option | Purpose |
|---|---|
| -std=c++11 | Compile using the C++11 standard |
| -Wall | Enable common compiler warnings |
| -Wextra | Enable additional compiler warnings |
| -Werror | Treat compiler warnings as errors |
| -o | Specify the executable filename |

### Observed Test Output

```text
28 sensor logic assertions passed
```

### Automated Test Results

| Metric | Result |
|---|---:|
| Total assertions | 28 |
| Passed assertions | 28 |
| Failed assertions | 0 |
| Assertion pass rate | 100% |

### Test Execution Evidence

![C++ Regression Test Results](../images/cpp_test_results.png)

## 7. Detailed Automated Test Results

The following table documents the assertions contained
in the committed C++ regression test program.

### Ultrasonic Distance Calculation

| Test ID | Input | Expected Result | Status |
|---|---|---|---|
| TC-01 | Echo duration = 0 us | Invalid distance, NaN | PASS |
| TC-02 | Echo duration = 1000 us | 17.15 cm within tolerance | PASS |

### Obstacle and IR Classification

| Test ID | Distance | IR Detection | Expected Result | Status |
|---|---|---|---|---|
| TC-03 | 20 cm | False | Critical | PASS |
| TC-04 | 20.01 cm | False | Warning | PASS |
| TC-05 | 50 cm | False | Warning | PASS |
| TC-06 | 50.01 cm | False | Clear | PASS |
| TC-07 | NaN | False | Fault | PASS |
| TC-08 | 1 cm | False | Fault | PASS |
| TC-09 | 401 cm | False | Fault | PASS |
| TC-10 | 100 cm | True | Warning | PASS |
| TC-11 | NaN | True | Warning | PASS |
| TC-12 | 10 cm | True | Critical | PASS |

The proximity classification function returns Warning
when IR detection is active and the ultrasonic distance
is invalid.

The complete Arduino firmware separately evaluates
the ultrasonic range-fault flag.

Consequently, the full firmware is designed to retain
the fault indication in this situation.

The combined hardware-level alert behavior is not
directly exercised by TC-11.

### Load-Cell Conversion

| Test ID | Raw Reading | Zero Reference | Counts/kg | Expected Result | Status |
|---|---:|---:|---:|---|---|
| TC-13 | 1500 | 1000 | 500 | 1 kg | PASS |
| TC-14 | 500 | 1000 | -500 | 1 kg | PASS |
| TC-15 | 10 | 0 | 0 | NaN | PASS |

### Load Classification

| Test ID | Measured Mass | Load Limit | Expected Result | Status |
|---|---|---|---|---|
| TC-16 | 2 kg | 2 kg | Overload | PASS |
| TC-17 | 1.99 kg | 2 kg | Normal | PASS |
| TC-18 | NaN | 2 kg | Fault | PASS |
| TC-19 | -1 kg | 2 kg | Fault | PASS |

### Measurement Filtering

These assertions use a sequential filter instance
with a smoothing coefficient of 0.25.

| Test ID | Input | Expected Result | Status |
|---|---|---|---|
| TC-20 | Initial input of 4 kg | 4 kg | PASS |
| TC-21 | Subsequent input of 8 kg | 5 kg | PASS |
| TC-22 | Invalid input, NaN | NaN and filter reset | PASS |
| TC-23 | First valid input after reset, 2 kg | 2 kg | PASS |

### Load-Change Detection

| Test ID | First Value | Second Value | Expected Result | Status |
|---|---|---|---|---|
| TC-24 | 0 kg | 0.2 kg | Change detected | PASS |
| TC-25 | 0 kg | 0.19 kg | No change detected | PASS |
| TC-26 | NaN | 1 kg | No valid change detected | PASS |

### Timer Rollover Handling

The timer tests use unsigned 32-bit arithmetic
across the maximum counter value.

| Test ID | Current Counter | Previous Counter | Interval | Expected Result | Status |
|---|---:|---|---:|---|---|
| TC-27 | 5 | UINT32_MAX - 4 | 10 ms | Elapsed | PASS |
| TC-28 | 5 | UINT32_MAX - 4 | 11 ms | Not elapsed | PASS |

These assertions verify that the elapsed-time
calculation works for the tested rollover scenario.

## 8. Test Coverage Summary

| Functional Area | Assertions | Passed |
|---|---:|---:|
| Ultrasonic distance calculation | 2 | 2 |
| Obstacle and IR classification | 10 | 10 |
| Load-cell conversion | 3 | 3 |
| Load classification | 4 | 4 |
| Measurement filtering | 4 | 4 |
| Load-change detection | 3 | 3 |
| Timer rollover handling | 2 | 2 |
| Total | 28 | 28 |

All 28 defined assertions passed during local testing.

The assertion pass rate does not represent complete
source code, branch, or requirements coverage.

## 9. Boundary Condition Verification

Boundary condition testing evaluates measurements
at or immediately above selected software thresholds.

| Parameter | Tested Value | Expected Behavior | Result |
|---|---|---|---|
| Critical obstacle threshold | 20 cm | Critical | PASS |
| Above critical threshold | 20.01 cm | Warning | PASS |
| Warning obstacle threshold | 50 cm | Warning | PASS |
| Above warning threshold | 50.01 cm | Clear | PASS |
| Overload threshold | 2 kg | Overload | PASS |
| Below overload threshold | 1.99 kg | Normal | PASS |
| Load-change threshold | 0.2 kg | Change detected | PASS |
| Below load-change threshold | 0.19 kg | No change detected | PASS |

These are synthetic software boundary tests.

The configured thresholds are illustrative and
have not been established as safe physical
operating limits.

## 10. Invalid Input and Fault Verification

| Fault Scenario | Tested Behavior | Result |
|---|---|---|
| Zero echo duration | Invalid distance generated | PASS |
| Non-numeric distance | Proximity fault without IR detection | PASS |
| Distance below accepted range | Proximity fault | PASS |
| Distance above accepted range | Proximity fault | PASS |
| Zero calibration coefficient | Invalid weight generated | PASS |
| Invalid weight | Load fault | PASS |
| Excessively negative weight | Load fault | PASS |
| Invalid filter input | Filter reset | PASS |
| Invalid load-change input | Change not reported | PASS |

These tests evaluate independent algorithm responses
to invalid or unavailable measurements.

They do not constitute physical sensor disconnection
or hardware fault-injection testing.

## 11. Continuous Integration

The repository includes an automated GitHub Actions
workflow for software verification.

[![Assistive Trolley Firmware Verification](https://github.com/ShanmukhJonnalagadda/MULTITASKING-TROLLEY-FOR-VISUALLY-IMPAIRED-PEOPLE/actions/workflows/tests.yml/badge.svg)](https://github.com/ShanmukhJonnalagadda/MULTITASKING-TROLLEY-FOR-VISUALLY-IMPAIRED-PEOPLE/actions/workflows/tests.yml)

The workflow contains two independent jobs.

### Job 1: C++ Regression Tests

- Check out the repository.
- Display the compiler version.
- Compile the sensor logic test program.
- Treat compiler warnings as errors.
- Execute the automated assertions.

### Job 2: Arduino Uno Firmware Compilation

- Check out the repository.
- Configure Arduino CLI.
- Install Arduino AVR core version 1.8.8.
- Install the HX711 library.
- Compile the complete Arduino Uno firmware.

### Verified CI Results

| CI Job | Result |
|---|---|
| C++ Regression Tests | PASS |
| Arduino Uno Firmware Compilation | PASS |

Both jobs completed successfully in a verified
GitHub Actions workflow.

[View Verified Successful CI Run](https://github.com/ShanmukhJonnalagadda/MULTITASKING-TROLLEY-FOR-VISUALLY-IMPAIRED-PEOPLE/actions/runs/45758027615)

[View Latest GitHub Actions Results](https://github.com/ShanmukhJonnalagadda/MULTITASKING-TROLLEY-FOR-VISUALLY-IMPAIRED-PEOPLE/actions)

The workflow executes automatically on repository
pushes and pull requests and also supports
manual execution.

The HX711 library version is not pinned in the
current workflow. Future reproducibility could
be improved by specifying the verified version.

## 12. Requirements Verification Matrix

| Requirement ID | Requirement | Verification Method | Status |
|---|---|---|---|
| REQ-01 | Firmware compiles for Arduino Uno | Local and CI compilation | PASS |
| REQ-02 | Program storage within target capacity | Compilation output | PASS |
| REQ-03 | Static SRAM within target capacity | Compilation output | PASS |
| REQ-04 | Echo duration conversion | Automated assertions | PASS |
| REQ-05 | Obstacle threshold classification | Automated assertions | PASS |
| REQ-06 | Supplementary IR classification | Automated assertions | PASS |
| REQ-07 | Load-cell count conversion | Automated assertions | PASS |
| REQ-08 | Invalid calibration coefficient handling | Automated assertion | PASS |
| REQ-09 | Load condition classification | Automated assertions | PASS |
| REQ-10 | Measurement filtering | Automated assertions | PASS |
| REQ-11 | Load-change detection | Automated assertions | PASS |
| REQ-12 | Timer rollover calculation | Automated assertions | PASS |
| REQ-13 | Repeatable CI build and test execution | GitHub Actions | PASS |
| REQ-14 | Physical ultrasonic sensor operation | Hardware testing required | NOT TESTED |
| REQ-15 | Physical IR sensor operation | Hardware testing required | NOT TESTED |
| REQ-16 | Physical load-cell calibration | Hardware testing required | NOT TESTED |
| REQ-17 | Buzzer alert timing | Hardware or integration testing required | NOT TESTED |
| REQ-18 | LED indication behavior | Hardware or integration testing required | NOT TESTED |
| REQ-19 | Sensor timeout behavior in the complete sketch | Integration testing required | NOT TESTED |
| REQ-20 | Complete system response latency | Hardware testing required | NOT TESTED |
| REQ-21 | Accessibility and usability | Supervised user evaluation required | NOT TESTED |
| REQ-22 | Battery runtime | Hardware testing required | NOT TESTED |

The verification matrix contains 13 passed software
and build requirements and nine untested integration
or hardware requirements.

## 13. Original Physical Prototype Evidence

The original trolley photograph and video were
recovered and preserved in the repository.

### Prototype Photograph

![Original Assistive Trolley](../images/Prototype_front.jpeg)

### Prototype Video

[Watch Original Prototype Walkthrough](../images/Prototype_walkthrough.mp4)

These artifacts document the existence and assembly
of the earlier physical prototype.

They do not demonstrate that the reconstructed
firmware was executed on that prototype.

No quantitative physical performance results are
derived from the photograph or video.

## 14. Proposed Hardware Validation Plan

The following tests are recommended for a future
physical reference implementation.

These tests have not been performed.

| Test ID | Proposed Hardware Test | Measurements |
|---|---|---|
| HW-01 | Ultrasonic reference-distance test | Reference and measured distance |
| HW-02 | Ultrasonic repeatability test | Repeated distance measurements |
| HW-03 | Different obstacle surfaces | Detection reliability |
| HW-04 | IR detection test | Detection state and effective range |
| HW-05 | Load-cell tare test | Zero-load offset |
| HW-06 | Known-mass calibration | Reference and measured mass |
| HW-07 | Load-cell repeatability | Variation across repeated measurements |
| HW-08 | Weight addition and removal | Load-change response |
| HW-09 | Buzzer operation | Alert timing and pattern |
| HW-10 | LED indication | Output state |
| HW-11 | Sensor disconnection | Fault response |
| HW-12 | Integrated operation | System response latency |
| HW-13 | Extended operation | Stability and memory behavior |
| HW-14 | Power consumption | Supply current and energy usage |

Physical testing must use appropriate reference
equipment, verified electrical connections, and
controlled test conditions.

The prototype must not be relied upon as a personal
safety device during testing.

## 15. Proposed Measurement Accuracy Evaluation

### Ultrasonic Distance Error

Absolute distance error:

Absolute error = |Measured distance - Reference distance|

Percentage distance error:

Percentage error = Absolute error / Reference distance x 100

Percentage error requires a nonzero reference distance.

### Load-Cell Measurement Error

Absolute mass error:

Absolute error = |Measured mass - Reference mass|

Percentage mass error:

Percentage error = Absolute error / Reference mass x 100

Percentage error requires a nonzero reference mass.

### Proposed Load-Cell Test Record

| Reference Mass | Measured Mass | Absolute Error | Percentage Error |
|---|---|---|---|
| To be measured | Pending | Pending | Pending |
| To be measured | Pending | Pending | Pending |
| To be measured | Pending | Pending | Pending |

No physical measurement results have been fabricated.

## 16. Validation Limitations

- The original firmware is unavailable.
- The reconstructed firmware has not been physically
  deployed on the original trolley.
- The reference electrical wiring is not physically verified.
- Automated assertions test independent sensor algorithms.
- Complete Arduino loop behavior is not automatically tested.
- Calibration serial commands are not independently tested.
- Full alert-priority behavior is not automatically tested.
- Physical sensor disconnection tests are unavailable.
- Sensor measurement accuracy is unknown.
- Actual response latency has not been measured.
- Runtime memory consumption has not been measured.
- Accessibility and usability have not been evaluated.
- GPS, camera, and biometric integration are outside
  the reconstructed firmware scope.
- The 100% assertion pass rate does not establish
  complete software correctness or physical safety.

This project is an educational assistive technology
prototype rather than a validated personal navigation
or safety device.

## 17. Supporting Documentation

- [System Design Documentation](system_design.md)
- [Hardware Wiring and Configuration](wiring.md)
- [Arduino Firmware](../firmware/assistive_trolley/assistive_trolley.ino)
- [Independent Sensor Logic](../firmware/assistive_trolley/sensor_logic.h)
- [Automated C++ Tests](../tests/test_sensor_logic.cpp)
- [Continuous Integration Workflow](../.github/workflows/tests.yml)

## 18. Conclusion

The reconstructed Multitasking Trolley firmware was
successfully compiled for Arduino Uno.

The firmware used 7748 bytes of program storage
and 309 bytes of static SRAM, remaining within
the available target memory capacity.

All 28 independent C++ regression assertions
passed during local testing.

A completed GitHub Actions workflow successfully
verified the desktop C++ test program and
Arduino Uno firmware compilation.

These results demonstrate the tested software
functionality, build compatibility, and repeatable
engineering verification process.

Physical sensor performance, complete hardware
integration, accessibility, and personal safety
remain subject to future validation.
