"""
Pre-build script: minifies src/config.html -> src/config.min.html
Registered via extra_scripts = pre:scripts/minify_html.py in platformio.ini.

The minifier:
  - removes HTML comments (<!-- ... -->)
  - strips leading / trailing whitespace from every line
  - drops blank lines
Runtime placeholders (SHORTVALS1, BATTERYENABLED, BATTERYSECTIONSTYLE, etc.)
are plain text / attribute values so they survive unchanged.
"""

import os
import re

Import("env")  # noqa: F821  (PlatformIO injects this)


def _minify(content: str) -> str:
    # Remove HTML comments
    content = re.sub(r"<!--.*?-->", "", content, flags=re.DOTALL)
    # Strip leading/trailing whitespace per line and drop blank lines
    lines = [line.strip() for line in content.splitlines()]
    lines = [line for line in lines if line]
    return "\n".join(lines)


def minify_html(*_args, **_kwargs):
    project_dir = env.subst("$PROJECT_DIR")  # noqa: F821
    src = os.path.join(project_dir, "src", "config.html")
    dst = os.path.join(project_dir, "src", "config.min.html")

    with open(src, "r", encoding="utf-8") as f:
        original = f.read()

    minified = _minify(original)

    with open(dst, "w", encoding="utf-8") as f:
        f.write(minified)

    print(
        f"minify_html: {os.path.getsize(src)} -> {os.path.getsize(dst)} bytes"
        f" ({100.0 * os.path.getsize(dst) / os.path.getsize(src):.1f}%)"
    )


# Run immediately when the pre: script is loaded (before compilation starts)
minify_html()
