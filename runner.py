#!/usr/bin/env python3
import sys
import subprocess
import shutil
import importlib.util
import platform
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parent
TEMPLATES_DIR = ROOT / "templates"
PROJECTS_DIR = ROOT / "projects"


# ---------------- OS ----------------

def current_os() -> str:
    name = platform.system().lower()
    if name.startswith("linux"):
        return "linux"
    if name.startswith("darwin"):
        return "macos"
    if name.startswith("windows"):
        return "windows"
    return name


# ---------------- FS utils ----------------

def project_path(name: str) -> Path:
    return PROJECTS_DIR / name


def template_path(name: str) -> Path:
    return TEMPLATES_DIR / name


# ---------------- template copy (RAW COPY ONLY) ----------------

def copy_tree(src: Path, dst: Path):
    if src.is_dir():
        dst.mkdir(parents=True, exist_ok=True)
        for item in src.iterdir():
            copy_tree(item, dst / item.name)
    else:
        shutil.copy2(src, dst)


# ---------------- workflow loader ----------------

def load_workflow(project: str):
    wf_path = project_path(project) / "workflow.py"
    if not wf_path.exists():
        raise RuntimeError("workflow.py not found")

    spec = importlib.util.spec_from_file_location("wf", wf_path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def normalize(x: Any):
    if x is None:
        return []
    if isinstance(x, str):
        return [x]
    return list(x)


def run_cmds(cmds, cwd: Path):
    for c in cmds:
        print(f"[cmd] {c}")
        result = subprocess.run(c, shell=True, cwd=cwd)

        if result.returncode != 0:
            print(f"[fail] command failed (code {result.returncode})")
            return False

    return True


# ---------------- dependencies (ONLY OS) ----------------

def check_os(deps):
    if not deps:
        return True

    os_list = deps.get("os")
    if not os_list:
        return True

    if isinstance(os_list, str):
        os_list = [os_list]

    return current_os() in [o.lower() for o in os_list]


# ---------------- commands ----------------

def cmd_new(template, name):
    src = template_path(template)
    dst = project_path(name)

    if not src.exists():
        raise RuntimeError("template not found")

    if dst.exists():
        raise RuntimeError("project already exists")

    print(f"[new] {name} from {template}")
    copy_tree(src, dst)
    print("[done]")


def cmd_list():
    if not TEMPLATES_DIR.exists():
        return
    for t in sorted(TEMPLATES_DIR.iterdir()):
        if t.is_dir():
            print(t.name)


def run_project(project: str, mode: str):
    wf = load_workflow(project)

    deps = getattr(wf, "dependencies", lambda: None)()
    if not check_os(deps):
        print(f"[skip] {project} (os mismatch)")
        return

    path = project_path(project)

    if mode == "build":
        ok = run_cmds(normalize(wf.build()), path)
        if not ok:
            print(f"[fail] build {project}")
        return

    if mode == "test":
        ok = run_cmds(normalize(wf.tests()), path)
        if not ok:
            print(f"[fail] test {project}")

def get_projects(target):
    if target == "--all":
        return [p.name for p in PROJECTS_DIR.iterdir() if p.is_dir()]
    return [target]


def cmd_build(target):
    for p in get_projects(target):
        run_project(p, "build")


def cmd_test(target):
    for p in get_projects(target):
        run_project(p, "test")


# ---------------- main ----------------

def main():
    if len(sys.argv) < 2:
        print("usage:")
        print("  runner new <template> <name>")
        print("  runner list")
        print("  runner build [project|--all]")
        print("  runner test [project|--all]")
        return

    cmd = sys.argv[1]

    if cmd == "new":
        cmd_new(sys.argv[2], sys.argv[3])

    elif cmd == "list":
        cmd_list()

    elif cmd == "build":
        cmd_build(sys.argv[2])

    elif cmd == "test":
        cmd_test(sys.argv[2])

    else:
        print("unknown command")


if __name__ == "__main__":
    main()