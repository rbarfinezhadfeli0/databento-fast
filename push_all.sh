#!/bin/bash
# Push databento-fast to both repositories

set -e

echo "════════════════════════════════════════════════════════════"
echo "Publishing databento-fast v1.0.1"
echo "════════════════════════════════════════════════════════════"
echo ""

# Step 1: Push databento-fast repository
echo "Step 1: Pushing to databento-fast repository..."
cd "$(dirname "$0")"
git push origin main
git push origin v1.0.1 --force

echo ""
echo "✅ databento-fast repository updated"
echo "   URL: https://github.com/rbarfinezhadfeli0/databento-fast"
echo ""

# Step 2: Update parent repository
echo "Step 2: Updating github-v1 submodule reference..."
cd /home/nvidia/github-v1
git add my_github_libraries/databento-fast
git commit -m "Update databento-fast submodule to v1.0.1" || echo "   No changes"
git push origin main

echo ""
echo "✅ github-v1 repository updated"
echo "   URL: https://github.com/rbarfinezhadfeli0/github-v1"
echo ""

echo "════════════════════════════════════════════════════════════"
echo "✅ Published to GitHub!"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "Now visible at:"
echo "  1. https://github.com/rbarfinezhadfeli0/databento-fast"
echo "  2. https://github.com/rbarfinezhadfeli0/github-v1/tree/main/my_github_libraries"
echo ""
echo "To publish to PyPI:"
echo "  cd /home/nvidia/github-v1/my_github_libraries/databento-fast"
echo "  python -m twine upload dist/*"
echo ""

