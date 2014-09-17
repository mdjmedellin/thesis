#include "ThesisTest.h"
#include "NeuralNetwork.h"

//=========================FNeuronConnection========================
FNeuronConnection::FNeuronConnection(int numberOfWeights)
{
	m_weight.Init(0.0, numberOfWeights);
	m_deltaWeight.Init(0.0, numberOfWeights);
}

void FNeuronConnection::RandomizeWeights()
{
	for (int i = 0; i < m_weight.Num(); ++i)
	{
		m_weight[i] = S_RandomWeight();
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
double FNeuron::s_eta = 0.15;    // overall net learning rate, [0.0..1.0]
double FNeuron::s_alpha = 0.5;   // momentum, multiplier of last deltaWeight, [0.0..1.0]

FNeuron::FNeuron()
{
	m_outputConnections.Empty();
	m_myIndex = -1;
}

FNeuron::FNeuron(int numberOfOutputs, int myIndex, int numberOfInputs)
{
	for (int i = 0; i < numberOfOutputs; ++i)
	{
		m_outputConnections.Add(FNeuronConnection(numberOfInputs));

		//randomize all of the weights for the inputs
		m_outputConnections.Last().RandomizeWeights();
	}

	m_myIndex = myIndex;
}

//by default this will set the output value at 0 if no index is passed
void FNeuron::SetOutputValue(double value, int index)
{
	m_outputVals[index] = value;
}

void FNeuron::SetOutputValues(TArray<double>& values)
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
	double dividend = 1.f + FGenericPlatformMath::Pow(EULERS_NUMBER, -x);

	return (1.f - dividend);
}

double FNeuron::S_TransferFunctionDerivative(double x)
{
	//logsig derivative
	double logsigValue = S_TransferFunction(x);

	double result = logsigValue * (1.f - logsigValue);

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
FNeuronLayer::FNeuronLayer()
: m_numberOfNeurons(0)
{}
//===================================================================

//===========================NEURALNETWORK===========================
double NeuralNetwork::m_recentAverageSmoothingFactor = 100.0; // Number of training samples to average over

NeuralNetwork::NeuralNetwork(const TArray<int>& topology)
{
	int numLayers = topology.Num();
	for (int layerNum = 0; layerNum < numLayers; ++layerNum)
	{
		//if we are on the last layer, then we are on the output layer and do not have any outputs
		unsigned numOutputs = layerNum == numLayers - 1 ? 0 : topology[layerNum + 1];

		// We have a new layer, now fill it with neurons, and
		// add a bias neuron in each layer.
		// The reason we have <= instead of < is that we want to add an extra neuron for the bias
		for (int neuronNum = 0; neuronNum <= topology[layerNum]; ++neuronNum)
		{
			m_neuronLayers.Last().m_neurons.Add(FNeuron(numOutputs, neuronNum));
		}

		// Force the bias node's output to 1.0 (it was the last neuron pushed in this layer):
		m_neuronLayers.Last().m_neurons.Last().SetOutputValue(1.0);
	}
}

void NeuralNetwork::GetResults(TArray<double>& resultVals) const
{
	resultVals.Empty();

	//we do not take into account the bias in the result on the output layer
	for (int n = 0; n < m_neuronLayers.Last().m_neurons.Num() - 1; ++n)
	{
		resultVals.Add(m_neuronLayers.Last().m_neurons[n].GetOutputValue());
	}
}

void NeuralNetwork::BackPropagation(const TArray<double>& targetVals)
{
	// Calculate overall net error (RMS of output neuron errors)
	FNeuronLayer& outputLayer = m_neuronLayers.Last();
	m_error = 0.0;

	//we do not care about the bias neuron on calculating the RMS
	for (int n = 0; n < outputLayer.m_neurons.Num() - 1; ++n)
	{
		double delta = targetVals[n] - outputLayer.m_neurons[n].GetOutputValue();
		m_error += delta * delta;
	}

	m_error /= outputLayer.m_neurons.Num() - 1; // get average error squared
	m_error = sqrt(m_error); // RMS

	// Implement a recent average measurement
	m_recentAverageError =
		(m_recentAverageError * m_recentAverageSmoothingFactor + m_error)
		/ (m_recentAverageSmoothingFactor + 1.0);

	// Calculate output layer gradients
	for (int n = 0; n < outputLayer.m_neurons.Num() - 1; ++n)
	{
		outputLayer.m_neurons[n].CalculateOutputGradients(targetVals[n]);
	}

	// Calculate hidden layer gradients
	for (int layerNum = m_neuronLayers.Num() - 2; layerNum > 0; --layerNum)
	{
		FNeuronLayer& hiddenLayer = m_neuronLayers[layerNum];
		FNeuronLayer& nextLayer = m_neuronLayers[layerNum + 1];

		for (int n = 0; n < hiddenLayer.m_neurons.Num(); ++n)
		{
			hiddenLayer.m_neurons[n].CalculateHiddenGradients(nextLayer);
		}
	}

	// For all layers from outputs to first hidden layer,
	// update connection weights
	for (int layerNum = m_neuronLayers.Num() - 1; layerNum > 0; --layerNum)
	{
		FNeuronLayer& layer = m_neuronLayers[layerNum];
		FNeuronLayer& prevLayer = m_neuronLayers[layerNum - 1];

		for (int n = 0; n < layer.m_neurons.Num() - 1; ++n)
		{
			layer.m_neurons[n].UpdateInputWeights(prevLayer);
		}
	}
}

void NeuralNetwork::FeedForward(const TArray<TArray<double>>& inputVals)
{
	check(inputVals.Num() == m_neuronLayers[0].m_neurons.Num() - 1);
	
	// Assign (latch) the input values into the input neurons
	for (int i = 0; i < inputVals.Num(); ++i)
	{
		m_neuronLayers[0].m_neurons[i].SetOutputValues(inputVals[i]);
	}

	//forward propagate
	//for every layer
	for (int layerNum = 1; layerNum < m_neuronLayers.Num(); ++layerNum)
	{
		FNeuronLayer& prevLayer = m_neuronLayers[layerNum - 1];
		
		//for every neuron in the layer
		//NOTE: we do not feed forward into the last neuron, which is the bias
		for (int n = 0; n < m_neuronLayers[layerNum].m_neurons.Num() - 1; ++n)
		{
			m_neuronLayers[layerNum].m_neurons[n].FeedForward(prevLayer);
		}
	}
}

NeuralNetwork::~NeuralNetwork()
{
}