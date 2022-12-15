#pragma once

#include <functional>
#include <string>

template<typename T>
class ExpandBranch;

template<typename T>
using ExpandTree = ExpandBranch<T>;

template<typename T>
class ExpandBranch
{
public:
	using walk_callback_t = std::function<bool(ExpandBranch<T>*)>;
	using deserializer_t = std::function<T(const std::string&)>;
private:
	int mDepth;
	T* mNodes;
	ExpandBranch<T>* mNext;
public:
	ExpandBranch()
	{
		mDepth = 0;
		mNodes = new T[mDepth + 1];
		mNext = nullptr;
	}

	ExpandBranch(T value)
	{
		mDepth = 0;
		mNodes = new T[mDepth + 1]{ value };
		mNext = nullptr;
	}

	ExpandBranch(T* values)
	{
		mDepth = 0;
		mNodes = values;
		mNext = nullptr;
	}

	~ExpandBranch()
	{
		delete[] mNodes;
		if (mNext != nullptr)
		{
			delete mNext;
		}
	}
public:
	void SetNext(ExpandBranch<T>* next)
	{
		mNext = next;

		mNext->mDepth = mDepth + 1;

		if (mNext->mNodes != nullptr)
		{
			delete[] mNext->mNodes;
		}

		mNext->mNodes = new T[mNext->mDepth + 1];
	}

	ExpandBranch<T>* GetNext()
	{
		return mNext;
	}

	T GetNode(int index)
	{
		return mNodes[index];
	}

	void SetNode(int index, T value)
	{
		mNodes[index] = value;
	}

	int GetDepth()
	{
		return mDepth;
	}

	int GetNodesAmount()
	{
		return mDepth + 1;
	}
public:
	size_t GetByteSize()
	{
		int result = 0;

		Walk([&result](ExpandBranch<T>* leaf) -> bool {
			result += sizeof(*leaf);

			return false;
		});

		return result;
	}
public:
	void Walk(walk_callback_t walker, bool includeSelf = true)
	{
		ExpandBranch<T>* currentBranch = this;
		if (!includeSelf)
		{
			currentBranch = mNext;
		}

		while (currentBranch != nullptr)
		{
			bool shouldStop = walker(currentBranch);
			if (shouldStop)
			{
				break;
			}

			currentBranch = currentBranch->mNext;
		}
	}
public:
	void Serialize(std::ostream& stream, int skipDeep = -1, bool pretty = false)
	{
		Walk([&](ExpandBranch<T>* branch) -> bool {
			if (!pretty)
			{
				stream << "R" << std::endl;
			}

			for (int n = 0; n < branch->GetNodesAmount(); n++)
			{
				stream << branch->GetNode(n);

				if (pretty)
				{
					stream << ' ';
				}
				else
				{
					stream << std::endl;
				}
			}

			if (skipDeep != -1 && branch->GetDepth() > skipDeep)
			{
				stream << std::endl << "..." << std::endl;

				return true;
			}

			if(pretty)
			{
				stream << std::endl;
			}

			return false;
		});
	}

	static void Deserialize(std::istream& stream, ExpandBranch<T>** output, deserializer_t valueDeserializer)
	{
		ExpandBranch<T>** generationCandidate = output;
		ExpandBranch<T>* parent = nullptr;

		int inode = 0;
		
		std::string curline = " ";
		while (stream.good() && !stream.eof())
		{
			std::getline(stream, curline);
			if (curline.size() <= 0)
			{
				continue;
			}
			else if(curline == "R")
			{
				if (parent != nullptr)
				{
					generationCandidate = (&parent->mNext);
				}

				(*generationCandidate) = new ExpandBranch<T>();
				if (parent != nullptr)
				{
					parent->SetNext(*generationCandidate);
				}

				inode = 0;
				parent = (*generationCandidate);

				continue;
			}

			(*generationCandidate)->SetNode(inode++, valueDeserializer(curline));
		}
	}
};

template<typename T>
using SSSMatrix = ExpandTree<T>;

template<typename T>
using SSSMatrixRow = ExpandBranch<T>;