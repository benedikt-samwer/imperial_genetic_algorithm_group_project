# Development Guide

## 1. Branch Structure

### 1.1 Core Branches

The project maintains four core development branches, all based on the `data-structure-setup` branch:

- `genetic-algorithm-dev`: Genetic algorithm optimization development
- `circuit-simulation-dev`: Circuit simulation and modeling
- `circuit-viz-dev`: Circuit visualization and UI
- `validity-check-dev`: Circuit validation and verification

### 1.2 Branch Naming Convention

For feature development, use the following naming pattern:
```
<core-branch>/<member-name>/<feature-name>
```

Examples:
- `genetic-algorithm-dev/alice/crossover-optimization`
- `circuit-simulation-dev/bob/flow-calculation`
- `circuit-viz-dev/charlie/d3-integration`
- `validity-check-dev/david/cycle-detection`

## 2. Development Workflow

### 2.1 Starting New Development

1. Choose the appropriate core branch for your feature
2. Create your feature branch:
   ```bash
   git checkout <core-branch>
   git pull origin <core-branch>
   git checkout -b <core-branch>/<your-name>/<feature-name>
   ```

### 2.2 Development Process

1. Make regular commits with meaningful messages:
   ```bash
   git commit -m "feat(<scope>): <description>"
   ```

2. Keep your branch updated:
   ```bash
   git fetch origin
   git rebase origin/<core-branch>
   ```

3. Push your changes:
   ```bash
   git push origin <your-branch-name>
   ```

### 2.3 Code Review Process

1. Create a Pull Request (PR) to merge into your core branch
2. Request reviews from at least two team members
3. Address review comments and update PR
4. Merge only after receiving approvals

## 3. Coding Standards

### 3.1 Commit Message Format

Follow the Conventional Commits specification:

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Adding or modifying tests
- `chore`: Maintenance tasks

Example:
```
feat(genetic): implement tournament selection
fix(simulation): correct mass balance calculation
docs(readme): update installation instructions
```

### 3.2 Code Style

- Use clear and descriptive variable/function names
- Add comments for complex algorithms
- Follow the existing code formatting
- Write unit tests for new features
- Document public APIs and functions

## 4. Testing Guidelines

### 4.1 Unit Testing

- Write tests for new features before implementation (TDD)
- Maintain test coverage above 80%
- Run tests locally before pushing:
  ```bash
  # Example for C++ (CMake/CTest)
  # mkdir build && cd build && cmake .. && make && ctest
  # Example for Python (pytest)
  # pytest
  # Or, using unittest
  # python -m unittest discover
  ```

### 4.2 Integration Testing

- Add integration tests for core functionality
- Test interactions between different modules
- Verify circuit validation rules
- Check simulation convergence

## 5. Documentation

### 5.1 Code Documentation

- Use Doxygen style comments for C++ and Sphinx/Epydoc style comments for Python.
- Document parameters and return values
- Explain complex algorithms
- Keep documentation up to date with code changes

### 5.2 Project Documentation

- Update README.md for new features
- Document API changes
- Maintain architecture diagrams
- Add usage examples

## 6. Deployment and Release

### 6.1 Release Process

1. Version bump following semver
2. Update CHANGELOG.md
3. Create release branch
4. Deploy to staging
5. Run full test suite
6. Deploy to production

### 6.2 Version Control

Follow Semantic Versioning (SemVer):
- MAJOR version for incompatible API changes
- MINOR version for new functionality
- PATCH version for bug fixes

## 7. Issue Tracking

### 7.1 Issue Labels

- `bug`: Something isn't working
- `enhancement`: New feature or request
- `documentation`: Documentation improvements
- `help wanted`: Extra attention needed
- `good first issue`: Good for newcomers

### 7.2 Issue Template

```markdown
## Description
[Describe the issue]

## Expected Behavior
[What should happen]

## Current Behavior
[What happens instead]

## Steps to Reproduce
1. [First Step]
2. [Second Step]
3. [and so on...]

## Environment
- OS: [e.g. macOS]
- Version: [e.g. 1.0.0]
```

## 8. Contact and Support

- Project Lead: [Name]
- Technical Lead: [Name]
- Documentation: [Link]
- Team Channel: [Link]

## 9. Additional Resources

- Project Wiki: [Link]
- API Documentation: [Link]
- Development Environment Setup: [Link]
- Troubleshooting Guide: [Link] 