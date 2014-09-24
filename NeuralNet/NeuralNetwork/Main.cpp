#include <iostream>
#include "NeuralNet.hpp"

int main(int argc, char* argv[])
{
	int MAX_TRAINING = 2000;

	GHProtein::TrainingData trainData("tmp/testTraining.txt");
	GHProtein::NeuralNet* testNet = new GHProtein::NeuralNet(trainData.GetTopology(), true);

	std::vector< const GHProtein::IrisData* > randomTrainingData;
	std::vector< const GHProtein::IrisData* > randomValidationData;

	std::vector< std::vector<double> >inputVals;
	int trainingDataLength = 0;

	for (int trainingPass = 0; trainingPass < MAX_TRAINING; ++trainingPass)
	{
		trainData.GetRandomTrainingData(randomTrainingData);
		trainData.GetRandomValidationData(randomValidationData);

		//now that we have the randomized sets, lets train the neural network
		trainingDataLength = randomTrainingData.size();
		for (int i = 0; i < trainingDataLength; ++i)
		{
			randomTrainingData[i]->GetInputValues(inputVals);

		}
	}
	// e.g., { 3, 2, 1 }
	//gh::NeuralNet* testNet = new gh::NeuralNet(topology, true);

	/*
	Net myNet(topology);

	vector<double> inputVals, targetVals, resultVals;
	int trainingPass = 0;

	while (!trainData.isEof())
	{
		++trainingPass;
		cout << endl << "Pass " << trainingPass;

		// Get new input data and feed it forward:
		if (trainData.getNextInputs(inputVals) != topology[0])
		{
			break;
		}
		showVectorVals(": Inputs:", inputVals);
		myNet.feedForward(inputVals);

		// Collect the net's actual output results:
		myNet.getResults(resultVals);
		showVectorVals("Outputs:", resultVals);

		// Train the net what the outputs should have been:
		trainData.getTargetOutputs(targetVals);
		showVectorVals("Targets:", targetVals);
		assert(targetVals.size() == topology.back());

		myNet.backProp(targetVals);

		// Report how well the training is working, average over recent samples:
		cout << "Net recent average error: "
			<< myNet.getRecentAverageError() << endl;
	}
	*/

	std::cout << std::endl << "Done" << std::endl;

	int x = 1;
}