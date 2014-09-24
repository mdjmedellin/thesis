#pragma once
#ifndef NEURAL_NET_H
#define NEURAL_NET_H

#include <vector>
#include <fstream>

namespace GHProtein
{
	class ProteinBuilder;
	class ProteinModel;

	struct IrisData
	{
	public:
		IrisData();
		~IrisData();
		void SetInputData(float input_0, float input_1, float input_2, float input_3);
		void SetOutputData(float output_0, float output_1, float output_2);

	private:
		float* m_inputData;
		float* m_expectedOutput;
		std::vector<float> m_inputs;
		std::vector<float> m_outputs;
	};

	struct NeuralNetDataSet
	{
	public:
		NeuralNetDataSet();
		void SetDirectory(const std::string& directoryName);
		void AddFile(const std::string& fileName);
		void LoadProteinModels(ProteinBuilder* proteinBuilder, const std::string& dataRootLocation);
		void LoadIrisData(const std::string& dataRootLocation);

	private:
		std::string m_directory;
		std::vector<std::string> m_files;
		std::vector<ProteinModel*> m_proteinModels;
		std::vector<IrisData*> m_irisDataContainer;
	};

	struct NeuralNetData
	{
	public:
		NeuralNetData();
		void SetRootLocation(const std::string& rootLocation);
		std::string GetRootLocation();
		void AddDataSet(const NeuralNetDataSet& dataSet);
		IrisData*

	private:
		std::string m_rootLocation;
		std::vector<NeuralNetDataSet> m_dataSets;
	};

	struct FNeuronConnection
	{
	public:
		FNeuronConnection(int numberOfWeights = 1);
		void RandomizeWeights();
		void SetDeltaWeight(double deltaWeight, int deltaWeightIndex);
		void AddToWeight(double amountToAdd, int weightIndex);

	private:


	public:
		std::vector<double> m_weight;
		std::vector<double> m_deltaWeight;
	};

	struct FNeuronLayer;

	struct FNeuron
	{
	public:
		FNeuron();
		FNeuron(int numberOfOutputs, int myIndex, int numberOfinputs = 1);
		void SetOutputValues(const std::vector<double>& values);
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
		std::vector<double> m_outputVals;
		std::vector<FNeuronConnection> m_outputConnections;
		unsigned m_myIndex;
		double m_gradient;
	};

	struct FNeuronLayer
	{
		int m_numberOfNeurons;
		std::vector<FNeuron> m_neurons;

		//constructor
		FNeuronLayer(int numberOfNeurons, int numberOfOutputs, int numberOfInputs, bool addBiasNeuron);
	};

	/**
	*
	*/
	class NeuralNet
	{
	public:
		NeuralNet(const std::vector< std::pair< int, int > >& topology, bool addBiasNeuron = false);
		~NeuralNet();

		void FeedForward(const std::vector< std::vector<double> >& inputValues);			//this is our update function, it is the one that passes the input to the network
		void BackPropagation(const std::vector<double>& targetValues);
		void GetResults(std::vector<double>& resultValues)const;
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

		std::vector<FNeuronLayer*> m_neuronLayers;
		double m_error;
		double m_recentAverageError;
		static double m_recentAverageSmoothingFactor;
	};

	class TrainingData
	{
	public:
		TrainingData(const std::string filename);
		~TrainingData();
		bool isEOF(void) { return m_trainingDataFile.eof(); }
		std::vector< std::pair<int, int> > GetTopology() { return m_topology; }
		void GetRandomTrainingData(std::vector<IrisData>& out_container);
		void GetRandomValidationData(std::vector<IrisData>& out_container);

	private:
		void LoadTopology();
		void GetTrainingData();
		void LoadTrainingData();
		void LoadIrisDataSet();
		void LoadDataSet();

	private:
		std::ifstream m_trainingDataFile;
		std::vector< std::pair<int, int> > m_topology;
		NeuralNetData m_data;
		ProteinBuilder* m_proteinBuilder;
	};
}

#endif

/*
// Silly class to read training data from a text file -- Replace This.
// Replace class TrainingData with whatever you need to get input data into the
// program, e.g., connect to a database, or take a stream of data from stdin, or
// from a file specified by a command line argument, etc.




int main()
{
	TrainingData trainData("/tmp/trainingData.txt");

	// e.g., { 3, 2, 1 }
	vector<unsigned> topology;
	trainData.getTopology(topology);

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

	cout << endl << "Done" << endl;
}
*/