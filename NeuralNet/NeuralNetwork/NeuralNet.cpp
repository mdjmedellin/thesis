#include "NeuralNet.hpp"
#include "ProteinUtilities.hpp"
#include "ProteinBuilder.hpp"
#include "ProteinModel.hpp"
#include <cassert>
#include <sstream>
#include <iostream>

namespace GHProtein
{
	//========================IrisData==================================
	IrisData::IrisData()
	: m_inputData(nullptr)
	, m_expectedOutput(nullptr)
	{}

	IrisData::~IrisData()
	{
		if (m_inputData)
		{
			delete m_inputData;
		}
		if (m_expectedOutput)
		{
			delete m_expectedOutput;
		}
	}

	void IrisData::SetInputData(float input_0, float input_1, float input_2, float input_3)
	{
		if (!m_inputData)
		{
			m_inputData = new float[4];
		}

		m_inputData[0] = input_0;
		m_inputs.push_back(input_0);
		m_inputData[1] = input_1;
		m_inputs.push_back(input_1);
		m_inputData[2] = input_2;
		m_inputs.push_back(input_2);
		m_inputData[3] = input_3;
		m_inputs.push_back(input_3);
	}

	void IrisData::SetOutputData(float output_0, float output_1, float output_2)
	{
		if (!m_expectedOutput)
		{
			m_expectedOutput = new float[3];
		}

		m_expectedOutput[0] = output_0;
		m_outputs.push_back(output_0);
		m_expectedOutput[1] = output_1;
		m_outputs.push_back(output_1);
		m_expectedOutput[2] = output_2;
		m_outputs.push_back(output_2);
	}

	void IrisData::GetInputValues(std::vector< std::vector<double> >& out_inputs) const
	{
		out_inputs.clear();
		std::vector<double> values;

		for (int i = 0; i < m_inputs.size(); ++i)
		{
			values.clear();
			values.push_back(m_inputData[i]);
			out_inputs.push_back(values);
		}
	}

	void IrisData::GetOutputValues(std::vector<double>& outputs) const
	{
		outputs.clear();

		for (int i = 0; i < 3; ++i)
		{
			outputs.push_back(m_expectedOutput[i]);
		}
	}
	//==================================================================

	//=========================NeuralNetData============================
	NeuralNetData::NeuralNetData()
		: m_rootLocation("")
	{}

	void NeuralNetData::SetRootLocation(const std::string& rootLocation)
	{
		m_rootLocation = rootLocation;
	}

	std::string NeuralNetData::GetRootLocation()
	{
		return m_rootLocation;
	}

	void NeuralNetData::AddDataSet(const NeuralNetDataSet& dataSet)
	{
		m_dataSets.push_back(dataSet);
	}

	IrisData* NeuralNetData::GetIrisDataFromSet(int dataSetIndex, int dataIndex)
	{
		return m_dataSets[dataSetIndex].GetIrisData(dataIndex);
	}

	int NeuralNetData::GetNumberOfSets() const
	{
		return m_dataSets.size();
	}

	const NeuralNetDataSet* NeuralNetData::GetDataSetAt(int dataSetIndex)
	{
		if (dataSetIndex >= 0
			&& dataSetIndex < m_dataSets.size())
		{
			return &(m_dataSets[dataSetIndex]);
		}
		else
		{
			return nullptr;
		}
	}
	//==================================================================
	
	//==========================NeuralNetDataSet========================
	NeuralNetDataSet::NeuralNetDataSet()
		: m_directory("")
	{}

	void NeuralNetDataSet::SetDirectory(const std::string& directoryName)
	{
		m_directory = directoryName;
	}

	void NeuralNetDataSet::AddFile(const std::string& fileName)
	{
		m_files.push_back(fileName);
	}

	IrisData* NeuralNetDataSet::GetIrisData(int dataIndex)
	{
		if (dataIndex > -1 && dataIndex < m_irisDataContainer.size())
		{
			return m_irisDataContainer[dataIndex];
		}
		return nullptr;
	}

