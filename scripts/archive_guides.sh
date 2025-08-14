#!/bin/bash
# Archive outdated/broken-link guides for consolidation
set -e

# Only move if file exists and is tracked
git mv docs/guides/testing_framework_guide.md docs/archives/guides/testing_framework_guide.md || echo "Already moved or not tracked: testing_framework_guide.md"
git mv docs/guides/SIMULATION_FRAMEWORK_GUIDE.md docs/archives/guides/SIMULATION_FRAMEWORK_GUIDE.md || echo "Already moved or not tracked: SIMULATION_FRAMEWORK_GUIDE.md"
git mv docs/guides/safety_systems.md docs/archives/guides/safety_systems.md || echo "Already moved or not tracked: safety_systems.md"
git mv docs/guides/RTOS_TROUBLESHOOTING_GUIDE.md docs/archives/guides/RTOS_TROUBLESHOOTING_GUIDE.md || echo "Already moved or not tracked: RTOS_TROUBLESHOOTING_GUIDE.md"
