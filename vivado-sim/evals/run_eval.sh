#!/usr/bin/env bash
# Run trigger evaluation for vivado-sim skill
# Usage: bash run_eval.sh [--verbose] [--runs N] [--workers N] [--model MODEL]
#
# Examples:
#   bash run_eval.sh --verbose
#   bash run_eval.sh --verbose --runs 3 --workers 5
#   bash run_eval.sh --verbose --model haiku

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKILL_DIR="$(dirname "$SCRIPT_DIR")"
EVAL_SET="$SCRIPT_DIR/trigger_eval.json"
EVAL_RUNNER="/home/liulongwei/.claude/skills/skill-creator/scripts/run_eval.py"

# Defaults
VERBOSE=""
RUNS=1
WORKERS=10
MODEL=""

# Parse args
while [[ $# -gt 0 ]]; do
    case $1 in
        --verbose|-v) VERBOSE="--verbose"; shift ;;
        --runs)       RUNS="$2"; shift 2 ;;
        --workers)    WORKERS="$2"; shift 2 ;;
        --model)      MODEL="--model $2"; shift 2 ;;
        --help|-h)
            echo "Usage: $0 [--verbose] [--runs N] [--workers N] [--model MODEL]"
            exit 0
            ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

if [[ ! -f "$EVAL_RUNNER" ]]; then
    echo "Error: Eval runner not found at $EVAL_RUNNER"
    echo "Make sure skill-creator is installed."
    exit 1
fi

echo "=== Vivado-Sim Skill Trigger Evaluation ==="
echo "Skill:    $SKILL_DIR"
echo "Eval set: $EVAL_SET"
echo "Runs/query: $RUNS, Workers: $WORKERS"
echo ""

cd /home/liulongwei

python3 -m scripts.run_eval \
    --eval-set "$EVAL_SET" \
    --skill-path "$SKILL_DIR" \
    --runs-per-query "$RUNS" \
    --num-workers "$WORKERS" \
    --timeout 60 \
    $VERBOSE \
    $MODEL 2>&1 | tee "$SCRIPT_DIR/eval_results.json"

echo ""
echo "Results saved to: $SCRIPT_DIR/eval_results.json"
