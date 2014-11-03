#include "ThesisTest.h"
#include "ProteinUtilities.h"
#include "NeuralNetwork.h"

//=========================FNeuronConnection========================
FNeuronConnection::FNeuronConnection(int numberOfWeights)
{
	m_weight.Init(0.0, numberOfWeights);
	m_deltaWeight.Init(0.0, numberOfWeights);
}

FNeuronConnection::FNeuronConnection(const TArray<FString>& stringWeightValues)
{
	FString currentString = "";
	double currentWeight = 0.0;
	for (int weightIndex = 0; weightIndex < stringWeightValues.Num(); ++weightIndex)
	{
		currentString = stringWeightValues[weightIndex];
		GHProtein::GetTypeFromString(currentWeight, currentString);
		m_weight.Add(currentWeight);
	}
}

void FNeuronConnection::RandomizeWeights()
{
	for (int i = 0; i < m_weight.Num(); ++i)
	{
		m_weight[i] = GHProtein::RandZeroToN(0.10);
	}
}

void FNeuronConnection::SetDeltaWeight(double deltaWeight, int deltaWeightIndex)
{
	if (deltaWeightIndex < m_deltaWeight.Num())
	{
		m_deltaWeight[deltaWeightIndex] = deltaWeight;
	}
}

void FNeuronConnection::AddToWeight(double amountToAdd, int weightIndex)
{
	if (weightIndex < m_weight.Num())
	{
		m_weight[weightIndex] += amountToAdd;
	}
}
//==================================================================

//=========================FNeuron==================================
double FNeuron::s_eta = 0.075;    // overall net learning rate, [0.0..1.0]
double FNeuron::s_alpha = 0.5;   // momentum, multiplier of last deltaWeight, [0.0..1.0]

FNeuron::FNeuron()
: m_gradient(0.0)
{
	m_outputConnections.Empty();
	m_myIndex = -1;
}

FNeuron::FNeuron(int numberOfOutputs, int myIndex, int numberOfInputs)
: m_gradient(0.0)
{
	for (int i = 0; i < numberOfOutputs; ++i)
	{
		m_outputConnections.Add(FNeuronConnection(numberOfInputs));

		//randomize all of the weights for the inputs
		m_outputConnections.Last().RandomizeWeights();
	}

	m_outputVals.Init(0.0, numberOfInputs);
	m_myIndex = myIndex;
}

FNeuron::FNeuron(const TArray<FString>& stringArray, int& index, int neuronIndex)
: m_gradient(0.0)
, m_myIndex(neuronIndex)
{
	TArray<FString> currentLineStringsNoSpaces;

	//the first line should indicate the number of connections
	FString currentString = stringArray[index];
	++index;

	GHProtein::SplitLines(currentLineStringsNoSpaces, (*currentString), currentString.Len(), ' ');
	int numberOfConnections = 0;
	if (currentLineStringsNoSpaces.Num() > 1)
	{
		GHProtein::GetTypeFromString(numberOfConnections, currentLineStringsNoSpaces[1]);
	}

	for (int connectionIndex = 0; connectionIndex < numberOfConnections; ++connectionIndex)
	{
		//extract the strings with the values of the weights
		currentString = stringArray[index];
		++index;

		//separate the values contained in the string into multiple strings
		currentLineStringsNoSpaces.Empty();
		GHProtein::SplitLines(currentLineStringsNoSpaces, (*currentString), currentString.Len(), '|');

		//add a new neuron connection
		m_outputConnections.Add(FNeuronConnection(currentLineStringsNoSpaces));
	}

	if (m_outputConnections.Num() > 0)
	{
		int numberOfInputs = m_outputConnections[0].m_weight.Num();
		m_outputVals.Init(0.0, numberOfInputs);
	}
	else
	{
		m_outputVals.Init(0.0, 1);
	}
}

//by default this will set the output value at 0 if no index is passed
void FNeuron::SetOutputValue(double value, int index)
{
	if (m_outputVals.Num() > 0)
	{
		m_outputVals[index] = value;
	}
}

void FNeuron::SetOutputValues(const TArray<double>& values)
{
	m_outputVals = values;
}

double FNeuron::GetOutputValue() const
{
	//we return the sum of all of our input values
	double outputVal = 0.f;

	for (int i = 0; i < m_outputVals.Num(); ++i)
	{
		outputVal += m_outputVals[i];
	}

	return outputVal;
}

int FNeuron::GetIndexOfValidOutputValue() const
{
	if (m_outputVals.Num() != 1)
	{
		//look for the index that has an output value that is non-zero
		for (int i = 0; i < m_outputVals.Num(); ++i)
		{
			if (m_outputVals[i] > 0.f)
			{
				return i;
				break;
			}
		}

		return 0;
	}
	else
	{
		return 0;
	}
}

