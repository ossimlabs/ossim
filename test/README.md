# Testing

Currently all testing in OSSIM is done via the ossim-batch-test executable with configuration files as input. The `config` subdirectory contains the keyword lists that define each test. See the [readme](config/README.md) file for more information.

The src directory contains individual standalone test executables that serve as unit and functional tests for various components of OSSIM core. The directory heirarchy parallels that of ossim/src. Any new tests should be located in the subdirectory that reflects the highest level class being tested.
