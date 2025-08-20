#!/usr/bin/env python3
"""
generate_docs_runner.py

Safe local runner to implement the `.copilot-tasks/generate_docs.md` workflow.

Behavior:
- Parses the YAML-like instructions in the prompt file.
- Assembles context files (STATUS, SSOT, .github/instructions, 00_reference/*.txt).
- Writes per-domain prompt files to `.copilot-tasks/prompts/`.
- By default runs in --dry-run mode (does NOT call Ollama or change git).
- If --execute is passed, will attempt to call local Ollama via the CLI to generate text outputs.
- If --commit is passed together with --branch, will create the branch and commit generated docs locally (no push).

Note: This script deliberately avoids auto-pushing to remote and leaves commit/push control to the operator.
"""
import argparse
import os
import sys
import shutil
import subprocess
import yaml
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read_prompt_file(path: Path) -> dict:
    text = path.read_text(encoding="utf-8")
    # strip surrounding triple-backticks if present
    if text.strip().startswith('```'):
        # remove first and last triple-backtick sections
        parts = text.split('```')
        if len(parts) >= 3:
            text = '```'.join(parts[1:-1])
    # Now attempt to load YAML from the remaining text
    try:
        data = yaml.safe_load(text)
        return data
    except Exception as e:
        print(f"Failed to parse prompt YAML: {e}")
        sys.exit(1)


def gather_context(max_chars=30000):
    ctx = {}
    # STATUS
    status = ROOT / 'docs' / 'STATUS.md'
    if status.exists():
        ctx['STATUS.md'] = status.read_text(encoding='utf-8')[:max_chars]
    # SSOT
    ssot = ROOT / 'src' / 'config' / 'ssot.yaml'
    if ssot.exists():
        ctx['ssot.yaml'] = ssot.read_text(encoding='utf-8')[:max_chars]
    # .github/instructions
    instr_dir = ROOT / '.github' / 'instructions'
    if instr_dir.exists():
        for p in instr_dir.glob('**/*'):
            if p.is_file() and p.suffix in ['.md', '.txt']:
                key = f"instructions/{p.name}"
                try:
                    ctx[key] = p.read_text(encoding='utf-8')[:max_chars]
                except Exception:
                    ctx[key] = p.read_bytes()[:max_chars].decode('utf-8', errors='replace')
    # 00_reference txt files
    ref = ROOT / '00_reference'
    if ref.exists():
        for p in list(ref.glob('**/*.txt'))[:200]:
            key = f"00_reference/{p.name}"
            try:
                ctx[key] = p.read_text(encoding='utf-8')[:max_chars]
            except Exception:
                ctx[key] = p.read_bytes()[:max_chars].decode('utf-8', errors='replace')
    return ctx


def build_domain_prompt(domain_name: str, domain_file: str, yaml_prompt: dict, context: dict) -> str:
    header = [
        f"Regenerate domain doc: {domain_file}",
        "Follow the policy in the prompt exactly. Produce a markdown document with sections:\nOverview, Detailed Description, Algorithm Details, Best Practices, Reference Links.",
        "Do not commit or push changes to remote. Output only the markdown for the domain doc.",
    ]
    # include anchors and short context
    anchors = yaml_prompt.get('anchors', {})
    header.append(f"Anchors: {anchors}")
    header.append("---\nContext excerpts (short):\n")
    # include STATUS and SSOT heads
    if 'STATUS.md' in context:
        header.append("[STATUS.md excerpt]\n" + context['STATUS.md'][:4000])
    if 'ssot.yaml' in context:
        header.append("[ssot.yaml excerpt]\n" + context['ssot.yaml'][:4000])
    header.append('\nPlease produce the domain document now.\n')
    header.append('\n### OUTPUT START (markdown only)\n')
    prompt = '\n\n'.join(header)
    return prompt


def write_prompt_file(out_dir: Path, domain: str, prompt_text: str):
    out_dir.mkdir(parents=True, exist_ok=True)
    p = out_dir / f"{domain}.prompt.txt"
    p.write_text(prompt_text, encoding='utf-8')
    return p


