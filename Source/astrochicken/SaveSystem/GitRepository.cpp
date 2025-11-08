// Source/astrochicken/SaveSystem/GitRepository.cpp

#include "GitRepository.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

// libgit2 includes
#include "git2.h"

FGitRepository::FGitRepository()
{
	InitializeLibGit2();
}

FGitRepository::~FGitRepository()
{
	CloseRepository();
	ShutdownLibGit2();
}

bool FGitRepository::InitializeLibGit2()
{
	if (bLibGit2Initialized)
	{
		return true;
	}

	int32 Result = git_libgit2_init();
	if (Result < 0)
	{
		SetError(TEXT("Failed to initialize libgit2"));
		return false;
	}

	bLibGit2Initialized = true;
	UE_LOG(LogTemp, Log, TEXT("GitRepository: libgit2 initialized"));
	return true;
}

void FGitRepository::ShutdownLibGit2()
{
	if (bLibGit2Initialized)
	{
		git_libgit2_shutdown();
		bLibGit2Initialized = false;
		UE_LOG(LogTemp, Log, TEXT("GitRepository: libgit2 shutdown"));
	}
}

bool FGitRepository::CreateRepository(const FString& Path)
{
	if (Repository)
	{
		SetError(TEXT("Repository already open"));
		return false;
	}

	// Convert to absolute path
	FString AbsolutePath = FPaths::ConvertRelativePathToFull(Path);

	// Create directory if it doesn't exist
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*AbsolutePath))
	{
		if (!PlatformFile.CreateDirectoryTree(*AbsolutePath))
		{
			SetError(FString::Printf(TEXT("Failed to create directory: %s"), *AbsolutePath));
			return false;
		}
	}

	// Initialize git repository
	git_repository* Repo = nullptr;
	int32 Result = git_repository_init(&Repo, TCHAR_TO_ANSI(*AbsolutePath), 0);

	if (Result != 0)
	{
		const git_error* Error = git_error_last();
		SetError(FString::Printf(TEXT("Failed to create repository: %s"),
			Error ? ANSI_TO_TCHAR(Error->message) : TEXT("Unknown error")));
		return false;
	}

	Repository = Repo;
	UE_LOG(LogTemp, Log, TEXT("GitRepository: Created repository at %s"), *AbsolutePath);
	return true;
}

bool FGitRepository::OpenRepository(const FString& Path)
{
	if (Repository)
	{
		SetError(TEXT("Repository already open"));
		return false;
	}

	FString AbsolutePath = FPaths::ConvertRelativePathToFull(Path);

	git_repository* Repo = nullptr;
	int32 Result = git_repository_open(&Repo, TCHAR_TO_ANSI(*AbsolutePath));

	if (Result != 0)
	{
		const git_error* Error = git_error_last();
		SetError(FString::Printf(TEXT("Failed to open repository: %s"),
			Error ? ANSI_TO_TCHAR(Error->message) : TEXT("Unknown error")));
		return false;
	}

	Repository = Repo;
	UE_LOG(LogTemp, Log, TEXT("GitRepository: Opened repository at %s"), *AbsolutePath);
	return true;
}

void FGitRepository::CloseRepository()
{
	if (Repository)
	{
		git_repository_free(Repository);
		Repository = nullptr;
		UE_LOG(LogTemp, Log, TEXT("GitRepository: Closed repository"));
	}
}

bool FGitRepository::CreateSignature(git_signature** OutSignature, const FString& Name, const FString& Email)
{
	int32 Result = git_signature_now(OutSignature,
		TCHAR_TO_ANSI(*Name),
		TCHAR_TO_ANSI(*Email));

	if (Result != 0)
	{
		const git_error* Error = git_error_last();
		SetError(FString::Printf(TEXT("Failed to create signature: %s"),
			Error ? ANSI_TO_TCHAR(Error->message) : TEXT("Unknown error")));
		return false;
	}

	return true;
}

bool FGitRepository::StageFile(const FString& RelativePath)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	git_index* Index = nullptr;
	int32 Result = git_repository_index(&Index, Repository);

	if (Result != 0)
	{
		SetError(TEXT("Failed to get repository index"));
		return false;
	}

	Result = git_index_add_bypath(Index, TCHAR_TO_ANSI(*RelativePath));

	if (Result != 0)
	{
		const git_error* Error = git_error_last();
		SetError(FString::Printf(TEXT("Failed to stage file: %s"),
			Error ? ANSI_TO_TCHAR(Error->message) : TEXT("Unknown error")));
		git_index_free(Index);
		return false;
	}

	Result = git_index_write(Index);
	git_index_free(Index);

	if (Result != 0)
	{
		SetError(TEXT("Failed to write index"));
		return false;
	}

	return true;
}

