"""
GitHub tools for SAM agents.

These tools demonstrate Level 3 (Advanced) agent capabilities by providing
custom Python functions that interact with GitHub repositories via PyGithub.

No local git installation required - queries GitHub API directly.

Logging Pattern:
    SAM tools use Python's standard logging with a module-level logger.
    Use bracketed identifiers like [ToolName:function] for easy filtering.
    Always use exc_info=True when logging exceptions to capture stack traces.
"""

import logging
from typing import Any, Dict, List, Optional

from github import Github, GithubException

# Module-level logger - SAM will configure this based on your YAML or logging_config.yaml
log = logging.getLogger(__name__)


async def github_get_commits(
    repo: str,
    count: int = 10,
    since: Optional[str] = None,
    branch: Optional[str] = None,
    tool_context: Optional[Any] = None,
    tool_config: Optional[Dict[str, Any]] = None,
) -> Dict[str, Any]:
    """
    Get recent commits from a GitHub repository (no local clone needed).

    Args:
        repo: GitHub repository in "owner/repo" format (e.g., "SolaceLabs/solace-agent-mesh").
        count: Number of commits to retrieve (default: 10, max: 100).
        since: Only show commits after this date (ISO format: "2025-01-01").
        branch: Branch to get commits from (default: repository's default branch).

    Returns:
        A dictionary with commit history from the remote repository.
    """
    log_id = f"[GitTools:get_commits:{repo}]"
    log.debug(f"{log_id} Fetching {count} commits (branch={branch}, since={since})")

    # Get GitHub token from tool_config (optional, increases rate limit)
    token = None
    if tool_config:
        token = tool_config.get("github_token")

    try:
        g = Github(token) if token else Github()
        repository = g.get_repo(repo)

        # Build kwargs for get_commits
        kwargs: Dict[str, Any] = {}
        if branch:
            kwargs["sha"] = branch
        if since:
            from datetime import datetime
            kwargs["since"] = datetime.fromisoformat(since)

        commits_iter = repository.get_commits(**kwargs)

        # Get commits (paginated, so slice carefully)
        commits: List[Dict[str, Any]] = []
        for i, commit in enumerate(commits_iter):
            if i >= min(count, 100):
                break
            commits.append({
                "sha": commit.sha,
                "short_sha": commit.sha[:7],
                "author": commit.commit.author.name if commit.commit.author else "unknown",
                "email": commit.commit.author.email if commit.commit.author else "",
                "date": commit.commit.author.date.strftime("%Y-%m-%d") if commit.commit.author else "",
                "message": commit.commit.message.split("\n")[0],  # First line only
                "url": commit.html_url,
            })

        log.info(f"{log_id} Retrieved {len(commits)} commits")
        return {
            "status": "success",
            "repository": repo,
            "branch": branch or repository.default_branch,
            "commit_count": len(commits),
            "commits": commits,
        }

    except GithubException as e:
        log.error(f"{log_id} GitHub API error: {e.data.get('message', str(e))}", exc_info=True)
        return {
            "status": "error",
            "message": f"GitHub API error: {e.data.get('message', str(e))}",
        }
    except Exception as e:
        log.error(f"{log_id} Unexpected error: {e}", exc_info=True)
        return {
            "status": "error",
            "message": f"Unexpected error: {str(e)}",
        }


async def github_get_releases(
    repo: str,
    count: int = 10,
    include_prereleases: bool = False,
    tool_context: Optional[Any] = None,
    tool_config: Optional[Dict[str, Any]] = None,
) -> Dict[str, Any]:
    """
    Get releases from a GitHub repository.

    Args:
        repo: GitHub repository in "owner/repo" format.
        count: Number of releases to retrieve (default: 10).
        include_prereleases: Include pre-release versions (default: False).

    Returns:
        A dictionary with release information.
    """
    log_id = f"[GitTools:get_releases:{repo}]"
    log.debug(f"{log_id} Fetching {count} releases (include_prereleases={include_prereleases})")

    token = None
    if tool_config:
        token = tool_config.get("github_token")

    try:
        g = Github(token) if token else Github()
        repository = g.get_repo(repo)

        releases: List[Dict[str, Any]] = []
        for i, release in enumerate(repository.get_releases()):
            if i >= count:
                break
            if not include_prereleases and release.prerelease:
                continue
            releases.append({
                "tag": release.tag_name,
                "name": release.title or release.tag_name,
                "date": release.published_at.strftime("%Y-%m-%d") if release.published_at else "",
                "prerelease": release.prerelease,
                "url": release.html_url,
                "body": release.body[:500] if release.body else "",  # Truncate long release notes
            })

        log.info(f"{log_id} Retrieved {len(releases)} releases")
        return {
            "status": "success",
            "repository": repo,
            "release_count": len(releases),
            "releases": releases,
        }

    except GithubException as e:
        log.error(f"{log_id} GitHub API error: {e.data.get('message', str(e))}", exc_info=True)
        return {
            "status": "error",
            "message": f"GitHub API error: {e.data.get('message', str(e))}",
        }
    except Exception as e:
        log.error(f"{log_id} Unexpected error: {e}", exc_info=True)
        return {
            "status": "error",
            "message": f"Unexpected error: {str(e)}",
        }


