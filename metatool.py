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

    placeholder = 'BOOOOM'
    def process_git_log(s: str):
        indented_output = []
        for line in s.split('\n'):
            if not line:  # skip empty
                continue
            if line.startswith(f'{placeholder} '): # replace placeholder with -
                indented_output.append('- ' + line[7:])
            else: # indent all other lines
                indented_output.append('  ' + line)

        return '\n'.join(indented_output)


    log = run(["git", "log", f"--pretty=format:{placeholder} %B", f"{latest_tag}..HEAD"], capture_output=True).stdout.decode().strip()
    log = process_git_log(log)
    if log:
        out.append(f"## Albert\n\n{log}")

    begin = run(["git", "ls-tree", latest_tag, native_plugins_root], capture_output=True).stdout.decode().strip().split()[2]
    log = run(["git", "-C", native_plugins_root, "log", f"--pretty=format:{placeholder} %B", f"{begin}..HEAD"], capture_output=True).stdout.decode().strip()
    log = process_git_log(log)
    if log:
        out.append(f"## Plugins\n\n{log}")

    begin = run(["git", "-C", native_plugins_root, "ls-tree", begin, python_plugins_root], capture_output=True).stdout.decode().strip().split()[2]
    log = run(["git", "-C", python_plugins_root, "log", f"--pretty=format:{placeholder} %B", f"{begin}..HEAD"], capture_output=True).stdout.decode().strip()
    log = process_git_log(log)
    if log:
        out.append(f"## Python\n\n{log}")

    return '\n\n'.join(out)


def test_build(args):

    cmds = {
        'Arch':   ["docker", "build", "--progress=plain", "-f", ".docker/arch.Dockerfile", "-t",
                   "albert:arch", "--platform", "linux/amd64", "."],
        'Fedora': ["docker", "build", "--progress=plain", "-f", ".docker/fedora.Dockerfile", "-t",
                   "albert:fedora", "."],
        'Ubuntu': ["docker", "build", "--progress=plain", "-f", ".docker/ubuntu.Dockerfile", "-t",
                   "albert:ubuntu", "."],
    }


    if args.distribution is not None:
        cmds = [v for k,v  in cmds.items() if args.distribution.lower() in k.lower()]
    else:
        keys = list(cmds.keys())
        for i, k in enumerate(keys):
            print(f"{i}: {k}")
        indices = input("Choose image: [All] ")
        indices = [int(s) for s in filter(None, indices.split())]
        cmds = [cmds[key] for key in [keys[i] for i in indices]] if indices else cmds.values()

    try:
        for cmd in cmds:
            print(f">>> {' '.join(cmd)}")
            run(cmd).check_returncode()
    except subprocess.CalledProcessError as e:
        print(e)
        sys.exit(1)


def release(args):
    root = Path(args.root)

    if 'main' != run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], capture_output=True).stdout.decode().strip():
        print('Not on main branch')
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
        run(["git", "push", "--tags", "--atomic", "origin", "main"], cwd=root).check_returncode()

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
title:  "Albert v{args.version} released"
date: {datetime.datetime.now().strftime("%Y-%m-%d %H:%M%z")}
---

# {{{ page.title }}}

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

    for c in ['changelog', 'cl']:
        sp = sps.add_parser(c, help='Create raw changelog.')
        sp.set_defaults(func=lambda args: print(create_changelog(args)))

    for c in ['test', 't']:
        sp = sps.add_parser(c, help='Test build using docker.')
        sp.add_argument('distribution', type=str, nargs='?', default=None, help="The distro.")
        sp.set_defaults(func=test_build)

    for c in ['release', 'r']:
        sp = sps.add_parser(c, help="Release a new version.")
        sp.add_argument('version', type=str, help="The semantic version.")
        sp.set_defaults(func=release)

    args, unknown = p.parse_known_args()
    args.unknown = unknown
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
