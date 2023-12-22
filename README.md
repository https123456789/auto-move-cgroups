# Auto Move CGroups

Automatically move newly spawned processes into CGroups.

## About

`auto-move-cgroups` is a daemon process that your system can start which will automatically detect when a new process is spawned and move the new process into a CGroup based upon some provided configuration. The daemon needs to be started as root in order to manipulate CGroups (i.e. move processes into them).

## System Requirements

You need to have [libcgroup](https://github.com/libcgroup/libcgroup) installed on your system.

## Build instructions

You can build the project via `make` and install the binary via `make install`.
