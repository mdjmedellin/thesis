#include <iostream>
#include "NeuralNet.hpp"
#include "Residue.hpp"
#include "ProteinUtilities.hpp"

void AnalyzePrediction(GHProtein::ESecondaryStructure predictedStructure,
	GHProtein::ESecondaryStructure expectedStructure,
	std::vector<int>& correctPredictionCounters,
	std::vector<int>& incorrectPredictionCounters,
	std::vector<int>& wrongfullyPredictedCounters)
{
	if (predictedStructure == expectedStructure)
	{
		//correct prediction
		correctPredictionCounters[expectedStructure] += 1;
	}
	else
	{
		//patterns incorrectly assigned to not the expected structure
		incorrectPredictionCounters[expectedStructure] += 1;

		//patterns incorrectly assigned to the predicted structure
		wrongfullyPredictedCounters[predictedStructure] += 1;
	}
}

double CalculateQ3(const std::vector<int>& correctPredictionCounters,
	int totalResidues)
{
	//add up the total number of correct prediction
	int totalCorrectPredictions = 0;
	for (int i = 0; i < correctPredictionCounters.size(); ++i)
	{
		totalCorrectPredictions += correctPredictionCounters[i];
	}

	double q3Rate = double(totalCorrectPredictions) / totalResidues;
	return q3Rate;
}

void CalculateCorrelationCoefficients(std::vector<double>& out_correlationCoefficients,
	const std::vector<int>& correctPredictionCounter,
	const std::vector<int>& incorrectPredictionCounter,
	const std::vector<int>& wrongfullyPredictedCounter)
{
	int totalCorrectPredictions = 0;
	for (int i = 0; i < correctPredictionCounter.size(); ++i)
	{
		totalCorrectPredictions += correctPredictionCounter[i];
	}

	int p = 0;
	int n = 0;
	int o = 0;
	int u = 0;

	int pn = 0;
	int ou = 0;

	int nAu = 0;
	int nAo = 0;
	int pAu = 0;
	int pAo = 0;

	int dividend = 0;
	double divisor = 0.0;

	for (int i = 0; i < GHProtein::ESecondaryStructure::ssCount; ++i)
	{
		p = correctPredictionCounter[i];
		n = totalCorrectPredictions - p;
		o = wrongfullyPredictedCounter[i];
		u = incorrectPredictionCounter[i];

		pn = p * n;
		ou = o * u;

		nAu = n + u;
		nAo = n + o;
		pAu = p + u;
		pAo = p + o;

		dividend = pn - ou;
		divisor = double(nAu * nAo * pAu * pAo);
		if (divisor > 0.0)
		{
			divisor = sqrt(divisor);

			out_correlationCoefficients[i] = dividend / divisor;
		}
		else
		{
			out_correlationCoefficients[i] = dividend / 1.0;
		}
	}
}

