#include <iostream>
#include <unordered_set>
#include <fstream>

#include "profile.hpp"

#include "matrix.hpp"

SSSMatrix<int>* GenerateMatrix(int maxBranches, int maxValue)
{
	srand(time(NULL));

	const int zeroChance = 60;

	SSSMatrix<int>* matrix = new SSSMatrix<int>(1);

	SSSMatrixRow<int>* currentParent = matrix;
	for (int b = 0; b < maxBranches; b++)
	{
		SSSMatrixRow<int>* generatedRow = new SSSMatrixRow<int>(nullptr);
		currentParent->SetNext(generatedRow);

		for (int n = 0; n < generatedRow->GetNodesAmount(); n++)
		{
			int value = 0;
			int shouldBeZero = (rand() % 100) > zeroChance;

			if (!shouldBeZero)
			{
				value = rand() % (maxValue + 1);
			}

			generatedRow->SetNode(n, value);
		}

		currentParent = generatedRow;
	}

	return matrix;
}

int main(int argc, const char** argv)
{
	std::ifstream input = std::ifstream("matrix.sss");
	std::ofstream output;

	SSSMatrix<int>* matrix = nullptr;

	int N = 6;
	int E = 32;

	if (input.is_open())
	{
		profile::StartMemoryProfiling();
		profile::StartTimeProfiling();

		SSSMatrix<int>::Deserialize(input, &matrix, [](const std::string& value) -> int {
			return std::stoi(value);
		});


		profile::EndTimeProfiling();
		profile::EndMemoryProfiling();

		std::cout << "1. Deserialization (loading from file) took " << profile::GetProfiledTime().count() << " microseconds." << std::endl;
		std::cout << "\t with " << profile::GetProfiledMemory() << " bytes of memory allocated in total" << std::endl << std::endl;
	}
	else
	{
		std::cout << "Enter max amount of rows: " << std::endl;
		std::cin >> N;

		std::cout << "Enter max value of element: " << std::endl;
		std::cin >> E;

		profile::StartMemoryProfiling();
		profile::StartTimeProfiling();

		matrix = GenerateMatrix(N, E);

		profile::EndTimeProfiling();
		profile::EndMemoryProfiling();

		std::cout << "1. Generation took " << profile::GetProfiledTime().count() << " microseconds." << std::endl;
		std::cout << "\t with " << profile::GetProfiledMemory() << " bytes of memory allocated in total" << std::endl << std::endl;

		output = std::ofstream("matrix.sss");
	}

	profile::StartMemoryProfiling();
	profile::StartTimeProfiling();

	std::unordered_set<int> unique = {};
	matrix->Walk([&unique, E](SSSMatrixRow<int>* row) -> bool {
		for (int n = 0; n < row->GetNodesAmount(); n++)
		{
			int value = row->GetNode(n);
			if (unique.find(value) == unique.end())
			{
				unique.insert(value);
			}

			if (unique.size() == E)
			{
				return true;
			}
		}

		return false;
	});

	profile::EndTimeProfiling();
	profile::EndMemoryProfiling();

	int printRowAmount = std::min(N, 6);
	int** printMatrixRows = new int*[printRowAmount];
	for (int i = 0; i < printRowAmount; i++)
	{
		printMatrixRows[i] = new int[printRowAmount];
	}

	matrix->Walk([&printMatrixRows, printRowAmount](SSSMatrixRow<int>* row) -> bool {
		if (row->GetDepth() == printRowAmount)
		{
			return true;
		}
		
		for (int n = 0; n < row->GetNodesAmount(); n++)
		{
			printMatrixRows[n][row->GetDepth()] = printMatrixRows[row->GetDepth()][n] = row->GetNode(n);
		}

		return false;
	});

	std::cout << "Matrix: " << std::endl;

	for (int y = 0; y < printRowAmount; y++)
	{
		for (int x = 0; x < printRowAmount; x++)
		{
			std::cout << printMatrixRows[y][x] << " ";
		}

		std::cout << std::endl;

		delete[] printMatrixRows[y];
	}

	delete[] printMatrixRows;

	if (N > printRowAmount)
	{
		std::cout << "..." << std::endl;
	}

	std::cout << "Found unique values - " << unique.size() <<  ": " << std::endl;
	for (int nunique : unique)
	{
		std::cout << nunique << ' ';
	}

	std::cout << std::endl;

	std::cout << "2. Search took " << profile::GetProfiledTime().count() << " microseconds." << std::endl;
	std::cout << "Entire matrix takes up " << matrix->GetByteSize() << " bytes of memory." << std::endl;
	std::cout << profile::GetProfiledMemory() / 1000 << " kb of memory was used in total." << std::endl;

	if (output.is_open())
	{
		profile::StartMemoryProfiling();
		profile::StartTimeProfiling();

		matrix->Serialize(output);

		profile::EndTimeProfiling();
		profile::EndMemoryProfiling();

		std::cout << "3. Serialization (writing to file) took " << profile::GetProfiledTime().count() << " microseconds." << std::endl;
		std::cout << "\t with " << profile::GetProfiledMemory() << " bytes of memory allocated in total" << std::endl << std::endl;

		output.close();
	}

	delete matrix;

	return 0;
}