	void NeuralNetDataSet::LoadProteinModels(ProteinBuilder* proteinBuilder, const std::string& dataRootLocation)
	{
		std::string rootPath = dataRootLocation;
		if (!rootPath.empty())
		{
			rootPath.append("/");
		}

		if (!m_directory.empty())
		{
			rootPath.append(m_directory + "/");
		}

		//iterate through the files and load the proteins
		std::string filePath = "";
		ProteinModel* currentProteinModel = nullptr;
		for (int fileIndex = 0; fileIndex < m_files.size(); ++fileIndex)
		{
			filePath = rootPath + m_files[fileIndex];

			//now that we have the file path
			//load the protein model
			currentProteinModel = proteinBuilder->LoadFile(filePath);
			m_proteinModels.push_back(currentProteinModel);
		}
	}

	void NeuralNetDataSet::LoadIrisData(const std::string& dataRootLocation)
	{
		std::string rootPath = dataRootLocation;
		if (!rootPath.empty())
		{
			rootPath.append("/");
		}

		if (!m_directory.empty())
		{
			rootPath.append(m_directory + "/");
		}

		//iterate through the files and load the proteins
		std::string filePath = "";
		IrisData* currentData = nullptr;
		std::string currentLine = "";
		std::ifstream dataFile;
		std::vector<std::string> tempStringList;
		std::vector<std::string> tempStringList2;
		for (int fileIndex = 0; fileIndex < m_files.size(); ++fileIndex)
		{
			filePath = rootPath + m_files[fileIndex];

			// Create file reader
			dataFile.open(filePath);
			if (dataFile.good())
			{
				while (getline(dataFile, currentLine))
				{
					currentData = new IrisData();

					//we have the line of text, time to parse it
					TokenizeString(tempStringList, currentLine, "|");

					//now we have the input and expected out separated
					//tokenize the input
					TokenizeString(tempStringList2, tempStringList[0], ",");
					currentData->SetInputData(std::atof(tempStringList2[0].c_str())
						, std::atof(tempStringList2[1].c_str())
						, std::atof(tempStringList2[2].c_str())
						, std::atof(tempStringList2[3].c_str()));

					//tokenize the output
					TokenizeString(tempStringList2, tempStringList[1], ",");
					currentData->SetOutputData(std::atof(tempStringList2[0].c_str())
						, std::atof(tempStringList2[1].c_str())
						, std::atof(tempStringList2[2].c_str()));

					m_irisDataContainer.push_back(currentData);
				}
			}
		}
	}

	int NeuralNetDataSet::GetNumberOfFilesInSet() const
	{
		return m_proteinModels.size();
	}

	int NeuralNetDataSet::GetSizeOfTrainingDataAtSpecifiedIndex(int fileIndex) const
	{
		if (fileIndex >= 0
			&& fileIndex < m_proteinModels.size())
		{
			return m_proteinModels[fileIndex]->GetLengthOfChain();
		}
		else
		{
			return 0;
		}
	}

	void NeuralNetDataSet::GetInputValues(int fileIndex, int residueIndex, std::vector< std::vector<double> >& out_inputs) const
	{
		if (fileIndex >= 0
			&& fileIndex < m_proteinModels.size())
		{
			m_proteinModels[fileIndex]->GetInputValues(residueIndex, out_inputs);
		}
	}

	void NeuralNetDataSet::GetOutputValues(int fileIndex, int residueIndex, std::vector< double >& out_outputs) const
	{
		if (fileIndex >= 0
			&& fileIndex < m_proteinModels.size())
		{
			m_proteinModels[fileIndex]->GetOutputValues(residueIndex, out_outputs);
		}
	}
	//==================================================================

	//=========================FNeuronConnection========================
	FNeuronConnection::FNeuronConnection(int numberOfWeights)
		: m_weight(numberOfWeights,0.0)
		, m_deltaWeight(numberOfWeights,0.0)
	{}