bool FGitRepository::StageAllFiles()
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	git_index* Index = nullptr;
	int32 Result = git_repository_index(&Index, Repository);

	if (Result != 0)
	{
		SetError(TEXT("Failed to get repository index"));
		return false;
	}

	// Add all files
	Result = git_index_add_all(Index, nullptr, 0, nullptr, nullptr);

	if (Result != 0)
	{
		const git_error* Error = git_error_last();
		SetError(FString::Printf(TEXT("Failed to stage all files: %s"),
			Error ? ANSI_TO_TCHAR(Error->message) : TEXT("Unknown error")));
		git_index_free(Index);
		return false;
	}

	Result = git_index_write(Index);
	git_index_free(Index);

	if (Result != 0)
	{
		SetError(TEXT("Failed to write index"));
		return false;
	}

	return true;
}

bool FGitRepository::CreateCommit(
	const FString& Message,
	const FString& AuthorName,
	const FString& AuthorEmail,
	FString& OutCommitSHA)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	// Create signature
	git_signature* Signature = nullptr;
	if (!CreateSignature(&Signature, AuthorName, AuthorEmail))
	{
		return false;
	}

	// Get the index
	git_index* Index = nullptr;
	int32 Result = git_repository_index(&Index, Repository);
	if (Result != 0)
	{
		git_signature_free(Signature);
		SetError(TEXT("Failed to get index"));
		return false;
	}

	// Write tree from index
	git_oid TreeOID;
	Result = git_index_write_tree(&TreeOID, Index);
	git_index_free(Index);

	if (Result != 0)
	{
		git_signature_free(Signature);
		SetError(TEXT("Failed to write tree"));
		return false;
	}

	// Get tree object
	git_tree* Tree = nullptr;
	Result = git_tree_lookup(&Tree, Repository, &TreeOID);
	if (Result != 0)
	{
		git_signature_free(Signature);
		SetError(TEXT("Failed to lookup tree"));
		return false;
	}

	// Get HEAD commit (parent) if it exists
	git_oid ParentOID;
	git_commit* ParentCommit = nullptr;
	bool bHasParent = false;

	git_reference* Head = nullptr;
	if (git_repository_head(&Head, Repository) == 0)
	{
		const git_oid* HeadOID = git_reference_target(Head);
		if (HeadOID && git_commit_lookup(&ParentCommit, Repository, HeadOID) == 0)
		{
			bHasParent = true;
		}
		git_reference_free(Head);
	}

	// Create commit
	git_oid CommitOID;
	const git_commit* Parents[] = { ParentCommit };

	Result = git_commit_create_v(
		&CommitOID,
		Repository,
		"HEAD",
		Signature,
		Signature,
		nullptr,
		TCHAR_TO_ANSI(*Message),
		Tree,
		bHasParent ? 1 : 0,
		bHasParent ? Parents : nullptr
	);

	// Cleanup
	git_tree_free(Tree);
	git_signature_free(Signature);
	if (ParentCommit)
	{
		git_commit_free(ParentCommit);
	}

	if (Result != 0)
	{
		const git_error* Error = git_error_last();
		SetError(FString::Printf(TEXT("Failed to create commit: %s"),
			Error ? ANSI_TO_TCHAR(Error->message) : TEXT("Unknown error")));
		return false;
	}

	// Convert OID to string
	OutCommitSHA = OIDToString(&CommitOID);

	UE_LOG(LogTemp, Log, TEXT("GitRepository: Created commit %s"), *OutCommitSHA);
	return true;
}

