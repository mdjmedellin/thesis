#pragma once
#ifndef NEURAL_NET_H
#define NEURAL_NET_H

#include <vector>
#include <fstream>

namespace GHProtein
{
	enum NetDataType
	{
		NET_DATA_NONE,
		NET_DATA_IRIS,
		NET_DATA_PROTEIN,
		NET_DATA_COUNT
	};

	class ProteinBuilder;
	class ProteinModel;

	struct IrisData
	{
	public:
		IrisData();
		~IrisData();
		void SetInputData(float input_0, float input_1, float input_2, float input_3);
		void SetOutputData(float output_0, float output_1, float output_2);
		void GetInputValues(std::vector< std::vector<double> >& out_inputs) const;
		void GetOutputValues(std::vector<double>& outputs) const;

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
		const IrisData* GetIrisData(int dataIndex) const;
		int GetNumberOfFilesInSet() const;
		int GetSizeOfTrainingDataAtSpecifiedIndex(int fileIndex) const;
		void GetInputValues(int fileIndex, int residueIndex, std::vector< std::vector<double> >& out_inputs, int numberOfInputsToExtract) const;
		void GetOutputValues(int fileIndex, int residueIndex, std::vector< double >& out_outputs) const;

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
		const IrisData* GetIrisDataFromSet(int dataSetIndex, int dataIndex) const;
		int GetNumberOfSets() const;
		const NeuralNetDataSet* GetDataSetAt(int dataSetIndex);

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
		void SaveWeights(std::ostream& out_file);

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
		bool m_hasBias;
		std::vector<FNeuron> m_neurons;

		//constructor
		FNeuronLayer(int numberOfNeurons, int numberOfOutputs, int numberOfInputs, bool addBiasNeuron);
		void SaveWeights(std::ostream& out_file);
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
		int GetNumberOfRequiredInputs() const;
		double GetRecentAverageError() const { return m_recentAverageError; }
		void SaveWeights(const std::string& fileName);

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
		std::vector< std::pair<int, int> > GetFilterTopology() { return m_filterTopology; }
		void GetRandomTrainingData(std::vector<const IrisData*>& out_container);
		void GetRandomValidationData(std::vector<const IrisData*>& out_container);
		int GetNumberOfSets() const;
		const NeuralNetDataSet* GetDataSetAtSpecifiedIndex(int dataSetIndex);


	private:
		void LoadTopology();
		void LoadFilterTopology();
		void GetTrainingData();
		void LoadTrainingData();
		void LoadIrisDataSet();
		void LoadDataSet();

	private:
		std::ifstream m_trainingDataFile;
		std::vector< std::pair<int, int> > m_topology;
		std::vector< std::pair<int, int> > m_filterTopology;
		NeuralNetData m_data;
		ProteinBuilder* m_proteinBuilder;

	public:
		NetDataType m_dataType;
	};
}

#endif