def call_ollama(model: str, prompt_text: str) -> str:
    # Attempt to call local Ollama CLI. This function may raise if Ollama isn't available.
    try:
        proc = subprocess.run(
            ['ollama', 'run', model],
            input=prompt_text.encode('utf-8'),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if proc.returncode != 0:
            raise RuntimeError(f"Ollama call failed: {proc.stderr.decode()}")
        return proc.stdout.decode('utf-8')
    except FileNotFoundError:
        raise RuntimeError('Ollama CLI not found on PATH. Please ensure Ollama is installed and accessible as `ollama`.')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--prompt', required=True, help='Path to .copilot-tasks/generate_docs.md')
    parser.add_argument('--out', default='docs', help='Output docs directory')
    parser.add_argument('--model', default='gpt-OSS:20b', help='Local Ollama model name')
    parser.add_argument('--branch', default='docs/regeneration', help='Git branch to create for generated docs')
    parser.add_argument('--dry-run', action='store_true', default=True, help='Do not call Ollama or commit by default')
    parser.add_argument('--execute', action='store_true', help='Call Ollama to generate outputs')
    parser.add_argument('--commit', action='store_true', help='Create branch and commit generated files locally')
    args = parser.parse_args()

    prompt_path = Path(args.prompt)
    if not prompt_path.exists():
        print(f"Prompt file not found: {prompt_path}")
        sys.exit(1)
    yaml_prompt = read_prompt_file(prompt_path)
    # yaml_prompt can be a dict or a list (safe_load can return either). Normalize to dict.
    if isinstance(yaml_prompt, list):
        # attempt to merge list entries into a single dict
        merged = {}
        for item in yaml_prompt:
            if isinstance(item, dict):
                merged.update(item)
        yaml_prompt = merged

    doms = []
    anchors = None
    if isinstance(yaml_prompt, dict):
        anchors = yaml_prompt.get('anchors')
    elif isinstance(yaml_prompt, list):
        # find a mapping in the list that contains 'anchors' or merge list items
        for item in yaml_prompt:
            if isinstance(item, dict) and 'anchors' in item:
                anchors = item['anchors']
                break
        if anchors is None:
            # maybe anchors are directly a list of mappings at top-level
            # try to merge any mapping entries that contain 'SSOT'/'DOMAINS'
            merged = {}
            for item in yaml_prompt:
                if isinstance(item, dict):
                    merged.update(item)
            anchors = merged.get('anchors')

    if anchors:
        # anchors may be a dict or list of mappings; normalize
        if isinstance(anchors, list):
            # find a mapping with DOMAINS
            for a in anchors:
                if isinstance(a, dict) and 'DOMAINS' in a:
                    doms = a.get('DOMAINS', [])
                    break
        elif isinstance(anchors, dict):
            doms = anchors.get('DOMAINS', [])
    # support top-level DOMAINS key
    if not doms and isinstance(yaml_prompt, dict) and 'DOMAINS' in yaml_prompt:
        doms = yaml_prompt.get('DOMAINS', [])
    # final fallback: if doms is a dict-like, convert to list
    if isinstance(doms, dict):
        doms = list(doms.values())

    context = gather_context()
    prompts_dir = Path('.copilot-tasks') / 'prompts'
    if prompts_dir.exists():
        shutil.rmtree(prompts_dir)
    prompts_dir.mkdir(parents=True, exist_ok=True)

    generated_files = []
    for domain_file in doms:
        domain_name = Path(domain_file).stem
        prompt_text = build_domain_prompt(domain_name, domain_file, yaml_prompt, context)
        prompt_file = write_prompt_file(prompts_dir, domain_name, prompt_text)
        print(f"Wrote prompt for {domain_file} -> {prompt_file}")

        if args.execute:
            print(f"Calling Ollama model {args.model} for {domain_file}...")
            out_text = call_ollama(args.model, prompt_text)
            # Sanitize output: strip any chain-of-thought or preamble before first markdown header
            def sanitize_output(text: str) -> str:
                # If the prompt asked for a marker, try to respect it
                marker = '### OUTPUT START (markdown only)'
                if marker in text:
                    idx = text.find(marker) + len(marker)
                    candidate = text[idx:]
                else:
                    candidate = text
                # Find first markdown header (line starting with '#')
                for i, line in enumerate(candidate.splitlines()):
                    if line.strip().startswith('#'):
                        return '\n'.join(candidate.splitlines()[i:]).lstrip('\n')
                # fallback: return original
                return text

            clean = sanitize_output(out_text)
            if clean != out_text:
                print(f"Sanitized output for {domain_file}: removed leading non-markdown content")

            out_path = Path(args.out) / domain_file
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_text(clean, encoding='utf-8')
            generated_files.append(str(out_path))
            print(f"Wrote generated doc to {out_path}")

    print('\nSummary:')
    print(f"Prepared {len(list(prompts_dir.glob('*.prompt.txt')))} prompt files in {prompts_dir}")
    if args.execute:
        print(f"Generated {len(generated_files)} docs under {args.out}")

    if args.commit and args.execute and generated_files:
        # create branch and commit locally; if branch exists, checkout existing branch
        branch = args.branch
        try:
            subprocess.run(['git', 'rev-parse', '--verify', branch], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            print(f"Branch {branch} already exists; checking out existing branch")
            subprocess.run(['git', 'checkout', branch], check=True)
        except subprocess.CalledProcessError:
            subprocess.run(['git', 'checkout', '-b', branch], check=True)
        subprocess.run(['git', 'add'] + generated_files, check=True)
        subprocess.run(['git', 'commit', '-m', 'docs: regenerate domain docs (local run)'], check=True)
        print(f"Committed generated docs to branch {branch}. Not pushed.")


if __name__ == '__main__':
    main()
