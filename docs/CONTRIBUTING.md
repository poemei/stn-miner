# Contributing to STN Miner

Thank you for your interest in contributing to STN Miner.

STN Miner is developed under a simple rule:

> Small. Deterministic. Easy to use.

Contributions are welcome, but all development must preserve the architecture, protocol behavior, consensus compatibility, and platform independence of the STN Chain ecosystem.

---

## Requirements

Before contributing, you must:

- Have a GitHub account.
- Know how to fork a GitHub repository.
- Know how to create and maintain a development branch in your fork.
- Know how to submit a GitHub Pull Request.
- Be able to build and test the code you are changing.

STN Miner is not developed through direct contributor access to the root repository.

---

## No Direct Commit Access

No contributor will receive direct commit access to the root STN Miner repository.

This includes:

- external contributors;
- occasional contributors;
- regular contributors;
- platform maintainers;
- hardware-backend contributors.

All proposed changes must originate from a fork and enter the project through a Pull Request.

The root repository remains under project-maintainer control.

Maintainers are responsible for reviewing and merging accepted contributions.

---

## Contribution Workflow

The required contribution workflow is:

    STN Miner root repository
        |
        v
    Fork repository
        |
        v
    Create development branch
        |
        v
    Make changes
        |
        v
    Build and test
        |
        v
    Push branch to your fork
        |
        v
    Open Pull Request
        |
        v
    Project review
        |
        v
    Accept, request changes, or reject

Do not develop directly against the root repository.

---

## Fork the Repository

Create your own fork of the STN Miner repository through GitHub.

Clone your fork locally.

Example:

    git clone https://github.com/YOUR-ACCOUNT/stn-miner.git

Enter the repository:

    cd stn-miner

Create a branch for your work:

    git checkout -b your-change-name

Do not perform contribution work directly on your fork's default branch unless there is a specific reason to do so.

---

## Keep Changes Focused

Pull Requests should address a clear and bounded purpose.

Examples:

- Linux platform support;
- ARM qualification;
- GPU backend development;
- USB-ASIC support;
- dedicated ASIC support;
- protocol corrections;
- documentation;
- build-system changes;
- bug fixes;
- logging improvements;
- tests.

Avoid combining unrelated changes into one Pull Request.

A smaller change is easier to review, test, qualify, and understand.

---

## Platform Independence

STN Miner is platform-agnostic by design.

Operating-system-specific behavior belongs behind the platform abstraction.

Current platform direction includes:

    - Windows
    - Linux
	- macOS
    - future STN Chain OS

Current architecture direction includes:

    x86_64
    ARM32
    ARM64

Hashing systems include:

    CPU
    GPU
    USB-ASIC
    ASIC

A platform-specific implementation must not change consensus-visible mining behavior.

The same work must remain the same work regardless of the machine performing it.

---

## Common Code and Platform Code

Common miner behavior belongs in the shared source tree.

Examples include:

    src/stn_miner.c
    src/stn_protocol.c
    src/stn_hash.c
    src/stn_cpu.c
    src/stn_config.c
    src/stn_display.c
    src/stn_log.c

Operating-system-specific behavior belongs under:

    platforms/

For example:

    platforms/windows/
    platforms/linux/
	platforms/mackos/

Platform implementations may provide services such as:

- networking;
- timing;
- console behavior;
- logging;
- device access;
- operating-system integration.

They must not redefine STN mining or consensus behavior.

---

## Mining Backend Contributions

Mining backends may support:

- CPU;
- GPU;
- USB-ASIC;
- ASIC.

A hashing backend is not permitted to redefine the STN Chain proof-of-work contract.

Hardware differences affect performance.

They do not affect validity.

A valid result must remain valid under the same STN Chain rules regardless of the hashing device that produced it.

---

## Protocol Compatibility

STN Miner communicates with STN-Stratum using STNM.

Current message assignments include:

    Type 1    JOB
    Type 2    SUBMIT
    Type 3    RESULT
    Type 4    HASH_PROGRESS
    Type 5    ADDRESS

Existing message assignments must not be silently renumbered or repurposed.

Protocol changes require explicit review because they can affect:

- STN Miner;
- STN-Stratum;
- STN Chain integration;
- accounting;
- deployed miners;
- future hardware implementations.

Do not introduce incompatible protocol behavior in a normal feature Pull Request.

---

## Consensus

STN Chain remains authoritative for determining whether submitted work is valid.

The miner:

- receives work;
- hashes work;
- reports progress;
- submits candidate proof.

STN-Stratum:

- coordinates miners;
- associates sessions with STN Chain addresses;
- measures mining participation;
- forwards solved work.

STN Chain:

- independently validates proof;
- determines acceptance.

Contributions must preserve these responsibility boundaries.

---

## Build and Test

Contributors are expected to build and test their changes before submitting a Pull Request.

### Windows

Windows development uses:

    build.cmd

The current Windows compiler path uses Microsoft `cl`.

### MacOS
a MacOS miner needs to use:

    build.pkg
	
The Chief Developer has no experience with MacOS, so if this is for you, Fork, build, test, create  pull request with an in depth description.

### Linux

Linux development uses:

    Makefile

Typical build:

    make

Cleanup:

    make clean

Do not claim a platform is supported or qualified merely because the code is expected to compile there.

Supported behavior must be tested.

Probable is not determinate.

---

## Pull Requests

Every contribution must be submitted as a Pull Request from the contributor's fork.

A Pull Request should clearly explain:

- what changed;
- why it changed;
- which files or subsystem are affected;
- which platform or hashing system is affected;
- how the change was tested;
- whether protocol behavior changed;
- whether consensus-visible behavior changed.

If the change has not been tested, say so explicitly.

Do not represent an untested implementation as qualified.

---

## Review

Submission of a Pull Request does not guarantee acceptance.

A Pull Request may be:

- accepted;
- returned for changes;
- superseded;
- deferred;
- rejected.

Project maintainers may require additional testing or qualification before merge.

Changes affecting protocol, Chain behavior, Stratum behavior, mining semantics, accounting, or other project-wide behavior may require additional project review before they are eligible for merge.

---

## Documentation

Changes that alter user-visible behavior, configuration, protocols, platform support, or build procedures should include corresponding documentation updates.

Relevant documentation may include:

    README.md
    CHANGELOG.md
    CONTRIB.md

Documentation must describe implemented behavior, not intended or hypothetical behavior.

---

## Security

Do not submit:

- credentials;
- private keys;
- API secrets;
- passwords;
- private infrastructure information;
- personal access tokens.

If a security issue should not be publicly disclosed, do not publish exploit details in a public Pull Request before coordinating with project maintainers.

---

## Development Doctrine

STN Miner follows the broader STN Chain engineering model:

    Small.
    Deterministic.
    Easy to use.

Additional working principles include:

    Probable != Determinate.

and:

    Platform-specific implementation
    must not create
    platform-specific consensus.

Contributors should prefer:

- clear code;
- bounded behavior;
- explicit interfaces;
- deterministic results;
- minimal dependencies;
- straightforward operation.

Avoid complexity that does not solve an established requirement.

---

## Final Rule

If you want to contribute:

    Fork it.
    Build it.
    Test it.
    Pull Request it.

No contributor receives direct commit access to the root repository.