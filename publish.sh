#!/bin/bash
# Quick publish script for databento-fast v1.0.1

set -e

cd "$(dirname "$0")"

echo "════════════════════════════════════════════════════════════"
echo "Publishing databento-fast v1.0.1"
echo "════════════════════════════════════════════════════════════"
echo ""

# Check if on GitHub repo
if [ ! -d ".git" ]; then
    echo "❌ Not a git repository. Initialize first:"
    echo "   git init"
    echo "   git add ."
    echo "   git commit -m 'v1.0.1: mmap support, 193M rec/s'"
    echo "   git remote add origin https://github.com/rbarfinezhadfeli0/databento-fast.git"
    echo "   git branch -M main"
    echo "   git push -u origin main"
    exit 1
fi

echo "1. Committing changes to Git..."
git add .
git commit -m "v1.0.1: Add mmap support for instant loading (193M rec/s)" || echo "   No changes to commit"

echo ""
echo "2. Tagging release v1.0.1..."
git tag -f v1.0.1

echo ""
echo "3. Pushing to GitHub..."
git push origin main
git push origin v1.0.1 --force

echo ""
echo "════════════════════════════════════════════════════════════"
echo "✅ Published to GitHub!"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "To publish to PyPI:"
echo "  python -m twine upload dist/*"
echo ""
echo "Visit your project:"
echo "  GitHub: https://github.com/rbarfinezhadfeli0/databento-fast"
echo "  PyPI:   https://pypi.org/project/databento-fast/"
echo ""

