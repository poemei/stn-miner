# STN Miner

STN Miner is the mining client for STN Chain.

It connects to STN-Stratum, receives canonical mining work, performs proof-of-work, reports mining progress, and submits valid solutions back through Stratum for independent validation by STN Chain.

STN Miner is platform-agnostic by design. Currently qualified platform targets are Windows and Linux, with future STN Chain OS support planned. macOS is not currently supported because no qualification environment is available.

The miner is designed around the same engineering doctrine as STN Chain:

```text
SMALL
DETERMINISTIC
EASY TO USE
```

STN Miner is not a consensus authority.

It provides proof.

STN Chain decides whether that proof is valid.

---

## Architecture

The mining path is:

```text
STN Miner
    |
    | STNM
    v
STN-Stratum
    |
    | STNC
    v
STN Chain
```

Responsibilities remain deliberately separated.

### STN Miner

STN Miner is responsible for:

- loading local miner configuration;
- identifying the mining participant by STN Chain address;
- connecting to STN-Stratum;
- receiving canonical mining jobs;
- hashing the supplied candidate;
- reporting mining progress;
- submitting discovered nonces;
- displaying current mining state;
- maintaining a local diagnostic log.

### STN-Stratum

STN-Stratum is responsible for:

- maintaining miner sessions;
- associating each session with an STN Chain address;
- obtaining canonical work from STN Chain;
- distributing jobs;
- receiving mining progress;
- receiving submitted solutions;
- managing stale work;
- forwarding solved work to STN Chain.

### STN Chain

STN Chain remains responsible for:

- validating block structure;
- validating canonical encoding;
- validating proof-of-work;
- validating target requirements;
- determining accepted chain state;
- accepting or rejecting submitted work.

