# Changelog

All notable changes to STN Miner will be documented in this file.

---

## [Unreleased]

### Added

- Added portable `config.json` support.
- Added STN Chain address configuration.
- Added Stratum host configuration.
- Added Stratum port configuration.
- Added platform-independent miner configuration loading.
- Added STNM miner session address identification.
- Added STNM `ADDRESS` message type `5`.
- Added 77-byte ADDRESS frame containing the canonical 69-byte `stn0_` address.
- Added miner address validation.
- Added address transmission immediately after Stratum connection.
- Added address-bound Stratum mining sessions.
- Added STNM `HASH_PROGRESS` support.
- Added bounded CPU hashing chunks.
- Added periodic hash-progress reporting to Stratum.
- Added live cumulative hash reporting.
- Added elapsed mining-time reporting.
- Added Stratum socket readability checks between hashing chunks.
- Added replacement-job detection while mining.
- Added stale/replacement work responsiveness.
- Added Windows socket-readability support using Winsock `select()`.
- Added fixed-height CLI mining interface.
- Added persistent miner header.
- Added abbreviated current job ID display.
- Added rolling five-job history.
- Added automatic removal of the oldest displayed job after five entries.
- Added live nonce display.
- Added live cumulative hash display.
- Added mining status display.
- Added result display for accepted, rejected, stale, replaced, and provider-unavailable work.
- Added platform-specific console clearing.
- Added Windows console refresh support using Win32 console APIs.
- Added diagnostic miner logging.
- Added `stn-miner.log`.
- Added connection logging.
- Added ADDRESS frame logging.
- Added JOB receive and parse logging.
- Added work-ID logging.
- Added mining-start logging.
- Added hash-progress logging.
- Added solution logging.
- Added SUBMIT logging.
- Added RESULT logging.
- Added disconnect and reconnect logging.
- Added unlocked Windows log access using shared file handles.
- Added immediate log flush and close after every diagnostic record.
- Added `README.md` documentation for STN Miner architecture, configuration, protocols, building, operation, and development direction.

### Changed

- Miner startup configuration moved from command-line Stratum arguments to `config.json`.
- Miner configuration is now intended to remain identical across Windows, Linux, macOS, ARM, and future STN Chain OS deployments.
- Removed end-user hashing-backend selection from configuration.
- Backend selection remains an internal miner responsibility.
- Changed normal miner output from an append-only diagnostic console to an old-school fixed CLI miner interface.
- Changed current-job display to use abbreviated work IDs.
- Changed job history to retain only the five most recent jobs.
- Changed console refresh behavior to use the platform abstraction rather than embedded ANSI escape sequences.
- Changed CPU mining from one blocking search through `UINT64_MAX` to bounded nonce ranges.
- CPU hashing now periodically returns control to the miner loop.
- Miner now checks for new Stratum data between nonce chunks.
- Miner now checks for replacement work before submitting a discovered solution.
- Miner now reports cumulative hashes and elapsed milliseconds to Stratum.
- Miner logging now uses platform-specific append behavior to ensure the log remains readable while mining.

### Session Flow

    load config.json
        |
        v
    connect to Stratum
        |
        v
    send ADDRESS
        |
        v
    receive JOB
        |
        v
    mine bounded nonce range
        |
        v
    send HASH_PROGRESS
        |
        v
    check for replacement JOB
        |
        v
    continue mining or submit solution

---

## Protocol

Current STNM Miner ↔ Stratum message assignments:

    Type 1    JOB
    Type 2    SUBMIT
    Type 3    RESULT
    Type 4    HASH_PROGRESS
    Type 5    ADDRESS

### ADDRESS

Added miner session identity frame:

    Offset      Size      Field
    ------      ----      -----

    0           4         "STNM"
    4           1         Version = 1
    5           1         Type = 5
    6           2         Reserved = 0
    8           69        Canonical STN Chain address

Total:

    77 bytes

Example:

    stn0_f565306974b8aa6174d42d989e8262817b06b024fe1bfb3b0233699e7f26c1b2

The address identifies the mining participant associated with the Stratum session.

It does not grant consensus authority.

### HASH_PROGRESS

Hash progress reports contain:

    work ID
    cumulative hashes completed
    elapsed milliseconds

Progress is associated with the address registered to the active Stratum session.

---

## Fixed

- Fixed Windows socket connection code where the `socket` parameter name shadowed Winsock's `socket()` function.
- Fixed unreachable-code warning in the miner reconnect loop.
- Fixed Windows console output displaying raw ANSI escape sequences.
- Fixed miner inability to report useful live hashrate while CPU mining.
- Fixed long-running CPU searches preventing timely Stratum progress reporting.
- Fixed miner inability to notice replacement work while hashing.
- Fixed missing miner identity association at Stratum.
- Fixed the protocol gap that prevented Stratum from associating mining work with an STN Chain address.
- Fixed the missing progress data required by Stratum to calculate live miner hashrate.

---

## Verified

Successful Windows Miner → Linux Stratum → Linux Chain operation has been demonstrated.

Example accepted work:

    job: 76ea380d02c4e93f9080158e296e5151e4aeca43e6ce9f3163dcb7846ae43fed
    block bytes: 168
    initial nonce: 0
    solution nonce: 333
    accepted

Address-enabled session operation has been demonstrated:

    CONNECT_OK
    TX ADDRESS bytes=77 type=5
    TX ADDRESS_OK
    RX HEADER bytes=84 magic=53544e4d version=1 type=1
    RX JOB_OK
    MINING_BEGIN

Live hash-progress reporting has been demonstrated through STN-Stratum.

Observed public hashrate:

    154,413 H/s

The hashrate was successfully surfaced on:

    stn-chain.org

This verifies the path:

    STN Miner
        |
        v
    bounded CPU hashing
        |
        v
    HASH_PROGRESS
        |
        v
    STN-Stratum
        |
        v
    address-associated miner session
        |
        v
    live hashrate
        |
        v
    stn-chain.org

---

## Current Mining Architecture

    STN Miner
        |
        | STNM
        v
    STN-Stratum
        |
        | STNC
        v
    STN Chain

Responsibilities remain separated:

    Miner
        hashes work
        reports progress
        submits proof

    Stratum
        coordinates miners
        associates work with addresses
        measures participation
        forwards solved work

    Chain
        independently validates proof
        determines acceptance

---

## Development Direction

Planned miner development continues toward:

- Linux platform implementation.
- ARM32 and ARM64 qualification.
- GPU detection and mining backend support.
- USB-ASIC detection and backend support.
- Multiple simultaneous hashing-device support.
- Dedicated ASIC operation.
- Future STN Chain OS deployment.
- Continued Phase 19 miner-accounting integration.
- Deterministic compensation and payout development when authorized by STN Chain economic policy.

The governing design rule remains:

    Small.
    Deterministic.
    Easy to use.