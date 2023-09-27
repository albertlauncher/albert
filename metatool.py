#!/usr/bin/env python3
import os
import re
import subprocess
import sys
import argparse
import datetime
import tempfile
from pathlib import Path
from subprocess import run


def create_changelog(args) -> str:
    native_plugins_root = f"{args.root}/plugins"
    python_plugins_root = f"{args.root}/plugins/python/plugins"
    latest_tag = run(["git", "describe", "--tags", "--abbrev=0"], capture_output=True).stdout.decode().strip()
    out = []

    log = run(["git", "log", "--pretty=format:* %B", f"{latest_tag}..HEAD"], capture_output=True).stdout.decode().strip()
    log = re.sub('\n+', '\n', log)
    if log:
        out.append(f"[albert]\n{log}")

    begin = run(["git", "ls-tree", latest_tag, native_plugins_root], capture_output=True).stdout.decode().strip().split()[2]
    log = run(["git", "-C", native_plugins_root, "log", "--pretty=format:* %B", f"{begin}..HEAD"], capture_output=True).stdout.decode().strip()
    log = re.sub('\n+', '\n', log)
    if log:
        out.append(f"[plugins]\n{log}")

    begin = run(["git", "-C", native_plugins_root, "ls-tree", begin, python_plugins_root], capture_output=True).stdout.decode().strip().split()[2]
    log = run(["git", "-C", python_plugins_root, "log", "--pretty=format:* %B", f"{begin}..HEAD"], capture_output=True).stdout.decode().strip()
    log = re.sub('\n+', '\n', log)
    if log:
        out.append(f"[python]\n{log}")

    return '\n\n'.join(out)


def docker_choice(args):
    files = list((Path(args.root) / ".docker").iterdir())

    if args.index is not None:
        indices = args.index
    else:
        for i, f in enumerate(files):
            print(f"{i}: {f.name}")
        indices = input(f"Choose image? [All] ")

    indices = [int(s) for s in filter(None, indices.split())]
    indices = indices if indices else list(range(len(files)))
    return [files[i] for i in indices]


def test_build(args):
    for file in docker_choice(args):
        tag = file.name.replace("Dockerfile", "albert")
        try:
            run(['docker', 'build', '-t', tag, '--target', 'builder', '-f', file, '.'],
                cwd=args.root, env=dict(os.environ, DOCKER_BUILDKIT='0')).check_returncode()
        except subprocess.CalledProcessError as e:
            print(e)
            sys.exit(1)


def test_run(args):
    for file in docker_choice(args):
        tag = file.name.replace("Dockerfile", "albert")
        try:
            run(['docker', 'build', '-t', tag, '--target', 'runtime', '-f', file, '.'],
                cwd=args.root, env=dict(os.environ, DOCKER_BUILDKIT='0')).check_returncode()

            run(['docker', 'container', 'remove', tag],
                cwd=args.root, env=dict(os.environ, DOCKER_BUILDKIT='0'))

            run(['docker', 'create', '-e', 'DISPLAY=docker.for.mac.host.internal:0', '--name', tag, tag],
                cwd=args.root, env=dict(os.environ, DOCKER_BUILDKIT='0')).check_returncode()

            run(['docker', 'start', '-i', tag],
                cwd=args.root, env=dict(os.environ, DOCKER_BUILDKIT='0')).check_returncode()
        except subprocess.CalledProcessError as e:
            print(e)
            sys.exit(1)


