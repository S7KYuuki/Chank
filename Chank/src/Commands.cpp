#include "Commands.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio> /* defines FILENAME_MAX */
#include <ctime>
#include <filesystem>
namespace fs = std::filesystem;

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
#define stat _stat
#else
#include <unistd.h>
#endif
#define REQUIRED_ARGS(x)                                            \
	if (args.size() < x)                                            \
	{                                                               \
		printf("Incorrect number of arguments (%d required)\n", x); \
		return;                                                     \
	}

namespace chank
{
void exit(Tree *tree, std::vector<std::string> &args) {}

void pwd(Tree *tree, std::vector<std::string> &args)
{
	printf("%s\n", tree->GetCurrentPath().c_str());
}

void cd(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(1);
	tree->ChangeCurrent(args.front().c_str());
}

void ls(Tree *tree, std::vector<std::string> &args)
{
	for (auto &child : tree->GetCurrent()->GetChilds())
	{
		printf("%s\t", child->IsDir() ? "DIR" : "FILE");
		printf("%s\t", child->GetName());
		printf("%ld\t", child->GetSize());

		auto modificationDate = child->GetLastModification();
		printf("%s", asctime(gmtime(&modificationDate)));
	}
}

void upload(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(1);
	char cwd[FILENAME_MAX];
	getcwd(cwd, sizeof(cwd));
	for (const auto &entry : fs::directory_iterator(cwd))
	{
		auto name = entry.path().filename().string();
		if (name != args.front())
			continue;

		// TODO!!!
		if (entry.is_directory())
			break;
		tree->CreateNode(name.c_str(), false);
	}

	tree->Save();
}

void mkdir(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(1);
	tree->CreateNode(args.front().c_str(), true);
}

void rmdir(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(1);
	if (auto node = tree->GetCurrent()->FindChild(args.front().c_str()); node != nullptr)
	{
		if (!node->IsDir())
		{
			printf("-bash: rmdir: %s: Not a directory. Use `rm` instead.\n", args.front().c_str());
			return;
		}

		if (node->GetChilds().size() > 0)
		{
			printf("-bash: rmdir: %s: Not empty.\n", args.front().c_str());
			return;
		}
		tree->GetCurrent()->RemoveChild(node->GetId());
		tree->DecrementLength();
		tree->Save();
	}
}

void rm(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(1);
	if (auto node = tree->GetCurrent()->FindChild(args.front().c_str()); node != nullptr)
	{
		if (node->IsDir())
		{
			printf("-bash: rm: %s: Is a directory. Use `rmdir` instead.\n", args.front().c_str());
			return;
		}

		tree->GetCurrent()->RemoveChild(node->GetId());
		tree->DecrementLength();
		tree->Save();
	}
}

void touch(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(1);
	tree->CreateNode(args.front().c_str(), false);
}

void mv(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(2);
	if (auto nodeToMove = tree->GetCurrent()->FindChild(args.front().c_str()); nodeToMove != nullptr)
	{
		// copying destination -> inside existent directory
		auto destination = args.back();

		bool destinationIsFolder = false;
		// remove slash
		if (destination.find('/') != std::string::npos)
		{
			destination = destination.substr(0, destination.find('/'));
			destinationIsFolder = true;
		}

		auto destinationNode = tree->GetCurrent()->FindChild(destination.c_str());
		if (destinationNode == nullptr || !destinationNode->IsDir())
		{
			if (destinationIsFolder)
			{
				printf("-bash: mv: cannot move '%s' to '%s': Not a directory\n", nodeToMove->GetName(), destination.c_str());
				return;
			}
			nodeToMove->Rename(destination.c_str());
		}
		else if (destinationNode->IsDir())
		{
			tree->GetCurrent()->RemoveChild(nodeToMove->GetId());
			nodeToMove->SetParent(destinationNode);
			destinationNode->AddChild(nodeToMove);
		}
		tree->Save();
	}
}

void cp(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(2);
	if (auto nodeToCopy = tree->GetCurrent()->FindChild(args.front().c_str()); nodeToCopy != nullptr)
	{
		// copying destination -> inside existent directory
		auto destination = args.back();

		bool destinationIsFolder = false;
		// remove slash
		if (destination.find('/') != std::string::npos)
		{
			destination = destination.substr(0, destination.find('/'));
			destinationIsFolder = true;
		}

		auto destinationNode = tree->GetCurrent()->FindChild(destination.c_str());
		if (destinationNode == nullptr || !destinationNode->IsDir())
		{
			if (destinationIsFolder)
			{
				// destination folder doesn't exist
				tree->CreateNode(destination.c_str(), true);
				tree->ChangeCurrent(destination.c_str());
				tree->CopyNode(*nodeToCopy, nodeToCopy->GetName());
				tree->ChangeCurrent(".."); // vuelve al directorio anterior
			}
			else
			{
				tree->CopyNode(*nodeToCopy, destination.c_str());
			}
		}
		else if (destinationNode->IsDir())
		{
			tree->ChangeCurrent(destinationNode->GetName());
			tree->CopyNode(*nodeToCopy, nodeToCopy->GetName());
			tree->ChangeCurrent(".."); // vuelve al directorio anterior
		}

		tree->Save();
	}
}

void lpwd(Tree *tree, std::vector<std::string> &args)
{
	char cwd[FILENAME_MAX];
	getcwd(cwd, sizeof(cwd));
	printf("%s\n", cwd);
}

void lcd(Tree *tree, std::vector<std::string> &args)
{
	REQUIRED_ARGS(1);
	if (chdir(args.front().c_str()) == -1)
	{
		printf("-bash: lcd: %s: Not a directory\n", args.front().c_str());
		return;
	}
}

void lls(Tree *tree, std::vector<std::string> &args)
{
	// TODO: fix the format | file names have different sizes
	char cwd[FILENAME_MAX];
	getcwd(cwd, sizeof(cwd));
	for (const auto &entry : fs::directory_iterator(cwd))
	{
		printf("%s\t", entry.is_directory() ? "DIR" : "FILE");
		printf("%s\t", entry.path().filename().string().c_str());
		auto size = entry.is_directory() ? 4096 : static_cast<long>(entry.file_size());
		printf("%ld\t", size);

		struct stat result;
		if (stat(entry.path().string().c_str(), &result) == 0)
			printf("%s", asctime(gmtime(&result.st_mtime)));
	}
}
} // namespace chank