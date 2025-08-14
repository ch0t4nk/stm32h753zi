#!/usr/bin/env python3
"""
Markdown Link Validator
Checks all markdown files for broken internal links and suggests fixes
"""

import os
import re
import sys
from pathlib import Path
from urllib.parse import urlparse


def find_markdown_files(root_path, exclude_dirs=None):
    """Find all markdown files in the workspace"""
    if exclude_dirs is None:
        exclude_dirs = []

    md_files = []
    for root, dirs, files in os.walk(root_path):
        # Skip excluded directories
        dirs[:] = [d for d in dirs if d not in exclude_dirs]

        for file in files:
            if file.endswith(".md"):
                md_files.append(Path(root) / file)

    return md_files


def extract_links(file_path):
    """Extract all links from a markdown file, excluding code blocks"""
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return []

    # Remove code blocks first (both ``` and `` blocks)
    # Remove triple backtick blocks
    content = re.sub(r"```[\s\S]*?```", "", content, flags=re.MULTILINE)
    # Remove inline code blocks
    content = re.sub(r"`[^`]*?`", "", content)

    # Find markdown links [text](url) and reference links [text]: url
    link_patterns = [
        r"\[([^\]]*)\]\(([^)]+)\)",
        # [text](url) - but exclude empty text with code-like URLs
        r"\[([^\]]*)\]:\s*([^\s]+)",  # [text]: url
    ]

    links = []
    for pattern in link_patterns:
        matches = re.findall(pattern, content)
        for match in matches:
            if len(match) == 2:
                text, url = match
                # Skip links that look like code artifacts
                if not text.strip() and (
                    url.startswith("<")
                    or url.startswith("(")
                    or "VERSION" in url
                    or "GPIO" in url
                    or "HAL_" in url
                    or url.count("<") > 0
                    or url.count("(") > 0
                ):
                    continue
                # Skip URLs that look like code or contain code patterns
                if (
                    url.startswith("PROTOCOL")
                    or url.startswith("PERIPHERAL")  # noqa: E501
                    or url.startswith("FUNCTION_NAME")
                    or url.startswith("KEYWORD")  # noqa: E501
                    or "stdint.h" in url
                    or "stdbool.h" in url
                    or "stddef.h" in url
                ):  # noqa: E501
                    continue
                links.append((text, url))

    return links


def classify_link(url):
    """Classify link type"""
    parsed = urlparse(url)

    if parsed.scheme in ["http", "https"]:
        return "external"
    elif parsed.scheme in ["mailto", "ftp"]:
        return "other"
    elif url.startswith("#"):
        return "anchor"
    elif url.startswith("/"):
        return "absolute"
    else:
        return "relative"


def check_internal_link(link_url, file_path, workspace_root):
    """Check if an internal link is valid"""
    try:
        # Remove anchor fragments
        clean_url = link_url.split("#")[0]
        if not clean_url:  # Pure anchor link
            return True, "Anchor link (not validated)"

        # Handle relative paths
        if not clean_url.startswith("/"):
            # Relative to current file
            target_path = file_path.parent / clean_url
        else:
            # Absolute path from workspace root
            target_path = workspace_root / clean_url.lstrip("/")

        # Resolve and check if exists
        resolved_path = target_path.resolve()

        if resolved_path.exists():
            return True, "Valid"
        else:
            # Try to find similar files
            suggestions = find_similar_files(resolved_path, workspace_root)
            return False, f"File not found. Suggestions: {suggestions}"

    except Exception as e:
        return False, f"Error checking link: {e}"


def find_similar_files(target_path, workspace_root):
    """Find files with similar names"""
    target_stem = target_path.stem

    suggestions = []

    # Search for files with similar names
    for md_file in workspace_root.rglob("*.md"):
        if md_file.stem == target_stem or target_stem in md_file.stem:
            relative_path = md_file.relative_to(workspace_root)
            suggestions.append(str(relative_path))

    return suggestions[:3]  # Limit to top 3 suggestions


def validate_workspace_links(workspace_root, exclude_dirs=None):
    """Validate all links in workspace markdown files"""
    if exclude_dirs is None:
        exclude_dirs = [".git", "node_modules", "__pycache__"]

    workspace_path = Path(workspace_root)
    md_files = find_markdown_files(workspace_path, exclude_dirs)

    print(f"🔍 Validating links in {len(md_files)} markdown files...")
    print("=" * 60)

    total_links = 0
    broken_links = 0
    html_links = 0

    for md_file in md_files:
        relative_path = md_file.relative_to(workspace_path)
        links = extract_links(md_file)

        if not links:
            continue

        file_issues = []

        for text, url in links:
            total_links += 1
            link_type = classify_link(url)

            # Check for .html links (potential broken references)
            if url.endswith(".html"):
                html_links += 1
                # Suggest .md replacement
                md_url = url.replace(".html", ".md")
                file_issues.append(
                    {
                        "type": "html_link",
                        "text": text,
                        "url": url,
                        "suggestion": md_url,
                        "status": "HTML link (consider .md)",
                    }
                )

            # Validate internal links
            elif link_type in ["relative", "absolute"]:
                is_valid, status = check_internal_link(
                    url, md_file, workspace_path
                )
                if not is_valid:
                    broken_links += 1
                    file_issues.append(
                        {
                            "type": "broken_link",
                            "text": text,
                            "url": url,
                            "suggestion": "",
                            "status": status,
                        }
                    )

        # Report issues for this file
        if file_issues:
            print(f"\n📄 {relative_path}")
            for issue in file_issues:
                if issue["type"] == "html_link":
                    print(
                        f"  🔗 HTML: [{issue['text']}]({issue['url']}) → "
                        f"Suggest: {issue['suggestion']}"
                    )
                else:
                    print(
                        f"  ❌ BROKEN: [{issue['text']}]({issue['url']}) - "
                        f"{issue['status']}"
                    )

    # Summary
    print("\n📊 Link Validation Summary")
    print("=" * 30)
    print(f"Total links checked: {total_links}")
    print(f"Broken internal links: {broken_links}")
    print(f"HTML links found: {html_links}")
    print(
        f"Files with issues: {len([f for f in md_files if extract_links(f)])}"
    )

    return {
        "total_links": total_links,
        "broken_links": broken_links,
        "html_links": html_links,
    }


def main():
    workspace_root = Path("/workspaces/code")

    if len(sys.argv) > 1:
        if sys.argv[1] == "--include-reference":
            exclude_dirs = [".git", "node_modules", "__pycache__"]
        else:
            print("Usage: python3 link_validator.py [--include-reference]")
            print(
                "  --include-reference: Include 00_reference directory "
                "in validation"
            )
            return
    else:
        exclude_dirs = [".git", "node_modules", "__pycache__", "00_reference"]

    validate_workspace_links(workspace_root, exclude_dirs)


if __name__ == "__main__":
    main()