	void FNeuronConnection::RandomizeWeights()
	{
		for (int i = 0; i < m_weight.size(); ++i)
		{
			m_weight[i] = RandZeroToN(0.10);
		}
	}

	void FNeuronConnection::SetDeltaWeight(double deltaWeight, int deltaWeightIndex)
	{
		if (deltaWeightIndex < m_deltaWeight.size())
		{
			m_deltaWeight[deltaWeightIndex] = deltaWeight;
		}
	}

	void FNeuronConnection::AddToWeight(double amountToAdd, int weightIndex)
	{
		if (weightIndex < m_weight.size())
		{
			m_weight[weightIndex] += amountToAdd;
		}
	}
	//==================================================================

	//=========================FNeuron==================================
	double FNeuron::s_eta = 0.075;    // In reality this should be .1 or less. Overall net learning rate, [0.0..1.0]
	double FNeuron::s_alpha = 0.5;   // momentum, multiplier of last deltaWeight, [0.0..1.0]

	FNeuron::FNeuron()
		: m_gradient(0.0)
	{
		m_myIndex = -1;
	}

	FNeuron::FNeuron(int numberOfOutputs, int myIndex, int numberOfInputs)
		: m_gradient(0.0)
	{
		for (int i = 0; i < numberOfOutputs; ++i)
		{
			m_outputConnections.push_back(FNeuronConnection(numberOfInputs));

			//randomize all of the weights for the inputs
			m_outputConnections.back().RandomizeWeights();
		}

		m_outputVals.resize(numberOfInputs);
		std::fill_n(m_outputVals.begin(), numberOfInputs, 0.0);

		m_myIndex = myIndex;
	}

	//by default this will set the output value at 0 if no index is passed
	void FNeuron::SetOutputValue(double value, int index)
	{
		if (std::isinf(value))
		{
			int x = 1;
		}
		m_outputVals[index] = value;
	}

	void FNeuron::SetOutputValues(const std::vector<double>& values)
	{
		m_outputVals = values;
	}

	double FNeuron::GetOutputValue() const
	{
		//we return the sum of all of our input values
		double outputVal = 0.f;

		for (int i = 0; i < m_outputVals.size(); ++i)
		{
			outputVal += m_outputVals[i];
		}

		//check that the output value is 1
		if (m_outputVals.size() > 1
			&& outputVal != 1.0)
		{
			int x = 1;
		}

		return outputVal;
	}