A miner, Stratum server, peer, or application never becomes consensus authority merely because it supplied data.
[STNC - The STN Chain](https://stn-chain.org)

---

## Current Status

The current implementation has been exercised with:

```text
Windows STN Miner
    |
    v
Linux STN-Stratum
    |
    v
Linux STN Chain
```

Successful end-to-end mining has been demonstrated.

Example:

```text
Job      76ea380d...6ae43fed
Nonce    333
Result   Accepted
```

This demonstrates that mining behavior can cross operating-system boundaries while preserving the same protocol-visible work.

---

## Platform Direction

STN Miner is designed to remain platform-independent.

Common miner behavior must not depend on:

- Windows-specific structures;
- Linux-specific structures;
- native structure padding;
- pointer representation;
- operating-system scheduling;
- architecture-specific memory layout;
- undefined C behavior.

Platform-specific functionality belongs behind the platform abstraction.

Current and intended host environments include:

```text
Windows
Linux
macOS
ARM32
ARM64
future STN Chain OS
```

Mining hardware may include:

```text
CPU
GPU
USB ASIC
dedicated ASIC
```

Hardware capability affects performance.

It does not affect protocol legitimacy.

A low-hashrate participant remains a valid participant if it produces protocol-valid work.

---

## Project Layout

Current project layout:

```text
stn-miner/
├── includes/
├── src/
├── platforms/
│   └── windows/
├── build/
├── config.json
└── build.cmd
```

The common miner implementation lives under:

```text
includes/
src/
```

Operating-system-specific functionality belongs under:

```text
platforms/
```

For example:

```text
platforms/windows/
platforms/linux/
platforms/macos/
platforms/stnos/
```

Platform directories should contain only implementation details required by that platform.

Protocol-visible mining behavior must remain identical.

---

## Configuration

STN Miner uses:

```text
config.json
```

The configuration file is intended to use the same format regardless of operating system or processor architecture.

Example:

```json
{
  "address": "stn0_f565306974b8aa6174d42d989e8262817b06b024fe1bfb3b0233699e7f26c1b2",
  "stratum": {
    "host": "stratum.stn-chain.org",
    "port": 18475
  }
}
```

Current configuration fields are:

```text
address
stratum.host
stratum.port
```

The configuration does not select a hashing backend.

Backend selection belongs to the miner.

The operator should not need to decide whether a system hashes using CPU, GPU, USB ASIC, or dedicated ASIC hardware.

The intended behavior is:

```text
load config
    |
    v
detect platform
    |
    v
detect architecture
    |
    v
detect available mining hardware
    |
    v
select supported hashing backend
    |
    v
mine
```

---

## Mining Address

Every miner session is associated with an STN Chain address.
 - Chain Address may be obtained at 
     - [STNC Address Page](https://stn-chain.org/address)

Example:

```text
stn0_f565306974b8aa6174d42d989e8262817b06b024fe1bfb3b0233699e7f26c1b2
```

The miner loads this address from `config.json`.

After connecting to Stratum, the miner sends an STNM ADDRESS frame.

Current ADDRESS frame:

```text
Offset      Size      Field
------      ----      -----

0           4         "STNM"
4           1         Version = 1
5           1         Type = 5
6           2         Reserved = 0
8           69        Canonical STN address
```

Total:

```text
77 bytes
```

The address identifies which participant performed the mining work.

It does not determine whether submitted work is valid.

---

## STNM Message Types

Current Miner ↔ Stratum message assignments:

```text
Type 1    JOB
Type 2    SUBMIT
Type 3    RESULT
Type 4    HASH_PROGRESS
Type 5    ADDRESS
```

Existing message numbers must not be silently renumbered.

---

## Mining Job

A mining JOB contains:

```text
STNM header
work ID
target
block length
initial nonce
canonical candidate block
```

The candidate block is supplied by Stratum from Chain-derived work.

The miner does not regenerate the candidate.

The miner mutates only the canonical nonce field.

---

## Proof-of-Work

STN Chain currently uses SHA-256 proof-of-work.

Mining uses:

```text
SHA-256
256-bit target
64-bit nonce
fixed nonce mutation region
deterministic work identity
```

The miner searches nonce values and compares the resulting digest against the supplied target.

Conceptually:

```text
Canonical Candidate
    |
    v
Set Nonce
    |
    v
SHA-256
    |
    v
Compare H <= Target
```

A discovered nonce is evidence.

Chain still independently validates it.

---

## Submission

A SUBMIT frame contains:

```text
work ID
nonce
```

The miner does not transmit a replacement block, replacement target, or locally reconstructed candidate.

Stratum associates the submission with:

```text
miner session
STN address
current work
```

and forwards solved work to Chain through the established Chain protocol.

Only Chain acceptance produces an accepted mining result.

---

## Hash Progress

STN Miner supports HASH_PROGRESS messaging.

Hash progress exists to provide mining evidence such as:

```text
work ID
hashes completed
elapsed time
```

Stratum may associate that progress with the address registered to the miner session.

Hash progress does not alter proof-of-work validity.

---

## Console Interface

STN Miner uses a fixed-height, old-school CLI miner interface.

The header remains visible while mining.

Example:

```text
STN Miner
Address : stn0_f565306974b8aa6174d42d989e8262817b06b024fe1bfb3b0233699e7f26c1b2
Stratum : stratum.stn-chain.org:18475
Backend : CPU
Status  : Mining
Job     : 5d6eecc5...de57f3d1
Nonce   : 482193
Hashes  : 482194

Last 5 Jobs
----------------------------------------------------------------
5d6eecc5...de57f3d1   Mining
76ea380d...6ae43fed   Accepted         nonce 333
28a47193...bd920ea4   Accepted         nonce 812
19c933ad...781a0021   Stale
cc0f3381...8a094b33   Chain rejected   nonce 91
----------------------------------------------------------------
```

The display is refreshed in place.

Only the five most recent jobs are retained in the console view.

This prevents the miner from producing an endless scrolling list during normal operation.

Platform-specific console manipulation remains behind the platform abstraction.

---

## Logging

STN Miner writes diagnostic information to:

```text
stn-miner.log
```

The log is append-only during operation.

The miner does not keep the log file permanently open.

Each record is written using:

```text
open
append
flush
close
```

On Windows the file is opened with sharing enabled so it can be inspected while the miner continues running.

The log may be read with tools such as:

```text
type stn-miner.log
```

or:

```powershell
Get-Content .\stn-miner.log -Wait
```

Diagnostic records include events such as:

```text
START
CONNECT
CONNECT_OK
ADDRESS
ADDRESS send result
JOB receive
JOB parse
work ID
mining start
solution
SUBMIT
RESULT
disconnect
protocol failure
```

Example:

```text
[2026-09-21 12:39:20] START address=stn0_f565... stratum=stratum.stn-chain.org:18475 backend=CPU
[2026-09-21 12:39:20] PLATFORM_INIT_OK
[2026-09-21 12:39:20] CONNECT host=stratum.stn-chain.org port=18475
[2026-09-21 12:39:20] CONNECT_OK
[2026-09-21 12:39:21] TX ADDRESS bytes=77 type=5 address=stn0_f565...
[2026-09-21 12:39:21] TX ADDRESS_OK
[2026-09-21 12:39:21] RX WAIT_JOB
[2026-09-21 12:39:21] RX HEADER bytes=84 magic=53544e4d version=1 type=1 reserved=0,0
[2026-09-21 12:39:21] RX JOB_OK work_id=5d6eecc5041b99d798b1c8cf63ad5cf454e63513e102cde6b8fe9305de57f3d1
[2026-09-21 12:39:21] RX JOB_DETAILS block_length=168 initial_nonce=0
[2026-09-21 12:39:21] MINING_BEGIN nonce_start=0
```

Logging is local operating policy.

Logging behavior must not affect mining or consensus results.

---

## Windows Build

The current Windows build uses the Microsoft C compiler directly.

No Visual Studio solution or IDE project is required.

Build:

```text
build
```

Current `build.cmd` compiles the common miner and the Windows platform backend.

The generated executable is:

```text
build\stn-miner.exe
```

Run:

```text
build\stn-miner.exe
```

The miner loads `config.json` at startup.

---

## Build Philosophy

STN Miner intentionally avoids unnecessary build-system complexity.

The current Windows build uses:

```text
cl
build.cmd
```

The project does not require:

```text
.sln
.vcxproj
CMake
IDE-generated project state
```

Other platforms may use appropriate native command-line toolchains while preserving the same source architecture and protocol behavior.

---

## Backend Philosophy

The operator should not need to configure hardware backends manually.

Intended miner behavior:

```text
CPU available
    -> use CPU

GPU available and supported
    -> use GPU

USB ASIC attached and supported
    -> use USB ASIC

multiple supported devices
    -> miner may coordinate available devices

dedicated ASIC
    -> start and mine
```

A dedicated mining appliance should eventually require little more than:

```text
power on
connect
mine
```

---

## Current Development Scope

The current miner implementation focuses on establishing the fundamental external miner path:

```text
configuration
    |
    v
miner identity
    |
    v
Stratum connection
    |
    v
canonical JOB
    |
    v
CPU hashing
    |
    v
SUBMIT
    |
    v
Chain validation
```

GPU, USB-ASIC, dedicated ASIC, and broader platform qualification are later implementation work unless otherwise marked complete in project documentation.

Do not treat planned backend support as already qualified.

---

## Development Rules

Development should remain bounded.

Do not introduce architecture merely because it may be useful someday.

Add capability when there is a defined requirement.

The governing design rule is:

```text
Small.
Deterministic.
Easy to use.
```

Platform-specific functionality must remain behind platform abstractions.

Protocol-visible behavior must not depend on operating system or hardware implementation.

The miner must not duplicate Chain consensus.

---

## Security and Trust Model

STN Miner does not ask Chain to trust the miner.

It supplies evidence.

STN-Stratum does not become authoritative because it coordinates miners.

It supplies and transports evidence.

STN Chain evaluates that evidence independently.

Conceptually:

```text
Mining Proof
    |
    v
Canonical Validation
    |
    v
Consensus Rules
    |
    v
Accepted or Rejected
```

The governing rule remains:

> **Consensus decides.**

---

## Relationship to STN Chain

STN Miner is part of the larger STN Chain mining architecture.

STN Chain is intended to support independently verifiable records and state beyond simple value transfer, including Sentinel Threat Network records, threat intelligence, agreements, software metadata, research, certifications, attestations, and future economic activity.

Mining provides proof-of-work for the Chain.

It does not define what the Chain accepts.

For Chain architecture and consensus requirements, refer to the STN Chain whitepaper.

---

## Documentation
Our Development and Governance Documents
- [CHANGELOG](/docs/CHANGELOG.md)
- [CONTRIBUTING](/docs/CONTRIB.md)

## License

License information should follow the governing STN Chain repository policy.

---

## Project

STN Chain

```text
https://stn-chain.org
```