bool FGitRepository::CreateBranch(const FString& BranchName, const FString& FromCommitSHA)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	git_commit* TargetCommit = nullptr;

	// If FromCommitSHA is provided, use that, otherwise use HEAD
	if (!FromCommitSHA.IsEmpty())
	{
		git_oid OID;
		if (!StringToOID(FromCommitSHA, &OID))
		{
			return false;
		}

		if (git_commit_lookup(&TargetCommit, Repository, &OID) != 0)
		{
			SetError(TEXT("Failed to find target commit"));
			return false;
		}
	}
	else
	{
		git_reference* Head = nullptr;
		if (git_repository_head(&Head, Repository) != 0)
		{
			SetError(TEXT("Failed to get HEAD"));
			return false;
		}

		const git_oid* HeadOID = git_reference_target(Head);
		if (git_commit_lookup(&TargetCommit, Repository, HeadOID) != 0)
		{
			git_reference_free(Head);
			SetError(TEXT("Failed to lookup HEAD commit"));
			return false;
		}

		git_reference_free(Head);
	}

	// Create branch
	git_reference* BranchRef = nullptr;
	int32 Result = git_branch_create(&BranchRef, Repository, TCHAR_TO_ANSI(*BranchName), TargetCommit, 0);

	git_commit_free(TargetCommit);

	if (Result != 0)
	{
		const git_error* Error = git_error_last();
		SetError(FString::Printf(TEXT("Failed to create branch: %s"),
			Error ? ANSI_TO_TCHAR(Error->message) : TEXT("Unknown error")));
		return false;
	}

	git_reference_free(BranchRef);

	UE_LOG(LogTemp, Log, TEXT("GitRepository: Created branch %s"), *BranchName);
	return true;
}

bool FGitRepository::SwitchBranch(const FString& BranchName)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	FString RefName = FString::Printf(TEXT("refs/heads/%s"), *BranchName);

	git_reference* BranchRef = nullptr;
	int32 Result = git_reference_lookup(&BranchRef, Repository, TCHAR_TO_ANSI(*RefName));

	if (Result != 0)
	{
		SetError(TEXT("Branch not found"));
		return false;
	}

	Result = git_repository_set_head(Repository, TCHAR_TO_ANSI(*RefName));
	git_reference_free(BranchRef);

	if (Result != 0)
	{
		SetError(TEXT("Failed to switch branch"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GitRepository: Switched to branch %s"), *BranchName);
	return true;
}

bool FGitRepository::CreateTag(const FString& TagName, const FString& CommitSHA, const FString& Message)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	git_oid CommitOID;
	if (!StringToOID(CommitSHA, &CommitOID))
	{
		return false;
	}

	git_commit* Commit = nullptr;
	if (git_commit_lookup(&Commit, Repository, &CommitOID) != 0)
	{
		SetError(TEXT("Failed to find commit"));
		return false;
	}

	git_signature* Signature = nullptr;
	if (!CreateSignature(&Signature, TEXT("AstroChicken"), TEXT("save@astrochicken.game")))
	{
		git_commit_free(Commit);
		return false;
	}

	git_oid TagOID;
	int32 Result = git_tag_create(
		&TagOID,
		Repository,
		TCHAR_TO_ANSI(*TagName),
		reinterpret_cast<git_object*>(Commit),
		Signature,
		TCHAR_TO_ANSI(*Message),
		0 // Don't force
	);

	git_signature_free(Signature);
	git_commit_free(Commit);

	if (Result != 0)
	{
		const git_error* Error = git_error_last();
		SetError(FString::Printf(TEXT("Failed to create tag: %s"),
			Error ? ANSI_TO_TCHAR(Error->message) : TEXT("Unknown error")));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GitRepository: Created tag %s"), *TagName);
	return true;
}

bool FGitRepository::GetCurrentBranch(FString& OutBranchName)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	git_reference* Head = nullptr;
	if (git_repository_head(&Head, Repository) != 0)
	{
		SetError(TEXT("Failed to get HEAD"));
		return false;
	}

	const char* BranchName = nullptr;
	if (git_branch_name(&BranchName, Head) == 0)
	{
		OutBranchName = ANSI_TO_TCHAR(BranchName);
		git_reference_free(Head);
		return true;
	}

	git_reference_free(Head);
	SetError(TEXT("Failed to get branch name"));
	return false;
}

bool FGitRepository::GetWorkingDirectoryPath(FString& OutPath)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	const char* Path = git_repository_workdir(Repository);
	if (Path)
	{
		OutPath = ANSI_TO_TCHAR(Path);
		return true;
	}

	SetError(TEXT("Failed to get working directory"));
	return false;
}

FString FGitRepository::OIDToString(const git_oid* OID)
{
	char SHA[GIT_OID_HEXSZ + 1];
	git_oid_tostr(SHA, sizeof(SHA), OID);
	return ANSI_TO_TCHAR(SHA);
}

