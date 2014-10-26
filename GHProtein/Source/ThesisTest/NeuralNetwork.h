#pragma once
#include "NeuralNetwork.generated.h"

//it seems that UE4 does not have the equivalent of a pair
USTRUCT()
struct FIntPair
{
public:
	GENERATED_USTRUCT_BODY()
	FIntPair(int firstInt = 0, int secondInt = 0)
	{
		first = firstInt;
		second = secondInt;
	}

private:

public:
	int first;
	int second;
};

USTRUCT()
struct FNeuronConnection
{
public:
	GENERATED_USTRUCT_BODY()

	FNeuronConnection(int numberOfWeights = 1);
	FNeuronConnection(const TArray<FString>& stringWeightValues);
	void RandomizeWeights();
	void SetDeltaWeight(double deltaWeight, int deltaWeightIndex);
	void AddToWeight(double amountToAdd, int weightIndex);

private:

public:
	TArray<double> m_weight;
	TArray<double> m_deltaWeight;
};

struct FNeuronLayer;

USTRUCT()
struct FNeuron
{
public:
	GENERATED_USTRUCT_BODY()
	
	FNeuron();
	FNeuron(int numberOfOutputs, int myIndex, int numberOfinputs = 1);
	FNeuron(const TArray<FString>& stringArray, int& index, int neuronIndex);
	void SetOutputValues(const TArray<double>& values);
	void SetOutputValue(double value, int index = 0);
	double GetOutputValue() const;
	double GetDeltaWeight(int connectionIndex) const;
	double GetWeight(int connectionIndex) const;
	int GetIndexOfValidOutputValue() const;
	void FeedForward(const FNeuronLayer& prevLayer);
	void CalculateOutputGradients(double targetVal);
	void CalculateHiddenGradients(const FNeuronLayer& nextLayer);
	void UpdateInputWeights(FNeuronLayer& prevLayer);

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
	bool m_hasBias;
	TArray<FNeuron> m_neurons;

	//constructor
	FNeuronLayer(int numberOfNeurons, int numberOfOutputs, int numberOfInputs, bool addBiasNeuron);
	FNeuronLayer(const TArray<FString>& stringArray, int &index, int neuronCount, bool hasBias);
	FNeuronLayer();
	//void LoadWeights();
};

/**
 * 
 */
class THESISTEST_API NeuralNetwork
{
public:
	NeuralNetwork(const FString& weightsFileLocation);
	NeuralNetwork(const TArray<FIntPair>& topology, bool addBiasNeuron = false);
	~NeuralNetwork();

	void FeedForward(const TArray< TArray<double> >& inputValues);			//this is our update function, it is the one that passes the input to the network
	void BackPropagation(const TArray<double>& targetValues);
	void GetResults(TArray<double>& resultValues)const;
	int GetNumberOfRequiredInputs() const;
	double GetRecentAverageError() const { return m_recentAverageError; }
	void SaveWeights(const FString& fileName);
	void LoadWeights(const FString& fileName);

private:
	//private functions
	void InitializeNeuralNetwork(const TArray<FIntPair>& topology, bool addBiasNeuron);
	void InitializeNetworkFromFile(const TArray<FString>& atringArray);
	void ExtractNeuralLayer(const TArray<FString>& stringArray, int& index);

public:
	//public data members

private:
	//private data members
	int m_numberOfInputs;
	int m_numberOfOutputs;
	int m_numberOfHiddenLayers;

	TArray<FNeuronLayer*> m_neuronLayers;
	double m_error;
	double m_recentAverageError;
	static double m_recentAverageSmoothingFactor;
};