def release(args):
    root = Path(args.root)

    if 'master' != run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], capture_output=True).stdout.decode().strip():
        print('Not on master branch')
        sys.exit(1)

    if args.version[0] == 'v':
        args.version = args.version[1:]

    if not re.match(r'^[0-9]+\.[0-9]+\.[0-9]+$', args.version):
        print('Expected version number as parameter: major.minor.patch')
        sys.exit(1)


    print("CHECK THESE!")
    print("- PRs and feature branches merged?")
    print("- submodules staged/committed? (python, plugins, …)")
    print("- 'v%s' > '%s' ?"
          % (args.version, run(["git", "describe", "--tags", "--abbrev=0"], capture_output=True).stdout.decode().strip()))

    if "y".startswith(input("Shall I run a test build in docker (docker running?)? [Y/n] ").lower()):
        test_build(args)

    atomic_changelog = root/f"changelog_v{args.version}"

    with open(atomic_changelog, 'w') as file:
        file.write(create_changelog(args))

    input("Edit the changelog created from git logs to be meaningful to humans. Press Enter to continue...")
    run(["vim", atomic_changelog]).check_returncode()

    if 'yes' == input("Release? (CHANGELOG, VERSION, tagged push)? [yes/NO]").lower():
        print("Appending changelog…")

        with open(atomic_changelog, 'r') as file:
            changelog = file.read().strip()

        with open(root/"CHANGELOG.md", 'r') as file:
            old_changelog = file.read()

        with open(root/"CHANGELOG.md", 'w') as file:
            file.write(f"v{args.version} ({datetime.date.today().strftime('%Y-%m-%d')})\n\n{changelog}\n\n{old_changelog}")

        print("Update CMake project version…")
        run(["sed", "-i.bak", f"s/^set(PROJECT_VERSION.*$/set(PROJECT_VERSION {args.version})/", root/"CMakeLists.txt"], cwd=root).check_returncode()

        run(["git", "add", root/"CHANGELOG.md", root/"CMakeLists.txt"], cwd=root).check_returncode()
        run(["git", "commit", "-m", f"v{args.version}"], cwd=root).check_returncode()
        run(["git", "tag", f"v{args.version}"], cwd=root).check_returncode()
        run(["git", "push", "--tags", "--atomic", "origin", "master"], cwd=root).check_returncode()

        run(["rm", atomic_changelog])
        run(["rm", "CMakeLists.txt.bak"])

        docs_root_path = root / "documentation"

        if docs_root_path.exists():
            run(["git", "pull"], cwd=docs_root_path).check_returncode()
        else:
            run(["git", "clone", "git@github.com:albertlauncher/documentation.git"], cwd=root).check_returncode()

        relative_file_path = f"src/_posts/{datetime.date.today().strftime('%Y-%m-%d')}-albert-v{args.version}-released.md"

        with open(docs_root_path / relative_file_path, 'w') as file:
            file.write(f"""---
layout: docs
title:  "Albert v{args.version} released"
date: {datetime.datetime.now().strftime("%Y-%m-%d %H:%M%z")}
---

{changelog.strip()}

Check the [GitHub repositories](https://github.com/albertlauncher/albert/commits/v{args.version}) for details.
""")

        run(["git", "add", relative_file_path], cwd=docs_root_path).check_returncode()
        run(["git", "commit", "-m", f"Albert v{args.version} released"], cwd=docs_root_path).check_returncode()
        run(["git", "push"], cwd=docs_root_path).check_returncode()

    print("Done.")


def main():
    p = argparse.ArgumentParser()
    sps = p.add_subparsers()

    sp = sps.add_parser('changelog', help='Create raw changelog.')
    sp.set_defaults(func=lambda args: print(create_changelog(args)))

    sp = sps.add_parser('test', help='Test build using docker.')
    sp.add_argument('index', nargs='?', default='')
    sp.set_defaults(func=test_build)

    sp = sps.add_parser('testrun', help='Test run using docker.')
    sp.add_argument('index', nargs='?', default='')
    sp.set_defaults(func=test_run)

    sp = sps.add_parser('release', help="Release a new version.")
    sp.add_argument('version', type=str, help="The semantic version.")
    sp.set_defaults(func=release)

    args = p.parse_args()
    if not hasattr(args, "func"):
        p.print_help()
        sys.exit(1)

    sha = run(["git", "rev-list", "--parents", "HEAD"], capture_output=True).stdout.decode().strip().split("\n")[-1]
    if sha != '4d409110b9771e688acbb995422541f03ef0d8a7':
        print("Working dir is not the albert repository")
        sys.exit(1)

    args.root = run(["git", "rev-parse", "--show-toplevel"], capture_output=True).stdout.decode().strip()

    try:
        args.func(args)
    except KeyboardInterrupt:
        print("\nBye.")


if __name__ == "__main__":
    main()