bool FGitRepository::StringToOID(const FString& SHA, git_oid* OutOID)
{
	if (git_oid_fromstr(OutOID, TCHAR_TO_ANSI(*SHA)) != 0)
	{
		SetError(TEXT("Invalid commit SHA"));
		return false;
	}
	return true;
}

void FGitRepository::SetError(const FString& Error)
{
	LastError = Error;
	UE_LOG(LogTemp, Error, TEXT("GitRepository Error: %s"), *Error);
}

bool FGitRepository::GetCommitHistory(TArray<FString>& OutCommitSHAs, int32 MaxCount)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	git_revwalk* Walker = nullptr;
	if (git_revwalk_new(&Walker, Repository) != 0)
	{
		SetError(TEXT("Failed to create rev walker"));
		return false;
	}

	git_revwalk_sorting(Walker, GIT_SORT_TIME);
	git_revwalk_push_head(Walker);

	git_oid OID;
	int32 Count = 0;

	while (git_revwalk_next(&OID, Walker) == 0)
	{
		OutCommitSHAs.Add(OIDToString(&OID));

		if (MaxCount > 0)
		{
			Count++;
			if (Count >= MaxCount)
			{
				break;
			}
		}
	}

	git_revwalk_free(Walker);
	return true;
}

bool FGitRepository::CheckoutCommit(const FString& CommitSHA)
{
	if (!Repository)
	{
		SetError(TEXT("No repository open"));
		return false;
	}

	git_oid OID;
	if (!StringToOID(CommitSHA, &OID))
	{
		return false;
	}

	git_commit* Commit = nullptr;
	if (git_commit_lookup(&Commit, Repository, &OID) != 0)
	{
		SetError(TEXT("Failed to find commit"));
		return false;
	}

	git_checkout_options CheckoutOpts;
	git_checkout_options_init(&CheckoutOpts, GIT_CHECKOUT_OPTIONS_VERSION);
	CheckoutOpts.checkout_strategy = GIT_CHECKOUT_SAFE;

	int32 Result = git_checkout_tree(Repository, reinterpret_cast<git_object*>(Commit), &CheckoutOpts);

	git_commit_free(Commit);

	if (Result != 0)
	{
		SetError(TEXT("Failed to checkout commit"));
		return false;
	}

	// Update HEAD
	FString RefName = FString::Printf(TEXT("refs/heads/detached-%s"), *CommitSHA.Left(8));
	git_repository_set_head_detached(Repository, &OID);

	UE_LOG(LogTemp, Log, TEXT("GitRepository: Checked out commit %s"), *CommitSHA);
	return true;
}

// Stub implementations for functions not yet fully implemented
bool FGitRepository::GetCommitInfo(const FString& CommitSHA, FSaveMetadata& OutMetadata)
{
	// TODO: Parse commit message for metadata
	SetError(TEXT("GetCommitInfo not yet implemented"));
	return false;
}

bool FGitRepository::DeleteBranch(const FString& BranchName)
{
	// TODO: Implement
	SetError(TEXT("DeleteBranch not yet implemented"));
	return false;
}

bool FGitRepository::ListBranches(TArray<FString>& OutBranches)
{
	// TODO: Implement
	SetError(TEXT("ListBranches not yet implemented"));
	return false;
}

bool FGitRepository::GetTag(const FString& TagName, FString& OutCommitSHA)
{
	// TODO: Implement
	SetError(TEXT("GetTag not yet implemented"));
	return false;
}

bool FGitRepository::ListTags(TArray<FString>& OutTags)
{
	// TODO: Implement
	SetError(TEXT("ListTags not yet implemented"));
	return false;
}

bool FGitRepository::GetParentCommit(const FString& CommitSHA, FString& OutParentSHA)
{
	// TODO: Implement
	SetError(TEXT("GetParentCommit not yet implemented"));
	return false;
}

bool FGitRepository::GetChildCommits(const FString& CommitSHA, TArray<FString>& OutChildSHAs)
{
	// TODO: Implement
	SetError(TEXT("GetChildCommits not yet implemented"));
	return false;
}

bool FGitRepository::PruneOldCommits(const FString& BranchName, int32 KeepCount)
{
	// TODO: Implement
	SetError(TEXT("PruneOldCommits not yet implemented"));
	return false;
}
