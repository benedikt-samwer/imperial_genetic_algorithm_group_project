# Palusznium Project Git Hooks

This directory contains Git hooks for the Palusznium Project to ensure proper workflow and code quality.

## Pre-commit Hook

The pre-commit hook enforces branch synchronization rules to maintain a clean project history and ensure all team members are working with up-to-date code.

### Branch Synchronization Rules

1. Personal development branches must sync from feature branches
2. Feature branches must sync from pre-main
3. Branch structure: pre-main -> feature branch -> personal development branch
   - Example: pre-main -> genetic-algorithm -> genetic-algorithm/pranav/initial-setup

## Installation

To install the pre-commit hook, run the installation script from the root of the repository:

```bash
chmod +x hooks/install.sh
./hooks/install.sh
```

This will copy the hook to the `.git/hooks` directory and make it executable.

## Skipping the Hook

If you need to bypass the pre-commit check for a specific commit, use:

```bash
git commit --no-verify
```

However, this should only be done in exceptional circumstances, as the hook helps maintain project integrity.

## Troubleshooting

If you encounter issues with the pre-commit hook:

1. Make sure you're working in the correct branch structure
2. Verify that you have the latest changes from your upstream branch
3. Run `git fetch origin` to update your local knowledge of remote branches

For further assistance, contact the project maintainers. 