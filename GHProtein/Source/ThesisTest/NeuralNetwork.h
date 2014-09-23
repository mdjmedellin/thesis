#pragma once
#include "NeuralNetwork.generated.h"

USTRUCT()
struct FNeuronConnection
{
	GENERATED_USTRUCT_BODY()

public:
	FNeuronConnection(int numberOfWeights = 1);
	void RandomizeWeights();
	void SetDeltaWeight(double deltaWeight, int deltaWeightIndex);
	void AddToWeight(double amountToAdd, int weightIndex);

private:
	static double S_RandomWeight(void) { return rand() / double(RAND_MAX); }

public:
	TArray<double> m_weight;
	TArray<double> m_deltaWeight;
};

struct FNeuronLayer;

USTRUCT()
struct FNeuron
{
	GENERATED_USTRUCT_BODY()

public:
	FNeuron();
	FNeuron(int numberOfOutputs, int myIndex, int numberOfinputs = 1);
	void SetOutputValues(TArray<double>& values);
	void SetOutputValue(double value, int index = 0);
	double GetOutputValue() const;
	double GetDeltaWeight(int connectionIndex) const;
	double GetWeight(int connectionIndex) const;
	int GetIndexOfValidOutputValue() const;
	void FeedForward(const FNeuronLayer &prevLayer);
	void CalculateOutputGradients(double targetVal);
	void CalculateHiddenGradients(const FNeuronLayer &nextLayer);
	void UpdateInputWeights(FNeuronLayer &prevLayer);

private:
	static double s_eta;   // [0.0..1.0] overall net training rate
	static double s_alpha; // [0.0..n] multiplier of last weight change (momentum)
	static double S_TransferFunction(double x);		//this is the function we use to smoot the curve. For example the sigmoid function
	static double S_TransferFunctionDerivative(double x);
	double SumDOW(const FNeuronLayer &nextLayer) const;
	TArray<double> m_outputVals;
	TArray<FNeuronConnection> m_outputConnections;
	unsigned m_myIndex;
	double m_gradient;
};

USTRUCT()
struct FNeuronLayer
{
	GENERATED_USTRUCT_BODY()

	int m_numberOfNeurons;
	TArray<FNeuron> m_neurons;

	//constructor
	FNeuronLayer();
};

/**
 * 
 */
class THESISTEST_API NeuralNetwork
{
public:
	NeuralNetwork(const TArray<int>& topology);
	~NeuralNetwork();

	void FeedForward(const TArray<double>& inputValues);			//this is our update function, it is the one that passes the input to the network
	void BackPropagation(const TArray<double>& targetValues);
	void GetResults(TArray<double>& resultValues)const;
	double GetRecentAverageError() const { return m_recentAverageError; }

private:
	//private functions

public:
	//public data members

private:
	//private data members
	int m_numberOfInputs;
	int m_numberOfOutputs;
	int m_numberOfHiddenLayers;

	TArray<FNeuronLayer> m_neuronLayers;
	double m_error;
	double m_recentAverageError;
	static double m_recentAverageSmoothingFactor;
};


/*
// Silly class to read training data from a text file -- Replace This.
// Replace class TrainingData with whatever you need to get input data into the
// program, e.g., connect to a database, or take a stream of data from stdin, or
// from a file specified by a command line argument, etc.

class TrainingData
{
public:
	TrainingData(const string filename);
	bool isEof(void) { return m_trainingDataFile.eof(); }
	void getTopology(vector<unsigned> &topology);

	// Returns the number of input values read from the file:
	unsigned getNextInputs(vector<double> &inputVals);
	unsigned getTargetOutputs(vector<double> &targetOutputVals);

private:
	ifstream m_trainingDataFile;
};

void TrainingData::getTopology(vector<unsigned> &topology)
{
	string line;
	string label;

	getline(m_trainingDataFile, line);
	stringstream ss(line);
	ss >> label;
	if (this->isEof() || label.compare("topology:") != 0) {
		abort();
	}

	while (!ss.eof()) {
		unsigned n;
		ss >> n;
		topology.push_back(n);
	}

	return;
}

TrainingData::TrainingData(const string filename)
{
	m_trainingDataFile.open(filename.c_str());
}

unsigned TrainingData::getNextInputs(vector<double> &inputVals)
{
	inputVals.clear();

	string line;
	getline(m_trainingDataFile, line);
	stringstream ss(line);

	string label;
	ss >> label;
	if (label.compare("in:") == 0) {
		double oneValue;
		while (ss >> oneValue) {
			inputVals.push_back(oneValue);
		}
	}

	return inputVals.size();
}

unsigned TrainingData::getTargetOutputs(vector<double> &targetOutputVals)
{
	targetOutputVals.clear();

	string line;
	getline(m_trainingDataFile, line);
	stringstream ss(line);

	string label;
	ss >> label;
	if (label.compare("out:") == 0) {
		double oneValue;
		while (ss >> oneValue) {
			targetOutputVals.push_back(oneValue);
		}
	}

	return targetOutputVals.size();
}


void showVectorVals(string label, vector<double> &v)
{
	cout << label << " ";
	for (unsigned i = 0; i < v.size(); ++i) {
		cout << v[i] << " ";
	}

	cout << endl;
}


int main()
{
	TrainingData trainData("/tmp/trainingData.txt");

	// e.g., { 3, 2, 1 }
	vector<unsigned> topology;
	trainData.getTopology(topology);

	Net myNet(topology);

	vector<double> inputVals, targetVals, resultVals;
	int trainingPass = 0;

	while (!trainData.isEof()) {
		++trainingPass;
		cout << endl << "Pass " << trainingPass;

		// Get new input data and feed it forward:
		if (trainData.getNextInputs(inputVals) != topology[0]) {
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

	cout << endl << "Done" << endl;
}
*/