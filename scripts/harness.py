#!/usr/bin/env python3
"""Cross-platform project harness for agents and developers."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


VALID_ACTIONS = ("configure", "build", "test", "validate", "format")
VALID_CONFIGS = ("Debug", "Release", "RelWithDebInfo", "MinSizeRel")


def run_step(name: str, command: list[str]) -> None:
    print(f"==> {name}", flush=True)
    subprocess.run(command, check=True)


def configure(args: argparse.Namespace) -> None:
    enable_test = "ON" if args.enable_test else "OFF"
    run_step(
        f"Configure CMake ({args.config}, ENABLE_TEST={enable_test})",
        [
            "cmake",
            "-S",
            ".",
            "-B",
            args.build_dir,
            f"-DCMAKE_BUILD_TYPE={args.config}",
            f"-DENABLE_TEST={enable_test}",
        ],
    )


def build(args: argparse.Namespace) -> None:
    run_step(
        "Build ({})".format(args.config),
        ["cmake", "--build", args.build_dir, "--config", args.config],
    )


def test(args: argparse.Namespace) -> None:
    run_step(
        "CTest ({})".format(args.config),
        ["ctest", "--test-dir", args.build_dir, "--build-config", args.config, "--output-on-failure"],
    )


def format_sources() -> None:
    run_step("clang-format all plugin sources", [sys.executable, "scripts/clang_format_all.py"])


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run repository verification harness actions.")
    parser.add_argument("--action", choices=VALID_ACTIONS, default="validate")
    parser.add_argument("--build-dir", default=str(Path("out") / "harness-build"))
    parser.add_argument("--config", choices=VALID_CONFIGS, default="Debug")
    parser.add_argument("--enable-test", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    try:
        if args.action == "configure":
            configure(args)
        elif args.action == "build":
            build(args)
        elif args.action == "test":
            test(args)
        elif args.action == "validate":
            configure(args)
            build(args)
            test(args)
        elif args.action == "format":
            format_sources()
    except subprocess.CalledProcessError as exc:
        return exc.returncode

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
