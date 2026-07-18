# PluginCrypto OpenSSL 3 API Compatibility

Date: 2026-07-04

## Context

`ThirdParty/sVanillaPluginCommon/PluginCrypto` provides shared crypto helpers
used by plugins. Some implementation details call OpenSSL APIs that are
deprecated in OpenSSL 3, especially direct RSA object access and legacy block
size constants.

## Goal

Update `PluginCrypto` to use current OpenSSL EVP-level APIs while preserving the
public helper signatures and byte-for-byte outputs for existing AES, RSA, and
MD5 helpers.

## Non-Goals

- Do not change public function names or formats.
- Do not change padding behavior, AES mode names, hex casing, or RSA no-padding
  input padding rules.
- Do not alter unrelated network or plugin behavior.

## Acceptance Criteria

- `Crypto.cpp` no longer uses direct deprecated RSA access helpers such as
  `EVP_PKEY_get1_RSA`, `RSA_size`, or `RSA_free`.
- AES buffer sizing uses EVP cipher metadata instead of `AES_BLOCK_SIZE` or
  deprecated cipher-size calls.
- Existing helper outputs remain stable for deterministic vectors.
- Narrow harness build and unit test results are recorded.
