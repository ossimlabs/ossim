# OSSIM Batch Test Scripts

The *.kwl files in this directory are configuration scripts for the ossim-batch-test executable. The format for the command line is
```
ossim-batch-test <test-configuration-file.kwl>
```
There are two required environment variables referenced by ossim-batch-test and associated config scripts, namely, 

`OSSIM_BATCH_TEST_DATA` -- top-level (read-only) directory containing all source test data used by batch tests, and 

`OSSIM_BATCH_TEST_RESULTS` -- location (read-write) where expected results, actual results, and log files are written.

These env vars must be set before running ossim-batch-test. There is a collection of public data (including expected results) available for download at [TBD]
