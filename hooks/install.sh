#!/bin/bash

# Color codes for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}Installing Palusznium Project pre-commit hook...${NC}"

# Check if the .git directory exists
if [ ! -d ".git" ]; then
  echo -e "${RED}Error: .git directory not found.${NC}"
  echo -e "${RED}Please run this script from the root of the git repository.${NC}"
  exit 1
fi

# Create the hooks directory if it doesn't exist
mkdir -p .git/hooks

# Copy the pre-commit hook to the git hooks directory
cp hooks/pre-commit .git/hooks/pre-commit

# Make the hook executable
chmod +x .git/hooks/pre-commit

echo -e "${GREEN}✓ Pre-commit hook successfully installed!${NC}"
echo -e "${BLUE}The hook will ensure proper branch synchronization before commits.${NC}"
echo -e "${BLUE}To skip the pre-commit check for a specific commit, use: ${NC}git commit --no-verify"
exit 0 