int main(int argc, char* argv[])
{
	int MAX_TRAINING = 10;

	//open the file we are going to write our test results into
	std::ofstream resultsDataFile;
	resultsDataFile.open("FINDTESTRESULTS.txt");

	GHProtein::TrainingData trainData("tmp/trainingData.txt");
	GHProtein::NeuralNet* testNet = new GHProtein::NeuralNet(trainData.GetTopology(), true);
	GHProtein::NeuralNet* testFilter = nullptr;

	//check if the filter neural network is valid
	if (trainData.GetFilterTopology().size() > 1)
	{
		testFilter = new GHProtein::NeuralNet(trainData.GetTopology(), true);
	}

	std::vector< std::vector<double> > inputVals;
	std::vector< std::vector< double > > resultsHolder;
	std::vector< std::vector<double> > expectedResultsHolder;
	std::vector< double > currentResults;
	std::vector< double > outputVals;
	int trainingDataLength = 0;
	int numberOfSets = trainData.GetNumberOfSets();
	int filesInSet = 0;
	const GHProtein::NeuralNetDataSet* currentDataSet = nullptr;

	int cyclesBeforeLastTest = 1;
	const int CYCLES_BETWEEN_TESTS = 1;

	for (int trainingPass = 0; trainingPass < MAX_TRAINING; ++trainingPass, ++cyclesBeforeLastTest)
	{
		//iterate over all the sets
		for (int i = 0; i < numberOfSets - 1; ++i)
		{
			currentDataSet = trainData.GetDataSetAtSpecifiedIndex(i);
			filesInSet = currentDataSet->GetNumberOfFilesInSet();

			//iterate over all the files in the set
			for (int j = 0; j < filesInSet; ++j)
			{
				resultsHolder.clear();
				trainingDataLength = currentDataSet->GetSizeOfTrainingDataAtSpecifiedIndex(j);

				//now that we know the set, the file, and the size of the training data
				//we iterate over all of the amino acids and train our neural network
				for (int residueIndex = 0; residueIndex < trainingDataLength; ++residueIndex)
				{
					currentDataSet->GetInputValues(j, residueIndex, inputVals, testNet->GetNumberOfRequiredInputs());
					testNet->FeedForward(inputVals);
					//save the current results
					testNet->GetResults(currentResults);
					resultsHolder.push_back(currentResults);

					currentDataSet->GetOutputValues(j, residueIndex, outputVals);
					expectedResultsHolder.push_back(outputVals);
					testNet->BackPropagation(outputVals);
				}

				//now feed the network for the filter if there is a filter neural network
				if (testFilter != nullptr)
				{
					int requiredInputs = testFilter->GetNumberOfRequiredInputs();
					int windowWidth = (requiredInputs - 1) * .5f;
					std::vector<double> emptyResults(3, 0.0);

					for (int currentPredictionIndex = 0; currentPredictionIndex < trainingDataLength; ++currentPredictionIndex)
					{
						inputVals.clear();

						for (int currentIndex = currentPredictionIndex - windowWidth;
							currentIndex <= currentPredictionIndex + windowWidth;
							++currentIndex)
						{
							if (currentIndex < 0
								|| currentIndex >= trainingDataLength)
							{
								inputVals.push_back(emptyResults);
							}
							else
							{
								inputVals.push_back(resultsHolder[currentIndex]);
							}
						}

						//now that i have the inputs, it is time to feed the neural network
						testFilter->FeedForward(inputVals);
						testFilter->BackPropagation(expectedResultsHolder[currentPredictionIndex]);
					}
				}
			}
		}

		//test how good the network is doing
		if (cyclesBeforeLastTest >= CYCLES_BETWEEN_TESTS)
		{
			resultsDataFile << "TRAINING PASS # " << trainingPass << "\n";

			currentDataSet = trainData.GetDataSetAtSpecifiedIndex(numberOfSets - 1);
			filesInSet = currentDataSet->GetNumberOfFilesInSet();
			std::vector<char> secondaryStructureResults;
			std::vector<double> predictionResults;
			std::vector<double> correlationCoefficients(3, 0.0);
			std::vector<int> correctPredictionCounters(3,0);
			std::vector<int> incorrectPredictionCounters(3,0);
			std::vector<int> wrongfullyPredictedCounters(3,0);
			GHProtein::ESecondaryStructure predictedStructure;
			GHProtein::ESecondaryStructure expectedStruccture;

			double q3Rate = 0.0;
			double q3AllCoils = 0.0;

			for (int j = 0; j < filesInSet; ++j)
			{
				//before filtering
				resultsDataFile << "BEFORE FILTERING\n";

				trainingDataLength = currentDataSet->GetSizeOfTrainingDataAtSpecifiedIndex(j);

				//reset all counters
				std::fill(correctPredictionCounters.begin(), correctPredictionCounters.end(), 0);
				std::fill(incorrectPredictionCounters.begin(), incorrectPredictionCounters.end(), 0);
				std::fill(wrongfullyPredictedCounters.begin(), wrongfullyPredictedCounters.end(), 0);
				resultsHolder.clear();
				expectedResultsHolder.clear();

				//now that we know the set, the file, and the size of the training data
				//we iterate over all of the amino acids and train our neural network
				for (int residueIndex = 0; residueIndex < trainingDataLength; ++residueIndex)
				{
					currentDataSet->GetInputValues(j, residueIndex, inputVals, testNet->GetNumberOfRequiredInputs());
					testNet->FeedForward(inputVals);

					testNet->GetResults(predictionResults);
					resultsHolder.push_back(predictionResults);
					
					currentDataSet->GetOutputValues(j, residueIndex, outputVals);
					expectedResultsHolder.push_back(outputVals);

					predictedStructure = GHProtein::Residue::VectorToSecondaryStructureType(predictionResults);
					expectedStruccture = GHProtein::Residue::VectorToSecondaryStructureType(outputVals);

					AnalyzePrediction(predictedStructure, expectedStruccture,
						correctPredictionCounters, incorrectPredictionCounters,
						wrongfullyPredictedCounters);
				}

				q3Rate = CalculateQ3(correctPredictionCounters, trainingDataLength);
				CalculateCorrelationCoefficients(correlationCoefficients, correctPredictionCounters,
					incorrectPredictionCounters, wrongfullyPredictedCounters);

				//calculate the q3 rate of all coils
				q3AllCoils = double(correctPredictionCounters[GHProtein::ESecondaryStructure::ssLoop] +
					incorrectPredictionCounters[GHProtein::ESecondaryStructure::ssLoop]) / trainingDataLength;

				resultsDataFile << "Q3 Rate = " << q3Rate << "\n";
				resultsDataFile << "Q3 All Coils = " << q3AllCoils << "\n";
				GHProtein::PrintVectorVals("COEFFICIENTS:", correlationCoefficients, resultsDataFile);

				//now we filter
				if (testFilter != nullptr)
				{
					resultsDataFile << "AFTER FILTERING\n";

					std::fill(correctPredictionCounters.begin(), correctPredictionCounters.end(), 0);
					std::fill(incorrectPredictionCounters.begin(), incorrectPredictionCounters.end(), 0);
					std::fill(wrongfullyPredictedCounters.begin(), wrongfullyPredictedCounters.end(), 0);

					int requiredInputs = testFilter->GetNumberOfRequiredInputs();
					int windowWidth = (requiredInputs - 1) * .5f;
					std::vector<double> emptyResults(3, 0.0);

					for (int currentPredictionIndex = 0; currentPredictionIndex < trainingDataLength; ++currentPredictionIndex)
					{
						inputVals.clear();

						for (int currentIndex = currentPredictionIndex - windowWidth;
							currentIndex <= currentPredictionIndex + windowWidth;
							++currentIndex)
						{
							if (currentIndex < 0
								|| currentIndex >= trainingDataLength)
							{
								inputVals.push_back(emptyResults);
							}
							else
							{
								inputVals.push_back(resultsHolder[currentIndex]);
							}
						}

						//now that i have the inputs, it is time to feed the neural network
						testFilter->FeedForward(inputVals);
						testFilter->BackPropagation(expectedResultsHolder[currentPredictionIndex]);

						testFilter->GetResults(predictionResults);

						predictedStructure = GHProtein::Residue::VectorToSecondaryStructureType(predictionResults);
						expectedStruccture = GHProtein::Residue::VectorToSecondaryStructureType(expectedResultsHolder[currentPredictionIndex]);

						AnalyzePrediction(predictedStructure, expectedStruccture,
							correctPredictionCounters, incorrectPredictionCounters,
							wrongfullyPredictedCounters);
					}

					q3Rate = CalculateQ3(correctPredictionCounters, trainingDataLength);
					CalculateCorrelationCoefficients(correlationCoefficients, correctPredictionCounters,
						incorrectPredictionCounters, wrongfullyPredictedCounters);

					//calculate the q3 rate of all coils
					q3AllCoils = double(correctPredictionCounters[GHProtein::ESecondaryStructure::ssLoop] + 
						incorrectPredictionCounters[GHProtein::ESecondaryStructure::ssLoop]) / trainingDataLength;

					resultsDataFile << "Q3 Rate = " << q3Rate << "\n";
					resultsDataFile << "Q3 All Coils = " << q3AllCoils << "\n";
					GHProtein::PrintVectorVals("COEFFICIENTS:", correlationCoefficients, resultsDataFile);
				}
			}

			resultsDataFile << "\n\n";

			cyclesBeforeLastTest = 0;
		}
	}

	testNet->SaveWeights("WEIGHTS.txt");
	if (testFilter != nullptr)
	{
		testFilter->SaveWeights("FILTER_WEIGHTS.txt");
	}
	resultsDataFile.close();
}








/*
//Backpropagation, 25x25x8 units, binary sigmoid function network
//Written by Thomas Riga, University of Genoa, Italy
//thomas@magister.magi.unige.it
*/