double FNeuron::GetDeltaWeight(int connectionIndex) const
{
	//this is not as simple
	//since our inputs can be an array, then we need to have a separate weight for each one of the elements in the array

	//Special case, if the inputs is an array of one element, then return the first deltaWeigh, otherwise check for the
	//corresponding deltaWeight
	double deltaWeight = m_outputConnections[connectionIndex].m_deltaWeight[0];
	if (m_outputVals.Num() != 1)
	{
		//look for the index that has an output value that is non-zero
		for (int i = 0; i < m_outputVals.Num(); ++i)
		{
			if (m_outputVals[i] > 0.f)
			{
				deltaWeight = m_outputConnections[connectionIndex].m_deltaWeight[i];
				break;
			}
		}
	}

	return deltaWeight;
}

double FNeuron::GetWeight(int connectionIndex) const
{
	double weight = m_outputConnections[connectionIndex].m_weight[0];

	if (m_outputVals.Num() != 1)
	{
		for (int i = 0; i < m_outputVals.Num(); ++i)
		{
			if (m_outputVals[i] > 0.f)
			{
				weight = m_outputConnections[connectionIndex].m_weight[i];
				break;
			}
		}
	}

	return weight;
}

void FNeuron::UpdateInputWeights(FNeuronLayer& prevLayer)
{
	int indexOfValidOutputValue = 0;
	// The weights to be updated are in the Connection container
	// in the neurons in the preceding layer
	for (int n = 0; n < prevLayer.m_neurons.Num(); ++n)
	{
		FNeuron& neuron = prevLayer.m_neurons[n];
		double oldDeltaWeight = neuron.GetDeltaWeight(m_myIndex);

		double newDeltaWeight =
			// Individual input, magnified by the gradient and train rate:
			s_eta
			* neuron.GetOutputValue()
			* m_gradient
			// Also add momentum = a fraction of the previous delta weight;
			+ s_alpha
			* oldDeltaWeight;

		//get the index of te weight we want to update from the set of weights assigned to this neuron
		//the following returns the index of the first non-zero output value
		//if the neuron only has one output, then it returns 0 even if the output value is zero
		indexOfValidOutputValue = neuron.GetIndexOfValidOutputValue();
		neuron.m_outputConnections[m_myIndex].SetDeltaWeight(newDeltaWeight, indexOfValidOutputValue);
		neuron.m_outputConnections[m_myIndex].AddToWeight(newDeltaWeight, indexOfValidOutputValue);
	}
}

double FNeuron::SumDOW(const FNeuronLayer& nextLayer) const
{
	double sum = 0.0;

	int weightIndex = GetIndexOfValidOutputValue();
	// Sum our contributions of the errors at the nodes we feed.
	// the reason we subtract 1 is to avoid doing this to the bias neuron
	for (int n = 0; n < nextLayer.m_neurons.Num() - 1; ++n)
	{
		//connection to the specified neuron
		const FNeuronConnection& outputConnection = m_outputConnections[n];

		//get the weight corresponding weight
		double weight = outputConnection.m_weight[weightIndex];
		sum += weight * nextLayer.m_neurons[n].m_gradient;
	}

	return sum;
}

void FNeuron::CalculateHiddenGradients(const FNeuronLayer& nextLayer)
{
	double dow = SumDOW(nextLayer);
	m_gradient = dow * FNeuron::S_TransferFunctionDerivative(GetOutputValue());
}

void FNeuron::CalculateOutputGradients(double targetVal)
{
	double outputVal = GetOutputValue();
	double delta = targetVal - outputVal;
	m_gradient = delta * FNeuron::S_TransferFunctionDerivative(outputVal);
}

double FNeuron::S_TransferFunction(double x)
{
	//we use logsig as the transfer function
	double dividend = 1.0 + FGenericPlatformMath::Pow(EULERS_NUMBER, -x);

	return (1.0 / dividend);
}

double FNeuron::S_TransferFunctionDerivative(double x)
{
	double result = x * (1.0 - x);
	return result;
}

void FNeuron::FeedForward(const FNeuronLayer& prevLayer)
{
	double sum = 0.0;

	// Sum the previous layer's outputs (which are our inputs)
	// Include the bias node from the previous layer.
	for (int n = 0; n < prevLayer.m_neurons.Num(); ++n)
	{
		sum += prevLayer.m_neurons[n].GetOutputValue() * prevLayer.m_neurons[n].GetWeight(m_myIndex);
	}

	SetOutputValue(FNeuron::S_TransferFunction(sum));
}
//===================================================================