async def github_compare_commits(
    repo: str,
    base: str,
    head: str,
    tool_context: Optional[Any] = None,
    tool_config: Optional[Dict[str, Any]] = None,
) -> Dict[str, Any]:
    """
    Compare two commits, branches, or tags in a GitHub repository.

    Args:
        repo: GitHub repository in "owner/repo" format.
        base: Base commit/branch/tag for comparison (e.g., "v1.0.0", "main").
        head: Head commit/branch/tag for comparison (e.g., "v1.1.0", "develop").

    Returns:
        A dictionary with comparison information including commits and file changes.
    """
    log_id = f"[GitTools:compare_commits:{repo}]"
    log.debug(f"{log_id} Comparing {base}...{head}")

    token = None
    if tool_config:
        token = tool_config.get("github_token")

    try:
        g = Github(token) if token else Github()
        repository = g.get_repo(repo)

        comparison = repository.compare(base, head)

        commits: List[Dict[str, str]] = []
        for commit in comparison.commits[:50]:  # Limit to 50 commits
            commits.append({
                "sha": commit.sha[:7],
                "message": commit.commit.message.split("\n")[0],
                "author": commit.commit.author.name if commit.commit.author else "unknown",
                "date": commit.commit.author.date.strftime("%Y-%m-%d") if commit.commit.author else "",
            })

        files_changed: List[Dict[str, Any]] = []
        for f in comparison.files[:30]:  # Limit to 30 files
            files_changed.append({
                "filename": f.filename,
                "status": f.status,  # added, removed, modified, renamed
                "additions": f.additions,
                "deletions": f.deletions,
            })

        log.info(f"{log_id} Comparison complete: {comparison.total_commits} commits, {len(comparison.files)} files changed")
        return {
            "status": "success",
            "repository": repo,
            "base": base,
            "head": head,
            "ahead_by": comparison.ahead_by,
            "behind_by": comparison.behind_by,
            "total_commits": comparison.total_commits,
            "commits": commits,
            "files_changed_count": len(comparison.files),
            "files_changed": files_changed,
            "compare_url": comparison.html_url,
        }

    except GithubException as e:
        log.error(f"{log_id} GitHub API error: {e.data.get('message', str(e))}", exc_info=True)
        return {
            "status": "error",
            "message": f"GitHub API error: {e.data.get('message', str(e))}",
        }
    except Exception as e:
        log.error(f"{log_id} Unexpected error: {e}", exc_info=True)
        return {
            "status": "error",
            "message": f"Unexpected error: {str(e)}",
        }


async def github_get_repo_info(
    repo: str,
    tool_context: Optional[Any] = None,
    tool_config: Optional[Dict[str, Any]] = None,
) -> Dict[str, Any]:
    """
    Get information about a GitHub repository.

    Args:
        repo: GitHub repository in "owner/repo" format.

    Returns:
        A dictionary with repository metadata.
    """
    log_id = f"[GitTools:get_repo_info:{repo}]"
    log.debug(f"{log_id} Fetching repository info")

    token = None
    if tool_config:
        token = tool_config.get("github_token")

    try:
        g = Github(token) if token else Github()
        repository = g.get_repo(repo)

        log.info(f"{log_id} Retrieved info for {repository.full_name}")
        return {
            "status": "success",
            "name": repository.name,
            "full_name": repository.full_name,
            "description": repository.description,
            "default_branch": repository.default_branch,
            "stars": repository.stargazers_count,
            "forks": repository.forks_count,
            "open_issues": repository.open_issues_count,
            "language": repository.language,
            "created_at": repository.created_at.strftime("%Y-%m-%d") if repository.created_at else "",
            "updated_at": repository.updated_at.strftime("%Y-%m-%d") if repository.updated_at else "",
            "url": repository.html_url,
        }

    except GithubException as e:
        log.error(f"{log_id} GitHub API error: {e.data.get('message', str(e))}", exc_info=True)
        return {
            "status": "error",
            "message": f"GitHub API error: {e.data.get('message', str(e))}",
        }
    except Exception as e:
        log.error(f"{log_id} Unexpected error: {e}", exc_info=True)
        return {
            "status": "error",
            "message": f"Unexpected error: {str(e)}",
        }
