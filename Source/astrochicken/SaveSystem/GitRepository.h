// Source/astrochicken/SaveSystem/GitRepository.h

#pragma once

#include "CoreMinimal.h"
#include "SaveData.h"

// Forward declarations for libgit2 types (to avoid header pollution)
struct git_repository;
struct git_signature;
struct git_oid;
struct git_commit;
struct git_tree;
struct git_index;
struct git_reference;

/**
 * Wrapper around libgit2 for git repository operations
 * Handles all low-level git operations for the save system
 */
class ASTROCHICKEN_API FGitRepository
{
public:
	FGitRepository();
	~FGitRepository();

	// Repository initialization
	bool InitializeLibGit2();
	void ShutdownLibGit2();

	// Repository management
	bool CreateRepository(const FString& Path);
	bool OpenRepository(const FString& Path);
	void CloseRepository();
	bool IsOpen() const { return Repository != nullptr; }

	// Commit operations
	bool CreateCommit(
		const FString& Message,
		const FString& AuthorName,
		const FString& AuthorEmail,
		FString& OutCommitSHA
	);

	bool GetCommitInfo(const FString& CommitSHA, FSaveMetadata& OutMetadata);
	bool CheckoutCommit(const FString& CommitSHA);

	// Branch operations
	bool CreateBranch(const FString& BranchName, const FString& FromCommitSHA = TEXT(""));
	bool SwitchBranch(const FString& BranchName);
	bool DeleteBranch(const FString& BranchName);
	bool GetCurrentBranch(FString& OutBranchName);
	bool ListBranches(TArray<FString>& OutBranches);

	// Tag operations (for checkpoints)
	bool CreateTag(const FString& TagName, const FString& CommitSHA, const FString& Message);
	bool GetTag(const FString& TagName, FString& OutCommitSHA);
	bool ListTags(TArray<FString>& OutTags);

	// File operations
	bool StageFile(const FString& RelativePath);
	bool StageAllFiles();
	bool GetWorkingDirectoryPath(FString& OutPath);

	// History operations
	bool GetCommitHistory(TArray<FString>& OutCommitSHAs, int32 MaxCount = -1);
	bool GetParentCommit(const FString& CommitSHA, FString& OutParentSHA);
	bool GetChildCommits(const FString& CommitSHA, TArray<FString>& OutChildSHAs);

	// Cleanup operations
	bool PruneOldCommits(const FString& BranchName, int32 KeepCount);

	// Error handling
	FString GetLastError() const { return LastError; }

private:
	// Helper functions
	bool CreateSignature(git_signature** OutSignature, const FString& Name, const FString& Email);
	bool CommitTreeFromIndex(const git_oid* ParentCommit, const FString& Message, git_oid* OutCommitOID);
	void SetError(const FString& Error);
	FString OIDToString(const git_oid* OID);
	bool StringToOID(const FString& SHA, git_oid* OutOID);

	git_repository* Repository = nullptr;
	FString LastError;
	bool bLibGit2Initialized = false;
};