//===========================NEURONLAYER=============================
FNeuronLayer::FNeuronLayer(int numberOfNeurons, int numberOfOutputs, int numberOfVariablesInInput, bool addBiasNeuron)
: m_numberOfNeurons(numberOfNeurons)
, m_hasBias(addBiasNeuron)
{
	for (int i = 0; i < numberOfNeurons; ++i)
	{
		m_neurons.Add(FNeuron(numberOfOutputs, i, numberOfVariablesInInput));
	}

	if (addBiasNeuron)
	{
		//add the bias
		//the bias only has one vairableper input
		m_neurons.Add(FNeuron(numberOfOutputs, numberOfNeurons));
		m_neurons.Last().SetOutputValue(1.0);
	}
}

FNeuronLayer::FNeuronLayer(const TArray<FString>& stringArray, int& index, int neuronCount, bool hasBias)
: m_numberOfNeurons(neuronCount)
, m_hasBias(hasBias)
{
	for (int neuronIndex = 0; neuronIndex < neuronCount; ++neuronIndex)
	{
		m_neurons.Add(FNeuron(stringArray, index, neuronIndex));
	}

	if (m_hasBias 
		&& m_neurons.Num() > 0)
	{
		m_numberOfNeurons -= 1;
		m_neurons.Last().SetOutputValue(1.0);
	}
}

FNeuronLayer::FNeuronLayer()
: m_numberOfNeurons(0)
, m_hasBias(false)
{}
//===================================================================

//===========================NEURALNETWORK===========================
double NeuralNetwork::m_recentAverageSmoothingFactor = 1.0; // Number of training samples to average over

NeuralNetwork::NeuralNetwork(const FString& weightsFileLocation)
: m_numberOfInputs(0)
, m_numberOfOutputs(0)
, m_numberOfHiddenLayers(0)
, m_error(0.0)
, m_recentAverageError(0.0)
{
	TArray<FString> stringArray;

	//lets load the file from the specified location
	//Note: this will only load the file from filders within the GameContents directory
	FString pathToFile = FPaths::GameContentDir() + weightsFileLocation;
	
	if (FFileHelper::LoadANSITextFileToStrings(*(pathToFile), NULL, stringArray))
	{
		InitializeNetworkFromFile(stringArray);
	}
}

NeuralNetwork::NeuralNetwork(const TArray<FIntPair>& topology, bool addBiasNeuron)
: m_numberOfInputs(0)
, m_numberOfOutputs(0)
, m_numberOfHiddenLayers(0)
, m_error(0.0)
, m_recentAverageError(0.0)
{
	InitializeNeuralNetwork(topology, addBiasNeuron);
}

void NeuralNetwork::InitializeNetworkFromFile(const TArray<FString>& stringArray)
{
	//the reason we don't increment the index is because ExtractNeuralLayer will offset
	//the index accordingly when it extracts the information for that layer
	for (int index = 0; index < stringArray.Num();)
	{
		ExtractNeuralLayer(stringArray, index);
	}
}

void NeuralNetwork::ExtractNeuralLayer(const TArray<FString>& stringArray, int& index)
{
	FString currentString;
	TArray<FString> currentLineNoSpaces;

	//first string indicates the layer number
	currentString = stringArray[index];
	++index;

	//if first line is empty, then we return
	if (currentString.IsEmpty())
	{
		return;
	}

	//second string indicates the neuron count
	currentString = stringArray[index];
	++index;

	//split the line according to the spaces
	GHProtein::SplitLines(currentLineNoSpaces, *(currentString), currentString.Len(), ' ');
	//second line is always the value of the key
	int neuronCount = 0;
	if (currentLineNoSpaces.Num() > 1)
	{
		GHProtein::GetTypeFromString(neuronCount, currentLineNoSpaces[1]);
	}

	//third string indicates wether the layer makes use of a bias neuron
	currentString = stringArray[index];
	++index;

	currentLineNoSpaces.Empty();
	GHProtein::SplitLines(currentLineNoSpaces, *(currentString), currentString.Len(), ' ');
	bool hasBias = false;
	if (currentLineNoSpaces.Num() > 1)
	{
		GHProtein::GetTypeFromString(hasBias, currentLineNoSpaces[1]);
	}

	m_neuronLayers.Add(new FNeuronLayer(stringArray, index, neuronCount, hasBias));
}