	int FNeuron::GetIndexOfValidOutputValue() const
	{
		if (m_outputVals.size() != 1)
		{
			//look for the index that has an output value that is non-zero
			for (int i = 0; i < m_outputVals.size(); ++i)
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
		if (m_outputVals.size() != 1)
		{
			//look for the index that has an output value that is non-zero
			for (int i = 0; i < m_outputVals.size(); ++i)
			{
				if (m_outputVals[i] > 0.0)
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

		if (m_outputVals.size() != 1)
		{
			for (int i = 0; i < m_outputVals.size(); ++i)
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
		for (int n = 0; n < prevLayer.m_neurons.size(); ++n)
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
		for (int n = 0; n < nextLayer.m_neurons.size() - 1; ++n)
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
		double dividend = 1.0 + exp(-x);

		return (1.0 / dividend);
	}

	double FNeuron::S_TransferFunctionDerivative(double x)
	{
		//Test something that i realize while reading more documentation on neural networks
		double result = x * (1.0 - x);
		return result;
		/*
		//logsig derivative
		double logsigValue = S_TransferFunction(x);

		double result = logsigValue * (1.0 - logsigValue);

		return result;
		*/
	}

	void FNeuron::FeedForward(const FNeuronLayer& prevLayer)
	{
		double sum = 0.0;

		// Sum the previous layer's outputs (which are our inputs)
		// Include the bias node from the previous layer.
		for (int n = 0; n < prevLayer.m_neurons.size(); ++n)
		{
			sum += prevLayer.m_neurons[n].GetOutputValue() * prevLayer.m_neurons[n].GetWeight(m_myIndex);
			//std::cout << "Output Value of Neuron [" << n << "] = " << prevLayer.m_neurons[n].GetOutputValue() << std::endl;
			//std::cout << "Weight of Neuron [" << n << "] = " << prevLayer.m_neurons[n].GetWeight(m_myIndex) << std::endl;

		}

		SetOutputValue(FNeuron::S_TransferFunction(sum));
	}
	//===================================================================

	//===========================NEURONLAYER=============================
	FNeuronLayer::FNeuronLayer(int numberOfNeurons, int numberOfOutputs, int numberOfVariablesInInput, bool addBiasNeuron)
	{
		for (int i = 0; i < numberOfNeurons; ++i)
		{
			m_neurons.push_back(FNeuron(numberOfOutputs, i, numberOfVariablesInInput));
		}

		if (addBiasNeuron)
		{
			//add the bias
			//the bias only has one variable per input
			m_neurons.push_back(FNeuron(numberOfOutputs, numberOfNeurons));
			m_neurons.back().SetOutputValue(1.0);
		}
	}
	//===================================================================

	//===========================NEURALNET===============================
	double NeuralNet::m_recentAverageSmoothingFactor = 1.0; // Number of training samples to average over

	NeuralNet::NeuralNet(const std::vector< std::pair<int,int> >& topology, bool addBiasNeuron)
	{
		int numLayers = topology.size();
		int numberOfNeuronsInLayer = 0;
		int numberOfVariablesInInput = 0;

		for (int layerNum = 0; layerNum < numLayers; ++layerNum)
		{
			//if we are on the last layer, then we are on the output layer and do not have any outputs
			numberOfNeuronsInLayer = topology[layerNum].first;
			numberOfVariablesInInput = topology[layerNum].second;

			unsigned numOutputs = layerNum == numLayers - 1 ? 0 : topology[layerNum + 1].first;

			m_neuronLayers.push_back(new FNeuronLayer(numberOfNeuronsInLayer, numOutputs, numberOfVariablesInInput, addBiasNeuron));
		}
	}

	void NeuralNet::GetResults(std::vector<double>& resultVals) const
	{
		resultVals.clear();

		//we do not take into account the bias in the result on the output layer
		for (int n = 0; n < m_neuronLayers.back()->m_neurons.size() - 1; ++n)
		{
			resultVals.push_back(m_neuronLayers.back()->m_neurons[n].GetOutputValue());
		}
	}

	void NeuralNet::BackPropagation(const std::vector<double>& targetVals)
	{
		// Calculate overall net error (RMS of output neuron errors)
		FNeuronLayer* outputLayer = m_neuronLayers.back();
		m_error = 0.0;

		//we do not care about the bias neuron on calculating the RMS
		for (int n = 0; n < outputLayer->m_neurons.size() - 1; ++n)
		{
			double delta = targetVals[n] - outputLayer->m_neurons[n].GetOutputValue();
			//std::cout << targetVals[n] << std::endl;
			//std::cout << outputLayer->m_neurons[n].GetOutputValue() << std::endl;
			m_error += delta * delta;
			/*if (std::isinf(m_error))
			{
				int x = 1;
			}
			outputLayer->m_neurons[n].GetOutputValue();
			*/
		}

		m_error /= (outputLayer->m_neurons.size() - 1); // get average error squared
		m_error = sqrt(m_error); // RMS

		// Implement a recent average measurement
		m_recentAverageError =
			(m_recentAverageError * m_recentAverageSmoothingFactor + m_error)
			/ (m_recentAverageSmoothingFactor + 1.0);

		//std::cout << m_recentAverageError << std::endl;

		// Calculate output layer gradients
		for (int n = 0; n < outputLayer->m_neurons.size() - 1; ++n)
		{
			outputLayer->m_neurons[n].CalculateOutputGradients(targetVals[n]);
		}

		// Calculate hidden layer gradients
		for (int layerNum = m_neuronLayers.size() - 2; layerNum > 0; --layerNum)
		{
			FNeuronLayer* hiddenLayer = m_neuronLayers[layerNum];
			FNeuronLayer* nextLayer = m_neuronLayers[layerNum + 1];

			for (int n = 0; n < hiddenLayer->m_neurons.size(); ++n)
			{
				hiddenLayer->m_neurons[n].CalculateHiddenGradients(*nextLayer);
			}
		}

		// For all layers from outputs to first hidden layer,
		// update connection weights
		for (int layerNum = m_neuronLayers.size() - 1; layerNum > 0; --layerNum)
		{
			FNeuronLayer* layer = m_neuronLayers[layerNum];
			FNeuronLayer* prevLayer = m_neuronLayers[layerNum - 1];

			for (int n = 0; n < layer->m_neurons.size() - 1; ++n)
			{
				layer->m_neurons[n].UpdateInputWeights(*prevLayer);
			}
		}
	}

	void NeuralNet::FeedForward(const std::vector< std::vector<double> >& inputVals)
	{
		assert(inputVals.size() == m_neuronLayers[0]->m_neurons.size() - 1);

		// Assign (latch) the input values into the input neurons
		for (int i = 0; i < inputVals.size(); ++i)
		{
			m_neuronLayers[0]->m_neurons[i].SetOutputValues(inputVals[i]);
		}

		//forward propagate
		//for every layer
		for (int layerNum = 1; layerNum < m_neuronLayers.size(); ++layerNum)
		{
			FNeuronLayer* prevLayer = m_neuronLayers[layerNum - 1];

			//for every neuron in the layer
			//NOTE: we do not feed forward into the last neuron, which is the bias
			for (int n = 0; n < m_neuronLayers[layerNum]->m_neurons.size() - 1; ++n)
			{
				m_neuronLayers[layerNum]->m_neurons[n].FeedForward(*prevLayer);
			}
		}
	}

	NeuralNet::~NeuralNet()
	{
		for (int i = 0; i < m_neuronLayers.size(); ++i)
		{
			delete m_neuronLayers[i];
			m_neuronLayers[i] = nullptr;
		}
	}
	//========================================================================

	//=============================TRAINING DATA==============================
	TrainingData::TrainingData(const std::string filename)
		: m_proteinBuilder(new ProteinBuilder())
	{
		//open the file
		m_trainingDataFile.open(filename.c_str());
		
		if (m_trainingDataFile.good())
		{
			//load the training data
			LoadTrainingData();
		}

		m_trainingDataFile.close();
	}

	TrainingData::~TrainingData()
	{
		delete m_proteinBuilder;
	}

	void TrainingData::LoadTrainingData()
	{
		LoadTopology();
		GetTrainingData();
	}

	void TrainingData::LoadTopology()
	{
		std::string line;
		std::string label;

		getline(m_trainingDataFile, line);
		std::stringstream ss(line);
		ss >> label;

		//the first line should be the topology line
		if (this->isEOF() || label.compare("topology:") != 0)
		{
			abort();
		}

		//parse the line into the topology
		std::vector<std::string> results;

		TokenizeString(results, line, " ");

		//at this point I should have the information about each one of the layers
		std::string pair;
		std::pair<int, int> integerPair;
		std::vector<std::string> tokenized_pair;
		for (int i = 1; i < results.size(); ++i)
		{
			pair = results[i];
			//now we have to separate the pair into the two ints
			TokenizeString(tokenized_pair, pair, "|");

			//the tokenized pair should have at most 2 and at least 1 integer value
			assert(tokenized_pair.size() > 0 && tokenized_pair.size() < 3);

			integerPair.first = atoi(tokenized_pair[0].c_str());

			if (tokenized_pair.size() == 2)
			{
				integerPair.second = atoi(tokenized_pair[1].c_str());
			}
			else
			{
				integerPair.second = 1;
			}

			m_topology.push_back(integerPair);
		}

		//the next line should be empty, thus we skip it
		getline(m_trainingDataFile, line);
	}

	void TrainingData::GetTrainingData()
	{
		std::string line;
		std::string label;

		getline(m_trainingDataFile, line);
		std::stringstream ss(line);
		ss >> label;

		//the first line should be the data line that describes the number of sets and the location of them
		if (this->isEOF() || label.compare("data:") != 0)
		{
			abort();
		}

		//get the data section of the line
		std::vector<std::string> results;
		TokenizeString(results, line, " ");

		//at this point I should have the information about the training data
		std::string dataSubstring;
		std::vector<std::string> tokenized_substring;

		//we ignore the first part of the string because it is what contains the label
		dataSubstring = results[1];
		TokenizeString(tokenized_substring, dataSubstring, "|");

		assert(tokenized_substring.size() > 0);

		//save the total number of data sets and the root location of the data sets if it is specified
		int numberOfSets = std::atoi(tokenized_substring[0].c_str());
		if (tokenized_substring.size() > 1)
		{
			m_data.SetRootLocation(tokenized_substring[1]);
		}

		//the next line should be empty, thus we skip it
		getline(m_trainingDataFile, line);

		//start loading the sets of information
		for (int setIndex = 0; setIndex < numberOfSets; ++setIndex)
		{
			LoadDataSet();

			if (setIndex != numberOfSets - 1)
			{
				//skip the empty line
				getline(m_trainingDataFile, line);
			}
		}
	}

	void TrainingData::LoadDataSet()
	{
		std::string line;
		std::string label;

		getline(m_trainingDataFile, line);
		std::stringstream ss(line);
		ss >> label;

		if (this->isEOF() || label.compare("set:") != 0)
		{
			abort();
		}

		//get the data section of the line
		std::vector<std::string> results;
		TokenizeString(results, line, " ");

		std::string dataSubstring;
		std::vector<std::string> tokenized_substring;

		//we skip the first part of the string because it is what contains the label
		dataSubstring = results[1];
		TokenizeString(tokenized_substring, dataSubstring, "|");

		assert(tokenized_substring.size() > 0);

		NeuralNetDataSet currentSet;
		int numberOfFilesInSet = std::atoi(tokenized_substring[0].c_str());
		if (tokenized_substring.size() > 1)
		{
			currentSet.SetDirectory(tokenized_substring[1]);
		}

		//get the name of the files
		for (int fileIndex = 0; fileIndex < numberOfFilesInSet; ++fileIndex)
		{
			getline(m_trainingDataFile, line);
			currentSet.AddFile(line);
		}

		//load the protein models from the information in the data set
		currentSet.LoadProteinModels(m_proteinBuilder, m_data.GetRootLocation());
		//currentSet.LoadIrisData(m_data.GetRootLocation());

		m_data.AddDataSet(currentSet);
	}

	void TrainingData::LoadIrisDataSet()
	{
		std::string line;
		std::string label;

		getline(m_trainingDataFile, line);
		std::stringstream ss(line);
		ss >> label;

		if (this->isEOF() || label.compare("set:") != 0)
		{
			abort();
		}

		//get the data section of the line
		std::vector<std::string> results;
		TokenizeString(results, line, " ");

		//at this point I should have the information about each one of the layers
		std::string dataSubstring;
		std::vector<std::string> tokenized_substring;

		dataSubstring = results[1];
		TokenizeString(tokenized_substring, dataSubstring, "|");

		assert(tokenized_substring.size() > 0);

		NeuralNetDataSet currentSet;
		int numberOfFilesInSet = std::atoi(tokenized_substring[0].c_str());
		if (tokenized_substring.size() > 1)
		{
			currentSet.SetDirectory(tokenized_substring[1]);
		}

		//get the name of the files
		for (int fileIndex = 0; fileIndex < numberOfFilesInSet; ++fileIndex)
		{
			getline(m_trainingDataFile, line);
			currentSet.AddFile(line);
		}

		//load the protein models from the information in the data set
		currentSet.LoadIrisData(m_data.GetRootLocation());

		m_data.AddDataSet(currentSet);
	}

	void TrainingData::GetRandomTrainingData(std::vector<const IrisData*>& out_container)
	{
		//we randomize the order of the outputs
		int setA = 40;
		int setB = 40;
		int setC = 40;
		int sizeOfContainer = setA + setB + setC;

		out_container.clear();
		out_container.reserve(sizeOfContainer);

		int randSetIndex = 0;
		for (int i = 0; i < sizeOfContainer; ++i)
		{
			randSetIndex = RandZeroToN(3.0);

			bool valueAdded = false;
			while (!valueAdded)
			{
				switch (randSetIndex)
				{
				case 0:
					if (setA > 0)
					{
						out_container.push_back(m_data.GetIrisDataFromSet(0, setA - 1));
						--setA;
						valueAdded = true;
					}
					else
					{
						randSetIndex = 1;
					}
					break;
				case 1:
					if (setB > 0)
					{
						out_container.push_back(m_data.GetIrisDataFromSet(1, setB - 1));
						--setB;
						valueAdded = true;
					}
					else
					{
						randSetIndex = 2;
					}
					break;
				default:
					if (setC > 0)
					{
						out_container.push_back(m_data.GetIrisDataFromSet(2, setC - 1));
						--setC;
						valueAdded = true;
					}
					else
					{
						randSetIndex = 0;
					}
					break;
				}
			}
		}
	}

	void TrainingData::GetRandomValidationData(std::vector< const IrisData* >& out_container)
	{
		//we randomize the order of the outputs
		int setA = 10;
		int setB = 10;
		int setC = 10;
		int sizeOfContainer = setA + setB + setC;

		out_container.clear();
		out_container.reserve(sizeOfContainer);

		int randSetIndex = 0;
		for (int i = 0; i < sizeOfContainer; ++i)
		{
			randSetIndex = RandNToN(6.0,3.0);

			bool valueAdded = false;
			while (!valueAdded)
			{
				switch (randSetIndex)
				{
				case 3:
					if (setA > 0)
					{
						out_container.push_back(m_data.GetIrisDataFromSet(0, setA - 1));
						--setA;
						valueAdded = true;
					}
					else
					{
						randSetIndex = 4;
					}
					break;
				case 4:
					if (setB > 0)
					{
						out_container.push_back(m_data.GetIrisDataFromSet(1, setB - 1));
						--setB;
						valueAdded = true;
					}
					else
					{
						randSetIndex = 5;
					}
					break;
				default:
					if (setC > 0)
					{
						out_container.push_back(m_data.GetIrisDataFromSet(2, setC - 1));
						--setC;
						valueAdded = true;
					}
					else
					{
						randSetIndex = 3;
					}
					break;
				}
			}
		}
	}

	int TrainingData::GetNumberOfSets() const
	{
		return m_data.GetNumberOfSets();
	}

	const NeuralNetDataSet* TrainingData::GetDataSetAtSpecifiedIndex(int dataSetIndex)
	{
		return m_data.GetDataSetAt(dataSetIndex);
	}

	/*
	unsigned TrainingData::getNextInputs(std::vector<double> &inputVals)
	{
		inputVals.clear();

		std::string line;
		getline(m_trainingDataFile, line);
		std::stringstream ss(line);

		std::string label;
		ss >> label;
		if (label.compare("in:") == 0) {
			double oneValue;
			while (ss >> oneValue) {
				inputVals.push_back(oneValue);
			}
		}

		return inputVals.size();
	}

	unsigned TrainingData::getTargetOutputs(std::vector<double> &targetOutputVals)
	{
		targetOutputVals.clear();

		std::string line;
		getline(m_trainingDataFile, line);
		std::stringstream ss(line);

		std::string label;
		ss >> label;
		if (label.compare("out:") == 0)
		{
			double oneValue;
			while (ss >> oneValue)
			{
				targetOutputVals.push_back(oneValue);
			}
		}

		return targetOutputVals.size();
	}
	*/
}