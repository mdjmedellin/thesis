#include <iostream>
#include "NeuralNet.hpp"

int main(int argc, char* argv[])
{
	GHProtein::TrainingData trainData("tmp/trainingData.txt");

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