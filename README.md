# suid-wrapper

This utility lets you create an executable binary, with the suid flag set.
The suid flag makes it possible for an user to execute an executable as
if they are the owner of the file.

## Installation

- Option 1: Pre-Compiled binary (< 100 KiB)

    ```bash
    $ wget https://raw.githubusercontent.com/thiagorb/suid-wrapper/releases/suid-wrapper -O suid-wrapper
    $ chmod +x suid-wrapper
    $ sudo mv suid-wrapper /usr/bin/suid-wrapper
    ```

- Option 2: Compile from source

    Clone this repository, then run:
    ```
    $ make release
    $ sudo make install # (copies the compiled binary to /usr/bin)
    ```

## Usage

### Creating binary

    $ sudo suid-wrapper --output root_bash $(which bash) -- -p

  With the command above, a `root_bash` binary will be created, owned by `root`. When any user runs it (because of the suid flag), the process that will be started will have the uid set to `root`, with the same permissions `root` has.

  Running `root_bash` would be the equivalent of running `sudo /bin/bash -p`, except that it doesn't require typing the sudo user password.

### Inspecting created binary

    $ suid-wrapper --inspect root_bash

The inspect option can be used to see what command (and arguments) were used to to create the binary. In this example, read permissions to `root_bash` are required.