void NeuralNetwork::InitializeNeuralNetwork(const TArray<FIntPair>& topology, bool addBiasNeuron)
{
	int numLayers = topology.Num();
	int numberOfNeuronsInLayer = 0;
	int numberOfVariablesInInput = 0;

	for (int layerNum = 0; layerNum < numLayers; ++layerNum)
	{
		numberOfNeuronsInLayer = topology[layerNum].first;
		numberOfVariablesInInput = topology[layerNum].second;

		unsigned numOutputs = layerNum == numLayers - 1 ? 0 : topology[layerNum + 1].first;

		m_neuronLayers.Add(new FNeuronLayer(numberOfNeuronsInLayer, numOutputs, numberOfVariablesInInput, addBiasNeuron));
	}
}

int NeuralNetwork::GetNumberOfRequiredInputs() const
{
	if (m_neuronLayers.Num() > 1)
	{
		return m_neuronLayers[0]->m_numberOfNeurons;
	}

	return 0;
}

void NeuralNetwork::GetResults(TArray<double>& resultVals) const
{
	resultVals.Empty();

	//we do not take into account the bias in the result on the output layer
	int neuronCount = m_neuronLayers.Last()->m_neurons.Num();
	if (m_neuronLayers.Last()->m_hasBias)
	{
		neuronCount -= 1;
	}

	for (int n = 0; n < neuronCount; ++n)
	{
		resultVals.Add(m_neuronLayers.Last()->m_neurons[n].GetOutputValue());
	}
}

void NeuralNetwork::BackPropagation(const TArray<double>& targetVals)
{
	// Calculate overall net error (RMS of output neuron errors)
	FNeuronLayer* outputLayer = m_neuronLayers.Last();
	m_error = 0.0;

	//we do not care about the bias neuron on calculating the RMS
	for (int n = 0; n < outputLayer->m_neurons.Num() - 1; ++n)
	{
		double delta = targetVals[n] - outputLayer->m_neurons[n].GetOutputValue();
		m_error += delta * delta;
	}

	m_error /= outputLayer->m_neurons.Num() - 1; // get average error squared
	m_error = sqrt(m_error); // RMS

	// Implement a recent average measurement
	m_recentAverageError =
		(m_recentAverageError * m_recentAverageSmoothingFactor + m_error)
		/ (m_recentAverageSmoothingFactor + 1.0);

	// Calculate output layer gradients
	for (int n = 0; n < outputLayer->m_neurons.Num() - 1; ++n)
	{
		outputLayer->m_neurons[n].CalculateOutputGradients(targetVals[n]);
	}

	// Calculate hidden layer gradients
	for (int layerNum = m_neuronLayers.Num() - 2; layerNum > 0; --layerNum)
	{
		FNeuronLayer* hiddenLayer = m_neuronLayers[layerNum];
		FNeuronLayer* nextLayer = m_neuronLayers[layerNum + 1];

		for (int n = 0; n < hiddenLayer->m_neurons.Num(); ++n)
		{
			hiddenLayer->m_neurons[n].CalculateHiddenGradients(*nextLayer);
		}
	}

	// For all layers from outputs to first hidden layer,
	// update connection weights
	for (int layerNum = m_neuronLayers.Num() - 1; layerNum > 0; --layerNum)
	{
		FNeuronLayer* layer = m_neuronLayers[layerNum];
		FNeuronLayer* prevLayer = m_neuronLayers[layerNum - 1];

		for (int n = 0; n < layer->m_neurons.Num() - 1; ++n)
		{
			layer->m_neurons[n].UpdateInputWeights(*prevLayer);
		}
	}
}

void NeuralNetwork::FeedForward(const TArray< TArray<double> >& inputVals)
{
	check(inputVals.Num() == m_neuronLayers[0]->m_neurons.Num() - 1);
	
	// Assign (latch) the input values into the input neurons
	for (int i = 0; i < inputVals.Num(); ++i)
	{
		m_neuronLayers[0]->m_neurons[i].SetOutputValues(inputVals[i]);
	}

	//forward propagate
	//for every layer
	for (int layerNum = 1; layerNum < m_neuronLayers.Num(); ++layerNum)
	{
		FNeuronLayer* prevLayer = m_neuronLayers[layerNum - 1];
		
		//for every neuron in the layer
		//NOTE: we do not feed forward into the last neuron, which is the bias
		for (int n = 0; n < m_neuronLayers[layerNum]->m_neurons.Num() - 1; ++n)
		{
			m_neuronLayers[layerNum]->m_neurons[n].FeedForward(*prevLayer);
		}
	}
}

NeuralNetwork::~NeuralNetwork()
{
	for (int i = 0; i < m_neuronLayers.Num(); ++i)
	{
		delete m_neuronLayers[i];
		m_neuronLayers[i] = nullptr;
	}
}
//===============================================================================

